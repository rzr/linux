/*
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <asm/unaligned.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/power/battery-charger-gauge-comm.h>
#include <linux/pm.h>
#include <linux/jiffies.h>
#include <linux/regmap.h>
#include <linux/power/lc709203f_battery.h>

#define LC709203F_THERMISTOR_B		0x06
#define LC709203F_INITIAL_RSOC		0x07
#define LC709203F_TEMPERATURE		0x08
#define LC709203F_VOLTAGE		0x09

#define LC709203F_ADJUSTMENT_PACK_APPLI	0x0B
#define LC709203F_ADJUSTMENT_PACK_THERM	0x0C
#define LC709203F_RSOC			0x0D
#define LC709203F_INDICATOR_TO_EMPTY	0x0F

#define LC709203F_IC_VERSION		0x11
#define LC709203F_CHANGE_OF_THE_PARAM	0x12
#define LC709203F_ALARM_LOW_CELL_RSOC	0x13
#define LC709203F_ALARM_LOW_CELL_VOLT	0x14
#define LC709203F_IC_POWER_MODE		0x15
#define LC709203F_STATUS_BIT		0x16
#define LC709203F_NUM_OF_THE_PARAM	0x1A

#define LC709203F_DELAY			(30*HZ)
#define LC709203F_MAX_REGS		0x1A

#define LC709203F_BATTERY_LOW		15
#define LC709203F_BATTERY_FULL		100

struct lc709203f_chip {
	struct i2c_client		*client;
	struct delayed_work		work;
	struct power_supply		battery;
	struct lc709203f_platform_data	*pdata;
	struct battery_gauge_dev	*bg_dev;
	struct regmap			*regmap;

	/* battery voltage */
	int vcell;
	/* battery capacity */
	int soc;
	/* State Of Charge */
	int status;
	/* battery health */
	int health;
	/* battery capacity */
	int capacity_level;

	int lasttime_soc;
	int lasttime_status;
	int shutdown_complete;
	int charge_complete;
	struct mutex mutex;
};

struct lc709203f_chip *lc709203f_data;

static const struct regmap_config lc709203f_regmap_config = {
	.reg_bits		= 8,
	.val_bits		= 8,
	.max_register		= LC709203F_MAX_REGS,
};

static int lc709203f_read_word(struct i2c_client *client, u8 reg)
{
	int ret;
	u16 val;

	struct lc709203f_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->mutex);
	if (chip && chip->shutdown_complete) {
		mutex_unlock(&chip->mutex);
		return -ENODEV;
	}

	ret = regmap_raw_read(chip->regmap, reg, (u8 *) &val, sizeof(val));
	if (ret < 0) {
		dev_err(&client->dev, "err reading reg: 0x%x, %d\n", reg, ret);
		mutex_unlock(&chip->mutex);
		return ret;
	}

	mutex_unlock(&chip->mutex);
	return val;
}

static int lc709203f_read_byte(struct i2c_client *client, u8 reg)
{
	int ret;
	u8 val;

	struct lc709203f_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->mutex);
	if (chip && chip->shutdown_complete) {
		mutex_unlock(&chip->mutex);
		return -ENODEV;
	}

	ret = regmap_raw_read(chip->regmap, reg, (u8 *) &val, sizeof(val));
	if (ret < 0) {
		dev_err(&client->dev, "err reading reg: 0x%x\n, %d", reg, ret);
		mutex_unlock(&chip->mutex);
		return ret;
	}

	mutex_unlock(&chip->mutex);
	return val;
}


static int lc709203f_write_byte(struct i2c_client *client, u8 reg, u8 value)
{
	struct lc709203f_chip *chip = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&chip->mutex);
	if (chip && chip->shutdown_complete) {
		mutex_unlock(&chip->mutex);
		return -ENODEV;
	}

	ret = regmap_write(chip->regmap, reg, value);
	if (ret < 0)
		dev_err(&client->dev, "err writing 0x%0x, %d\n" , reg, ret);

	mutex_unlock(&chip->mutex);
	return ret;
}

static void lc709203f_work(struct work_struct *work)
{
	struct lc709203f_chip *chip;
	int val;

	chip = container_of(work, struct lc709203f_chip, work.work);

	val = lc709203f_read_word(chip->client, LC709203F_VOLTAGE);
	if (val < 0)
		dev_err(&chip->client->dev, "%s: err %d\n", __func__, val);
	else
		chip->vcell = val;

	val = lc709203f_read_word(chip->client, LC709203F_RSOC);
	if (val < 0)
		dev_err(&chip->client->dev, "%s: err %d\n", __func__, val);
	else
		chip->soc = val;
	if (chip->soc >= LC709203F_BATTERY_FULL && chip->charge_complete != 1)
		chip->soc = LC709203F_BATTERY_FULL-1;

	if (chip->status == POWER_SUPPLY_STATUS_FULL && chip->charge_complete) {
		chip->soc = LC709203F_BATTERY_FULL;
		chip->capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
		chip->health = POWER_SUPPLY_HEALTH_GOOD;
	} else if (chip->soc < LC709203F_BATTERY_LOW) {
		chip->status = chip->lasttime_status;
		chip->health = POWER_SUPPLY_HEALTH_DEAD;
		chip->capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	} else {
		chip->status = chip->lasttime_status;
		chip->health = POWER_SUPPLY_HEALTH_GOOD;
		chip->capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	}

	if (chip->soc != chip->lasttime_soc ||
		chip->status != chip->lasttime_status) {
		chip->lasttime_soc = chip->soc;
		power_supply_changed(&chip->battery);
	}

	schedule_delayed_work(&chip->work, LC709203F_DELAY);
}

static int lc709203f_get_temperature(struct lc709203f_chip *chip)
{
	int val;

	val = lc709203f_read_word(chip->client, LC709203F_TEMPERATURE);
	if (val < 0) {
		dev_err(&chip->client->dev, "%s: err %d\n", __func__, val);
		return -EINVAL;
	}
	return val;
}

static enum power_supply_property lc709203f_battery_props[] = {
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_TEMP,
};

static int lc709203f_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct lc709203f_chip *chip = container_of(psy,
				struct lc709203f_chip, battery);
	int temperature;

	switch (psp) {
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = chip->status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = chip->vcell;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = chip->soc;
		if (chip->soc == 15)
			dev_warn(&chip->client->dev,
			"\nSystem Running low on battery - 15 percent\n");
		if (chip->soc == 10)
			dev_warn(&chip->client->dev,
			"\nSystem Running low on battery - 10 percent\n");
		if (chip->soc == 5)
			dev_warn(&chip->client->dev,
			"\nSystem Running low on battery - 5 percent\n");
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = chip->health;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = chip->capacity_level;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		temperature = lc709203f_get_temperature(chip);
		val->intval = (temperature / 10) - 273;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int lc709203f_update_battery_status(struct battery_gauge_dev *bg_dev,
		enum battery_charger_status status)
{
	struct lc709203f_chip *chip = battery_gauge_get_drvdata(bg_dev);

	if (status == BATTERY_CHARGING) {
		chip->charge_complete = 0;
		chip->status = POWER_SUPPLY_STATUS_CHARGING;
	} else if (status == BATTERY_CHARGING_DONE) {
		chip->charge_complete = 1;
		chip->soc = LC709203F_BATTERY_FULL;
		chip->status = POWER_SUPPLY_STATUS_FULL;
		power_supply_changed(&chip->battery);
		return 0;
	} else {
		chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
		chip->charge_complete = 0;
	}
	chip->lasttime_status = chip->status;
	power_supply_changed(&chip->battery);
	return 0;
}

static struct battery_gauge_ops lc709203f_bg_ops = {
	.update_battery_status = lc709203f_update_battery_status,
};

static struct battery_gauge_info lc709203f_bgi = {
	.cell_id = 0,
	.bg_ops = &lc709203f_bg_ops,
};

static void of_lc709203f_parse_platform_data(struct i2c_client *client,
				struct lc709203f_platform_data *pdata)
{
	char const *pstr;
	struct device_node *np = client->dev.of_node;

	if (!of_property_read_string(np, "onsemi,tz-name", &pstr))
		pdata->tz_name = pstr;
	else
		dev_err(&client->dev, "Failed to read tz-name\n");

}

static int lc709203f_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct lc709203f_chip *chip;
	int ret;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;

	if (client->dev.of_node) {
		chip->pdata = devm_kzalloc(&client->dev,
					sizeof(*chip->pdata), GFP_KERNEL);
		if (!chip->pdata)
			return -ENOMEM;
		of_lc709203f_parse_platform_data(client, chip->pdata);
	} else {
		chip->pdata = client->dev.platform_data;
	}

	if (!chip->pdata)
		return -ENODATA;

	lc709203f_data = chip;
	mutex_init(&chip->mutex);
	chip->shutdown_complete = 0;
	i2c_set_clientdata(client, chip);

	chip->battery.name		= "battery";
	chip->battery.type		= POWER_SUPPLY_TYPE_BATTERY;
	chip->battery.get_property	= lc709203f_get_property;
	chip->battery.properties	= lc709203f_battery_props;
	chip->battery.num_properties	= ARRAY_SIZE(lc709203f_battery_props);
	chip->status			= POWER_SUPPLY_STATUS_DISCHARGING;
	chip->lasttime_status		= POWER_SUPPLY_STATUS_DISCHARGING;
	chip->charge_complete		= 0;

	chip->regmap = devm_regmap_init_i2c(client, &lc709203f_regmap_config);
	if (IS_ERR(chip->regmap)) {
		ret = PTR_ERR(chip->regmap);
		dev_err(&client->dev, "regmap init failed with err %d\n", ret);
		goto error;
	}

	/* Dummy read to check if the slave is present*/
	ret = lc709203f_read_word(chip->client, LC709203F_VOLTAGE);
	if (ret < 0) {
		dev_err(&client->dev, "Exiting driver as xfer failed\n");
		/* Exit driver if not present */
		goto error;
	}

	ret = power_supply_register(&client->dev, &chip->battery);
	if (ret) {
		dev_err(&client->dev, "failed: power supply register\n");
		goto error;
	}

	lc709203f_bgi.tz_name = chip->pdata->tz_name;

	chip->bg_dev = battery_gauge_register(&client->dev, &lc709203f_bgi,
				chip);
	if (IS_ERR(chip->bg_dev)) {
		ret = PTR_ERR(chip->bg_dev);
		dev_err(&client->dev, "battery gauge register failed: %d\n",
			ret);
		goto bg_err;
	}

	INIT_DEFERRABLE_WORK(&chip->work, lc709203f_work);
	schedule_delayed_work(&chip->work, 0);

	return 0;
bg_err:
	power_supply_unregister(&chip->battery);
error:
	mutex_destroy(&chip->mutex);

	return ret;
}

static int lc709203f_remove(struct i2c_client *client)
{
	struct lc709203f_chip *chip = i2c_get_clientdata(client);

	battery_gauge_unregister(chip->bg_dev);
	power_supply_unregister(&chip->battery);
	cancel_delayed_work_sync(&chip->work);
	mutex_destroy(&chip->mutex);

	return 0;
}

static void lc709203f_shutdown(struct i2c_client *client)
{
	struct lc709203f_chip *chip = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&chip->work);
	mutex_lock(&chip->mutex);
	chip->shutdown_complete = 1;
	mutex_unlock(&chip->mutex);

}

#ifdef CONFIG_PM_SLEEP
static int lc709203f_suspend(struct device *dev)
{
	struct lc709203f_chip *chip = dev_get_drvdata(dev);
	cancel_delayed_work_sync(&chip->work);
	return 0;
}

static int lc709203f_resume(struct device *dev)
{
	struct lc709203f_chip *chip = dev_get_drvdata(dev);
	schedule_delayed_work(&chip->work, LC709203F_DELAY);
	return 0;
}
#endif /* CONFIG_PM */

static SIMPLE_DEV_PM_OPS(lc709203f_pm_ops, lc709203f_suspend, lc709203f_resume);

#ifdef CONFIG_OF
static const struct of_device_id lc709203f_dt_match[] = {
	{ .compatible = "onsemi,lc709203f" },
	{ },
};
MODULE_DEVICE_TABLE(of, lc709203f_dt_match);
#endif

static const struct i2c_device_id lc709203f_id[] = {
	{ "lc709203f", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lc709203f_id);

static struct i2c_driver lc709203f_i2c_driver = {
	.driver	= {
		.name	= "lc709203f",
		.of_match_table = of_match_ptr(lc709203f_dt_match),
		.pm = &lc709203f_pm_ops,
	},
	.probe		= lc709203f_probe,
	.remove		= lc709203f_remove,
	.id_table	= lc709203f_id,
	.shutdown	= lc709203f_shutdown,
};

static int __init lc709203f_init(void)
{
	return i2c_add_driver(&lc709203f_i2c_driver);
}
fs_initcall_sync(lc709203f_init);

static void __exit lc709203f_exit(void)
{
	i2c_del_driver(&lc709203f_i2c_driver);
}
module_exit(lc709203f_exit);

MODULE_AUTHOR("Chaitanya Bandi <bandik@nvidia.com>");
MODULE_DESCRIPTION("LC709203F Fuel Gauge");
MODULE_LICENSE("GPL");
