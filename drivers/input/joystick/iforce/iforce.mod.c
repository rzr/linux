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

MODULE_ALIAS("usb:v044FpA01Cd*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v046DpC281d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v046DpC291d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v05EFp020Ad*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v05EFp8884d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v05EFp8888d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v061CpC0A4d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v061CpC084d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v06F8p0001d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v06F8p0003d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v06F8p0004d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v06F8pA302d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("serio:ty02pr1Did*ex*");
