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

MODULE_ALIAS("pci:v00001011d00000046sv00000E11sd00004058bc*sc*i*");
MODULE_ALIAS("pci:v00001011d00000046sv00000E11sd00004051bc*sc*i*");
MODULE_ALIAS("pci:v00001011d00000046sv00000E11sd00004050bc*sc*i*");
MODULE_ALIAS("pci:v00001000d00000010sv00000E11sd00004048bc*sc*i*");
MODULE_ALIAS("pci:v00001000d00000010sv00000E11sd00004040bc*sc*i*");
MODULE_ALIAS("pci:v00000E11d0000AE10sv00000E11sd00004034bc*sc*i*");
MODULE_ALIAS("pci:v00000E11d0000AE10sv00000E11sd00004033bc*sc*i*");
MODULE_ALIAS("pci:v00000E11d0000AE10sv00000E11sd00004032bc*sc*i*");
MODULE_ALIAS("pci:v00000E11d0000AE10sv00000E11sd00004031bc*sc*i*");
MODULE_ALIAS("pci:v00000E11d0000AE10sv00000E11sd00004030bc*sc*i*");
