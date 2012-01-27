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

MODULE_ALIAS("usb:v0711p0550d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0900d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0901d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0902d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0903d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0918d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0711p0920d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v182Dp021Cd*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v182Dp0269d*dc*dsc*dp*ic*isc*ip*");
