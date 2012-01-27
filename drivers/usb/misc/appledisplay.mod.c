#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=backlight";

MODULE_ALIAS("usb:v05ACp9218d*dc*dsc*dp*ic03isc*ip00*");
MODULE_ALIAS("usb:v05ACp9219d*dc*dsc*dp*ic03isc*ip00*");
MODULE_ALIAS("usb:v05ACp921Cd*dc*dsc*dp*ic03isc*ip00*");
MODULE_ALIAS("usb:v05ACp921Dd*dc*dsc*dp*ic03isc*ip00*");
