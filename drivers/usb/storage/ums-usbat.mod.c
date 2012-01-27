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

MODULE_ALIAS("usb:v03F0p0207d0001dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v03F0p0307d0001dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v04E6p1010d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0781p0005d0005dc*dsc*dp*ic*isc*ip*");
