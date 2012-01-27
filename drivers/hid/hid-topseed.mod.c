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

MODULE_ALIAS("hid:b0003v00000766p00000204");
MODULE_ALIAS("hid:b0003v0000046Ep00005578");
MODULE_ALIAS("hid:b0003v0000046Ep00005577");
MODULE_ALIAS("hid:b0003v00001784p00000004");
MODULE_ALIAS("hid:b0003v000004F2p00000618");
