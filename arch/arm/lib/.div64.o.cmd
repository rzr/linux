cmd_arch/arm/lib/div64.o := arm-linux-gnueabi-gcc -Wp,-MD,arch/arm/lib/.div64.o.d  -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -nostdinc -isystem /usr/lib/gcc/arm-linux-gnueabi/4.4.5/include -I/usr/local/src/project/linux/linux/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /usr/local/src/project/linux/linux/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-orion5x/include -Iarch/arm/plat-orion/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables  -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=xscale -include asm/unified.h -msoft-float -gdwarf-2         -c -o arch/arm/lib/div64.o arch/arm/lib/div64.S

source_arch/arm/lib/div64.o := arch/arm/lib/div64.S

deps_arch/arm/lib/div64.o := \
  /usr/local/src/project/linux/linux/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /usr/local/src/project/linux/linux/arch/arm/include/asm/linkage.h \
  /usr/local/src/project/linux/linux/arch/arm/include/asm/unwind.h \
    $(wildcard include/config/arm/unwind.h) \

arch/arm/lib/div64.o: $(deps_arch/arm/lib/div64.o)

$(deps_arch/arm/lib/div64.o):
