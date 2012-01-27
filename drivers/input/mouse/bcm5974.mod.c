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

MODULE_ALIAS("usb:v05ACp0223d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0224d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0225d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0230d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0231d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0232d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0236d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0237d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0238d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp023Fd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0240d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0241d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0242d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0243d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0244d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0245d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0246d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0247d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0249d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp024Ad*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp024Bd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp024Cd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp024Dd*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp024Ed*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0252d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0253d*dc*dsc*dp*ic03isc*ip02*");
MODULE_ALIAS("usb:v05ACp0254d*dc*dsc*dp*ic03isc*ip02*");
