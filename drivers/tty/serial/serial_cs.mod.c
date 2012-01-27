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
"depends=pcmcia";

MODULE_ALIAS("pcmcia:m0057c0021f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0089c110Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0104c000Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c0D0Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c0E0Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105cEA15f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0109c0501f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0138c110Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0140c000Af*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0143c3341f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0143cC0ABf*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m016Cc0081f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m021Bc0101f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m08A1cC0ABf*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF510DB04pb04CD2988pc46A52D63pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF510DB04pb0143B773pc46A52D63pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF510DB04pb856D66C8pcBD6C43EFpd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF510DB04pb52D21E1EpcBD6C43EFpd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pb*pc0EA978EApd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pb*pc80609023pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pb*pcA650C32Apd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pb*pc76DF1D29pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pb*pcF1403719pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa578BA6E7pbB0AC62C4pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paBA9EB7E2pb077C174Epc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa1A424A1CpbB23897FFpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paDD9989BEpb662C394Cpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2A151FACpb48B932AEpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa0733CC81pbB3765033pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF7CB0B07pb7A821B58pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF510DB04pb52D21E1Epc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paB2CED065pb3CED0555pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa18DF0BA0pb831B1064pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa0C2F80CDpb656947B9pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa0C2F80CDpbDC9BA5EDpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paDCFE12D3pbCD8906CCpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paDCFE12D3pbC67C648Fpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa1EAE9475pbD9A93BEDpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa2E3EE845pbECA401BFpc*pd*");
MODULE_ALIAS("pcmcia:m0032c0E01f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0032c0A05f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0032c0B05f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0032c1101f*fn*pfn01pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0104c0070f*fn00pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0101c0562f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0104c0070f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m016Cc0020f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa11C2DA09pb7289DC5DpcAAD95E1Fpd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*paB569A6E5pb5BD4FF2Cpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*paB569A6E5pb4BDF15C3pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*paB569A6E5pbAE911C15pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*paF03E4E77pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa*pb1ED59302pc*pd*");
MODULE_ALIAS("pcmcia:m0089c0301f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m00A4c0276f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0101c0039f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0104c0006f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c0101f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c100Af*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c3E0Af*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0105c410Af*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0107c0002f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m010Bc0D50f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m010Bc0D51f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m010Bc0D52f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m010Bc0D53f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m010BcD180f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0115c3330f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0124c0100f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0134c5600f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0137c000Ef*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0137c001Bf*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0137c0025f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0137c0045f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0137c0052f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m016Cc0006f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0200c0001f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa67459937pb*pc916D02BApd8FBE92AE*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa506BCCAEpbCB3685F1pc*pdBD6C43EF*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paF510DB04pb*pc*pdBD6C43EF*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paB4585A1Apb7271409Cpc*pdBD6C43EF*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa6EE5A3D8pb30EAD12BpcB00F05A0pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa32607776pbD9E73B13pc0E87332Epd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paF510DB04pb*pcBB2CCE4Apd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paEE138382pbD4CE9B02pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paE625F451pbCECD6DFApc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paA3A3062Cpb8CBD7C76pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paA3A3062Cpb5A00CE95pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paD0B78F51pb7E2D49EDpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa76942813pb8B96CE65pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paB569A6E5pbFEE5297Bpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paB496E65EpbF31602A6pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa816CC815pb412729FBpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paF8A097E3pb97880447pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BD2D898pb92ABC92Fpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa668388B3pb3F9BDF2Fpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa5F73BE51pb0B3E2383pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa5F73BE51pb4CD7C09Epc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paB572D360pbD2385B7Apc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa9D7CD6F5pb5CB8BF41pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa99BCAFE9pbAA25BCABpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paC4420B35pb92ABC92Fpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paC4420B35pb031A380Dpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa0E2CDD5EpbC9314B38pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa2D8CE292pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa281F1C5Dpb570F348Epc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01pa281F1C5Dpb6FDCACEEpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF5F025C2pb338E8155pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF5F025C2pb4AE85D35pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paF7CB0B07pb66881874pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn01paB4585A1Apb53F922F8pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa8FDF8F89pbDD5ED9E8pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa*pb*pc*pd58FC6056*");
MODULE_ALIAS("pcmcia:m0101c0556f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0175c0000f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0101c0035f*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m0101c003Df*fn01pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paD85F6206pb42A2C018pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paD85F6206pb698F93DBpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*paD85F6206pb761B11E0pc*pd*");
MODULE_ALIAS("pcmcia:m0192cA555f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m013FcA555f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa842047EEpbC2EFCF03pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa96913A85pb27AB5437pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa96913A85pbCEC8F102pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa96913A85pb8FBE92AEpc0877B627pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa*pbAD20B156pc*pd*");
MODULE_ALIAS("pcmcia:m0013c0000f*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa19CA78AFpbF964F42Bpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa19CA78AFpb71D98E83pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa19CA78AFpb69FB7490pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa19CA78AFpbB6BC0235pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa63F2E0BDpbB9E175D3pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa63F2E0BDpbFCE33442pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pb171E7190pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pb20DA4262pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pbEA5DD57Dpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pbD77255FApc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pb6A709903pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pb18430676pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn*pfn*pa3BEB8CF2pb6F933767pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn00pfn*pa3BEB8CF2pb16DC1BA7pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn00pfn*pa3BEB8CF2pb19816C41pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn00pfn*pa3BEB8CF2pb64112029pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn00pfn*pa3BEB8CF2pb1CCE7AC4pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn00pfn*pa3BEB8CF2pbDB9E58BCpc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa3BEB8CF2pb16DC1BA7pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa3BEB8CF2pb19816C41pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa3BEB8CF2pb64112029pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn01pfn*pa3BEB8CF2pb1CCE7AC4pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn02pfn*pa3BEB8CF2pb1CCE7AC4pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f*fn03pfn*pa3BEB8CF2pb1CCE7AC4pc*pd*");
MODULE_ALIAS("pcmcia:m0279c950Bf*fn*pfn*pa*pb*pc*pd*");
MODULE_ALIAS("pcmcia:m*c*f02fn*pfn*pa*pb*pc*pd*");
