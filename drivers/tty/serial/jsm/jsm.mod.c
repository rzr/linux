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

MODULE_ALIAS("pci:v0000114Fd000000C8sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000114Fd000000C9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000114Fd000000CAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000114Fd000000CBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000114Fd000000F4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000114Fd000000B1sv*sd*bc*sc*i*");
