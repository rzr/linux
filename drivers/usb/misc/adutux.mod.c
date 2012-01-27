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
"depends=";

MODULE_ALIAS("usb:v0A07p0064d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0A07p0078d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0A07p0082d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0A07p00C8d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0A07p00D0d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0A07p00DAd*dc*dsc*dp*ic*isc*ip*");
