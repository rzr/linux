cmd_.tmp_kallsyms3.o := arm-linux-gnueabi-gcc -Wp,-MD,./..tmp_kallsyms3.o.d -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables  -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=xscale -include asm/unified.h -msoft-float -gdwarf-2    -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -I/usr/local/src/project/linux/linux/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /usr/local/src/project/linux/linux/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-orion5x/include -Iarch/arm/plat-orion/include    -c -o .tmp_kallsyms3.o .tmp_kallsyms3.S

source_.tmp_kallsyms3.o := .tmp_kallsyms3.S

deps_.tmp_kallsyms3.o := \
  /usr/local/src/project/linux/linux/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  /usr/local/src/project/linux/linux/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \

.tmp_kallsyms3.o: $(deps_.tmp_kallsyms3.o)

$(deps_.tmp_kallsyms3.o):
