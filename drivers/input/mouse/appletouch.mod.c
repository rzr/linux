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

MODULE_ALIAS("usb:v05ACp020Ed*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp020Fd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp030Ad*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp030Bd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0214d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0215d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0216d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0217d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0218d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0219d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp021Ad*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp021Bd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp021Cd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0229d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp022Ad*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp022Bd*dc*dsc*dp*ic03isc*ip02*");
