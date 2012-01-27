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

MODULE_ALIAS("hid:b0003v0000045Ep0000003B");
MODULE_ALIAS("hid:b0003v0000045Ep000000DB");
MODULE_ALIAS("hid:b0003v0000045Ep000000F9");
MODULE_ALIAS("hid:b0003v0000045Ep00000713");
MODULE_ALIAS("hid:b0003v0000045Ep00000730");
MODULE_ALIAS("hid:b0003v0000045Ep0000009D");
MODULE_ALIAS("hid:b0003v0000045Ep0000076C");
MODULE_ALIAS("hid:b0005v0000045Ep00000701");
