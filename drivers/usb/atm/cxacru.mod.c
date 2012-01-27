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
"depends=usbatm,atm";

MODULE_ALIAS("usb:v0572pCAFEd*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0572pCB00d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0572pCB01d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0572pCB02d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0572pCB06d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0572pCB07d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v08E3p0100d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v08E3p0102d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0EB0p3457d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v1803p5510d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0675p0200d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0586p330Ad*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0586p330Bd*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0659p0020d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0509p0812d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v100DpCB01d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v100Dp3342d*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "2626D9990CFCB9CA2230C96");
