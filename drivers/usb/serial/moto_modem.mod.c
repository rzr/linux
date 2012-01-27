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
"depends=usbserial";

MODULE_ALIAS("usb:v05C6p3197d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0C44p0022d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v22B8p2A64d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v22B8p2C84d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v22B8p2C64d*dc*dsc*dp*ic*isc*ip*");
