cmd_fs/ncpfs/ncpsign_kernel.o := arm-linux-gnueabi-gcc -Wp,-MD,fs/ncpfs/.ncpsign_kernel.o.d  -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -I/usr/local/src/project/linux/linux/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /usr/local/src/project/linux/linux/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-orion5x/include -Iarch/arm/plat-orion/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=xscale -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ncpsign_kernel)"  -D"KBUILD_MODNAME=KBUILD_STR(ncpfs)" -c -o fs/ncpfs/ncpsign_kernel.o fs/ncpfs/ncpsign_kernel.c

source_fs/ncpfs/ncpsign_kernel.o := fs/ncpfs/ncpsign_kernel.c

deps_fs/ncpfs/ncpsign_kernel.o := \
    $(wildcard include/config/ncpfs/packet/signing.h) \

fs/ncpfs/ncpsign_kernel.o: $(deps_fs/ncpfs/ncpsign_kernel.o)

$(deps_fs/ncpfs/ncpsign_kernel.o):
