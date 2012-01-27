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
"depends=ff-memless";

MODULE_ALIAS("usb:v*p*d*dc*dsc*dp*ic58isc42ip00*");
MODULE_ALIAS("usb:v045Ep*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v045Ep*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v046Dp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v046Dp*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v0738p*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v0738p*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v0E6Fp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v0E6Fp*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v12ABp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v12ABp*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v1430p*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v1430p*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v146Bp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v146Bp*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v1BADp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v1BADp*d*dc*dsc*dp*icFFisc5Dip81*");
MODULE_ALIAS("usb:v0F0Dp*d*dc*dsc*dp*icFFisc5Dip01*");
MODULE_ALIAS("usb:v0F0Dp*d*dc*dsc*dp*icFFisc5Dip81*");
