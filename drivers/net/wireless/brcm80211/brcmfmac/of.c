/*
 * Copyright (c) 2014 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <linux/init.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/mmc/card.h>
#include <linux/platform_data/brcmfmac-sdio.h>

#include <defs.h>
#include "dhd_dbg.h"
#include "sdio_host.h"

struct brcmf_of_gpio_list {
	struct list_head list;
	struct gpio_desc *gpio;
};

struct brcmf_of_clock_list {
	struct list_head list;
	struct clk *clock;
};

const struct of_device_id brcmf_of_ids[] = {
	{ .compatible = "brcm,bcm43xx-fmac" },
	{ /* sentinal */ },
};
MODULE_DEVICE_TABLE(of, brcmf_of_ids);

static LIST_HEAD(gpio_list);
static LIST_HEAD(clock_list);

static void brcmf_of_power_on(struct device_node *np)
{
	struct brcmf_of_gpio_list *item;
	struct gpio_desc *pwr;
	enum of_gpio_flags flags;

	pwr = of_get_named_gpiod_flags(np, "brcm,wlan-supply", 0,
				       &flags);
	if (IS_ERR(pwr)) {
		brcmf_err("no brcm,wlan-supply specified\n");
		return;
	}

	if (gpio_request(desc_to_gpio(pwr), "wlan-supply")) {
		brcmf_err("Failed to request gpio(wlan-supply)\n");
		gpiod_put(pwr);
	} else {
		item = kzalloc(sizeof(*item), GFP_KERNEL);
		item->gpio = pwr;
		list_add(&item->list, &gpio_list);
		gpiod_direction_output(pwr, 0);
		gpiod_set_value_cansleep(pwr, 1);
	}
}

static void brcmf_of_clocks_enable(struct device_node *np)
{
	struct brcmf_of_clock_list *item;
	struct clk *clk;

	clk = of_clk_get_by_name(np, "sysclk");
	if (!IS_ERR(clk)) {
		item = kzalloc(sizeof(*item), GFP_KERNEL);
		item->clock = clk;
		list_add(&item->list, &clock_list);
		clk_prepare_enable(clk);
	}
	clk = of_clk_get_by_name(np, "sleepclk");
	if (!IS_ERR(clk)) {
		item = kzalloc(sizeof(*item), GFP_KERNEL);
		item->clock = clk;
		list_add(&item->list, &clock_list);
		clk_prepare_enable(clk);
	}
}

void __init brcmf_of_init(void)
{
	struct device_node *np;

	brcmf_dbg(SDIO, "Enter\n");
	for_each_matching_node(np, brcmf_of_ids) {
		/* enable card and optional clocks */
		brcmf_of_clocks_enable(np);
		brcmf_of_power_on(np);
	}
}

void brcmf_of_exit(void)
{
	struct brcmf_of_gpio_list *pwr;
	struct brcmf_of_gpio_list *pwr_next;
	struct brcmf_of_clock_list *clk;
	struct brcmf_of_clock_list *clk_next;

	list_for_each_entry_safe(pwr, pwr_next, &gpio_list, list) {
		/* gpio goes back to original state on put */
		gpiod_put(pwr->gpio);
		list_del(&pwr->list);
		kfree(pwr);
	}
	list_for_each_entry_safe(clk, clk_next, &clock_list, list) {
		clk_disable_unprepare(clk->clock);
		clk_put(clk->clock);
		list_del(&clk->list);
		kfree(clk);
	}

}

void brcmf_of_probe(struct brcmf_sdio_dev *sdiodev)
{
	struct device *dev = sdiodev->dev;
	struct device_node *np = dev->of_node;
	int irq;
	u32 irqf;
	u32 val;

	if (!np)
		return;

	sdiodev->pdata = devm_kzalloc(dev, sizeof(*sdiodev->pdata), GFP_KERNEL);
	if (!sdiodev->pdata)
		return;

	irq = irq_of_parse_and_map(np, 0);
	if (irq < 0) {
		brcmf_err("interrupt could not be mapped: err=%d\n", irq);
		devm_kfree(dev, sdiodev->pdata);
		return;
	}
	irqf = irqd_get_trigger_type(irq_get_irq_data(irq));

	sdiodev->pdata->oob_irq_supported = true;
	sdiodev->pdata->oob_irq_nr = irq;
	sdiodev->pdata->oob_irq_flags = irqf;

	if (of_property_read_u32(np, "brcm,drive-strength", &val) == 0)
		sdiodev->pdata->drive_strength = val;
}

