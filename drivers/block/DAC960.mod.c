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

MODULE_ALIAS("pci:v00001069d0000B166sv00001069sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001069d0000BA56sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001069d00000050sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001011d00001065sv00001069sd00000020bc*sc*i*");
MODULE_ALIAS("pci:v00001069d00000010sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001069d00000002sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001069d00000001sv*sd*bc*sc*i*");
