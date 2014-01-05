/*
 * $Id: chipreg.c,v 1.17 2004/11/16 18:29:00 dwmw2 Exp $
 *
 * Registration for chip drivers
 *
 */

#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/compatmac.h>

static DEFINE_SPINLOCK(chip_drvs_lock);
static LIST_HEAD(chip_drvs_list);

//#define CONFIG_BUFFALO_DEBUG

void register_mtd_chip_driver(struct mtd_chip_driver *drv)
{
	spin_lock(&chip_drvs_lock);
	list_add(&drv->list, &chip_drvs_list);
	spin_unlock(&chip_drvs_lock);
}

void unregister_mtd_chip_driver(struct mtd_chip_driver *drv)
{
	spin_lock(&chip_drvs_lock);
	list_del(&drv->list);
	spin_unlock(&chip_drvs_lock);
}

static struct mtd_chip_driver *get_mtd_chip_driver (const char *name)
{
	struct list_head *pos;
	struct mtd_chip_driver *ret = NULL, *this;

	spin_lock(&chip_drvs_lock);

	list_for_each(pos, &chip_drvs_list) {
		this = list_entry(pos, typeof(*this), list);

		if (!strcmp(this->name, name)) {
			ret = this;
			break;
		}
	}
	if (ret && !try_module_get(ret->module))
		ret = NULL;

	spin_unlock(&chip_drvs_lock);

	return ret;
}

	/* Hide all the horrid details, like some silly person taking
	   get_module_symbol() away from us, from the caller. */

struct mtd_info *do_map_probe(const char *name, struct map_info *map)
{
	struct mtd_chip_driver *drv;
	struct mtd_info *ret;
#if defined CONFIG_BUFFALO_DEBUG
	printk("%s :Entered\n", __FUNCTION__);
#endif
#if defined CONFIG_BUFFALO_DEBUG
	printk("%s :do mtd_chip_driver(%s)\n", __FUNCTION__, name);
#endif
	drv = get_mtd_chip_driver(name);
#if defined CONFIG_BUFFALO_DEBUG
	printk("%s :finish mtd_chip_driver(%s)\n", __FUNCTION__, name);
#endif

#if defined CONFIG_BUFFALO_DEBUG
	printk("%s :do request_module(%s), get_mtd_chip_driver(%s)\n", __FUNCTION__, name, name);
#endif
	if (!drv && !request_module("%s", name))
		drv = get_mtd_chip_driver(name);
#if defined CONFIG_BUFFALO_DEBUG
	printk("%s :finish request_module(%s), get_mtd_chip_driver(%s)\n", __FUNCTION__, name, name);
#endif

	if (!drv)
		return NULL;

	ret = drv->probe(map);

	/* We decrease the use count here. It may have been a
	   probe-only module, which is no longer required from this
	   point, having given us a handle on (and increased the use
	   count of) the actual driver code.
	*/
	module_put(drv->module);

#if defined CONFIG_BUFFALO_DEBUG
	printk("%s: Leaving\n", __FUNCTION__);
#endif
	if (ret)
		return ret;

	return NULL;
}
/*
 * Destroy an MTD device which was created for a map device.
 * Make sure the MTD device is already unregistered before calling this
 */
void map_destroy(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;

	if (map->fldrv->destroy)
		map->fldrv->destroy(mtd);

	module_put(map->fldrv->module);

	kfree(mtd);
}

EXPORT_SYMBOL(register_mtd_chip_driver);
EXPORT_SYMBOL(unregister_mtd_chip_driver);
EXPORT_SYMBOL(do_map_probe);
EXPORT_SYMBOL(map_destroy);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org>");
MODULE_DESCRIPTION("Core routines for registering and invoking MTD chip drivers");
