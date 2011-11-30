/*
 * arch/arm/mach-orion5x/lsproduo-setup.c
 *
 * Source taken from arch/arm/mach-orion5x/lsmini-setup.c - kernel 2.6.30
 * Maintainer: Matt Gomboc <gomboc0@gmail.com>
 * Author: Manuel Bernhardt <prodigy7@gmail.com>
 *         Philippe Coval <rzr@gna.org>
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/leds.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/ata_platform.h>
#include <linux/gpio.h>
#include <linux/seq_file.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

/*****************************************************************************
 * Linkstation Pro Duo Info
 ****************************************************************************/

/*
 * 256K NOR flash Device bus boot chip select
 */

#define LSPRODUO_NOR_BOOT_BASE	0xf4000000
#define LSPRODUO_NOR_BOOT_SIZE	SZ_256K

/*****************************************************************************
 * 256KB NOR Flash on BOOT Device
 ****************************************************************************/

static struct physmap_flash_data lsproduo_nor_flash_data = {
	.width	 = 1,
};

static struct resource lsproduo_nor_flash_resource = {
	.flags	= IORESOURCE_MEM,
	.start	= LSPRODUO_NOR_BOOT_BASE,
	.end	= LSPRODUO_NOR_BOOT_BASE + LSPRODUO_NOR_BOOT_SIZE - 1,
};

static struct platform_device lsproduo_nor_flash = {
	.name		= "physmap-flash",
	.id			= 0,
	.dev		= {
		.platform_data	= &lsproduo_nor_flash_data,
	},
	.num_resources	   = 1,
	.resource		= &lsproduo_nor_flash_resource,
};

/*****************************************************************************
 * Ethernet
 ****************************************************************************/

static struct mv643xx_eth_platform_data lsproduo_eth_data = {
	.phy_addr	= 8,
};

/*****************************************************************************
 * RTC 5C372a on I2C bus
 ****************************************************************************/

static struct i2c_board_info lsproduo_i2c_rtc __initdata = {
	I2C_BOARD_INFO("rs5c372a", 0x32),
};

/*****************************************************************************
 * LEDs attached to GPIO
 ****************************************************************************/

#define LSPRODUO_GPIO_LED_ALARM	2
#define LSPRODUO_GPIO_LED_INFO	3
#define LSPRODUO_GPIO_LED_PWR	0

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
  #define LSPRODUO_GPIO_LED_FUNC   18
#endif

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV1
static struct gpio_led lsproduo_led_pins[] = {
	{
		.name	= "alarm:red",
		.gpio	= LSPRODUO_GPIO_LED_ALARM,
		.active_low	= 1,
	}, {
		.name	= "info:amber",
		.gpio	= LSPRODUO_GPIO_LED_INFO,
		.active_low	= 1,
	}, {
		.name	= "power:green",
		.gpio	= LSPRODUO_GPIO_LED_PWR,
		 .active_low	= 1,
	},
};
#endif
#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
static struct gpio_led lsproduo_led_pins[] = {
	{
		.name	= "alarm:red",
		.gpio	= LSPRODUO_GPIO_LED_ALARM,
		.active_low	= 1,
	}, {
		.name	= "info:amber",
		.gpio	= LSPRODUO_GPIO_LED_INFO,
		.active_low	= 1,
	}, {
		.name	= "power:green",
		.gpio	= LSPRODUO_GPIO_LED_PWR,
		.active_low	= 1,
	}, {
		.name	= "func:blue",
		.gpio	= LSPRODUO_GPIO_LED_FUNC,
		.active_low	= 1,
	},
};
#endif


static struct gpio_led_platform_data lsproduo_led_data = {
	.leds	   = lsproduo_led_pins,
	.num_leds       = ARRAY_SIZE(lsproduo_led_pins),
};

static struct platform_device lsproduo_leds = {
	.name   = "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data  = &lsproduo_led_data,
	},
};

/****************************************************************************
 * GPIO Attached Keys
 ****************************************************************************/
#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV1
 #define LSPRODUO_GPIO_KEY_POWER	8
 #define LSPRODUO_GPIO_KEY_AUTOPOWER	10

 #define LSPRODUO_SW_POWER		0x00
 #define LSPRODUO_SW_AUTOPOWER		0x01

static struct gpio_keys_button lsproduo_buttons[] = {
	{
		 .type	= EV_SW,
		 .code	= LSPRODUO_SW_POWER,
		 .gpio	= LSPRODUO_GPIO_KEY_POWER,
		 .desc	= "Power-on Switch",
		 .active_low	= 1,
	}, {
		 .type	= EV_SW,
		 .code	= LSPRODUO_SW_AUTOPOWER,
		 .gpio	= LSPRODUO_GPIO_KEY_AUTOPOWER,
		 .desc	= "Power-auto Switch",
		 .active_low	= 1,
	},
};
#endif

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
 #define LSPRODUO_GPIO_KEY_POWER	10
 #define LSPRODUO_GPIO_KEY_AUTOPOWER	22
 #define LSPRODUO_GPIO_KEY_FUNC		8

 #define LSPRODUO_SW_POWER		0x00
 #define LSPRODUO_SW_AUTOPOWER		0x01

static struct gpio_keys_button lsproduo_buttons[] = {
	{
		 .code	= KEY_OPTION,
		 .gpio	= LSPRODUO_GPIO_KEY_FUNC,
		 .desc	= "Function Button",
		 .active_low	= 1,
	}, {
		 .type	= EV_SW,
		 .code	= LSPRODUO_SW_POWER,
		 .gpio	= LSPRODUO_GPIO_KEY_POWER,
		 .desc	= "Power-on Switch",
		 .active_low	= 1,
	}, {
		 .type	= EV_SW,
		 .code	= LSPRODUO_SW_AUTOPOWER,
		 .gpio	= LSPRODUO_GPIO_KEY_AUTOPOWER,
		 .desc	= "Power-auto Switch",
		 .active_low	= 1,
	},
};

#endif

static struct gpio_keys_platform_data lsproduo_button_data = {
	.buttons	= lsproduo_buttons,
	.nbuttons	= ARRAY_SIZE(lsproduo_buttons),
};

static struct platform_device lsproduo_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		 .platform_data = &lsproduo_button_data,
	},
};

/****************************************************************************
 * GPIO Attached Fan
 ****************************************************************************/

/* Define max char len */
#define MAX_LEN 8

#define LSPRODUO_GPIO_FAN_LOW	17
#define LSPRODUO_GPIO_FAN_HIGH	14

static struct proc_dir_entry *lsproduo_proc_dir_root, *lsproduo_proc_dir_gpio;
static char lsproduo_fan_state[MAX_LEN];

static int lsproduo_fan_show(struct seq_file *m, void *v)
{
	seq_printf(m, "state: %s\n", lsproduo_fan_state);
	return 0;
};

static int lsproduo_fan_set(struct file *file, const char *buffer, size_t count, loff_t *data)
{
	int len, ret;
	char *ptr, tState[MAX_LEN];

	if (count > MAX_LEN)
		len = MAX_LEN;
	else
		len = count;

	ret = copy_from_user(tState, buffer, len);
	if (ret < 0) {
		printk(KERN_ERR "%s: Setting fan speed failed\n", "lsproduo");
		return -EFAULT;
	}

	ptr = strrchr(tState, '\n');
	if (ptr)
		*ptr = '\0';

	if (strcasecmp(tState, "off") == 0) {
		printk(KERN_DEBUG "%s: set fan off\n", "lsproduo");
		sprintf(lsproduo_fan_state, "off");
		gpio_set_value(LSPRODUO_GPIO_FAN_LOW, 1);
		gpio_set_value(LSPRODUO_GPIO_FAN_HIGH, 1);
	} else if (strcasecmp(tState, "slow") == 0) {
		printk(KERN_DEBUG "%s: set fan slow\n", "lsproduo");
		sprintf(lsproduo_fan_state, "slow");
		gpio_set_value(LSPRODUO_GPIO_FAN_LOW, 1);
		gpio_set_value(LSPRODUO_GPIO_FAN_HIGH, 0);
	} else if (strcasecmp(tState, "fast") == 0) {
		printk(KERN_DEBUG "%s: set fan fast\n", "lsproduo");
		sprintf(lsproduo_fan_state, "fast");
		gpio_set_value(LSPRODUO_GPIO_FAN_LOW, 0);
		gpio_set_value(LSPRODUO_GPIO_FAN_HIGH, 1);
	} else if (strcasecmp(tState, "full") == 0) {
		printk(KERN_DEBUG "%s: set fan full\n", "lsproduo");
		sprintf(lsproduo_fan_state, "full");
		gpio_set_value(LSPRODUO_GPIO_FAN_LOW, 0);
		gpio_set_value(LSPRODUO_GPIO_FAN_HIGH, 0);
	} else {
		printk(KERN_ERR "%s: unknown fan speed given\n", "lsproduo");
	}

	lsproduo_fan_state[len] = '\0';

	return len;
}

/*****************************************************************************
 * SATA
 ****************************************************************************/
static struct mv_sata_platform_data lsproduo_sata_data = {
	.n_ports	 = 2,
};


/*****************************************************************************
 * Linkstation Pro Duo specific power off method: reboot
 ****************************************************************************/
/*
 * On the Linkstation Pro Duo, the shutdown process is following:
 * - Userland monitors key events until the power switch goes to off position
 * - The board reboots
 * - U-boot starts and goes into an idle mode waiting for the user
 *   to move the switch to ON position
 */

static void lsproduo_power_off(void)
{
	orion5x_restart(REBOOT_HARD, NULL);
}


/*****************************************************************************
 * General Setup
 ****************************************************************************/
#define LSPRODUO_GPIO_HDD_POWER0	1
#define LSPRODUO_GPIO_USB_POWER		9
#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV1
 #define LSPRODUO_GPIO_POWER		8
 #define LSPRODUO_GPIO_AUTO_POWER	10
#endif
#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
 #define LSPRODUO_GPIO_POWER		10
 #define LSPRODUO_GPIO_USB_POWER2	19
 #define LSPRODUO_GPIO_AUTO_POWER	22
#endif

static unsigned int lsproduo_mpp_modes[] __initdata = {
	MPP0_GPIO,	/* LED_PWR */
	MPP1_GPIO,	/* HDD_PWR */
	MPP2_GPIO,	/* LED_ALARM */
	MPP3_GPIO,	/* LED_INFO */
	MPP4_UNUSED,
	MPP5_UNUSED,
	MPP6_GPIO,	/* FAN_LCK */
	MPP9_GPIO,	/* USB_PWR */
	MPP11_UNUSED,	/* LED_ETH dummy */
	MPP12_UNUSED,
	MPP13_UNUSED,
	MPP14_GPIO,	/* FAN_HIGH */
	MPP15_UNUSED,
	MPP16_UNUSED,
	MPP17_GPIO,	/* FAN_LOW */

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV1
	MPP7_GPIO,	/* INIT */
	MPP8_GPIO,	/* POWER */
	MPP10_GPIO,	/* AUTO_POWER */
	MPP18_UNUSED,
	MPP19_UNUSED,
#endif
#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
	MPP7_UNUSED,
	MPP8_GPIO,	/* FUNC */
	MPP10_GPIO,	/* POWER */
	MPP18_GPIO,	/* LED_FUNC*/
	MPP19_GPIO,	/* USB_PWR2 */
	MPP22_GPIO,	/* AUTO_POWER */
#endif
	0,
};

static int lsproduo_fan_open(struct inode *inode, struct file *file)
{
	return single_open(file, lsproduo_fan_show, NULL);
};

static const struct file_operations lsproduo_fan_fops = {
	.open = lsproduo_fan_open,
	.read = seq_read,
	.write = lsproduo_fan_set,
	.llseek = seq_lseek,
	.release = single_release,
};

static void __init lsproduo_init(void)
{
	/*
	 * Setup basic Orion functions. Need to be called early.
	 */
	orion5x_init();

	orion5x_mpp_conf(lsproduo_mpp_modes);

	/*
	 * Configure peripherals.
	 */
	orion5x_ehci0_init();
	orion5x_ehci1_init();
	orion5x_eth_init(&lsproduo_eth_data);
	orion5x_i2c_init();
	orion5x_sata_init(&lsproduo_sata_data);
	orion5x_uart0_init();
	orion5x_xor_init();

	mvebu_mbus_add_window_by_id(ORION_MBUS_DEVBUS_BOOT_TARGET,
				    ORION_MBUS_DEVBUS_BOOT_ATTR,
				    LSPRODUO_NOR_BOOT_BASE,
				    LSPRODUO_NOR_BOOT_SIZE);
	platform_device_register(&lsproduo_nor_flash);

	platform_device_register(&lsproduo_button_device);

	platform_device_register(&lsproduo_leds);

	i2c_register_board_info(0, &lsproduo_i2c_rtc, 1);

	/* enable USB power */
	gpio_set_value(LSPRODUO_GPIO_USB_POWER, 1);

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
	gpio_set_value(LSPRODUO_GPIO_USB_POWER2, 1);
#endif

	printk(KERN_INFO "Buffalo Linkstation Pro Duo fan driver loaded\n");
	sprintf(lsproduo_fan_state, "fast");
	gpio_set_value(LSPRODUO_GPIO_FAN_LOW, 1);
	gpio_set_value(LSPRODUO_GPIO_FAN_HIGH, 0);

	lsproduo_proc_dir_root = proc_mkdir("linkstation", NULL);
	lsproduo_proc_dir_gpio = proc_mkdir("gpio", lsproduo_proc_dir_root);
	proc_create("fan",
		S_IWUSR | S_IRUGO,
		lsproduo_proc_dir_gpio, &lsproduo_fan_fops);

	/* register power-off method */
	pm_power_off = lsproduo_power_off;

	pr_info("%s: finished\n", __func__);
}

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV1
MACHINE_START(LINKSTATION_PRODUO, "Buffalo Linkstation Pro Duo - Revision 1")
	.atag_offset	= 0x00000100,
	.init_machine	= lsproduo_init,
	.map_io		= orion5x_map_io,
	.init_early	= orion5x_init_early,
	.init_irq	= orion5x_init_irq,
	.init_time	= orion5x_timer_init,
	.fixup		= tag_fixup_mem32,
	.restart	= orion5x_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_LINKSTATION_PRODUO_REV2
MACHINE_START(LINKSTATION_PRODUO, "Buffalo Linkstation Pro Duo - Revision 2")
	.atag_offset	= 0x00000100,
	.init_machine	= lsproduo_init,
	.map_io		= orion5x_map_io,
	.init_early	= orion5x_init_early,
	.init_irq	= orion5x_init_irq,
	.init_time	= orion5x_timer_init,
	.fixup		= tag_fixup_mem32,
	.restart	= orion5x_restart,
MACHINE_END
#endif
