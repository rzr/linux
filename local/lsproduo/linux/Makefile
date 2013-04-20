#! /usr/bin/make -f

USER?=rzr

url?=git://github.com/${USER}/linux.git

ARCH?=arm

CROSS_COMPILE?=arm-linux-gnueabi-

KBUILD_DEBARCH?=armel

arch_url?=http://www.arm.linux.org.uk/developer/machines/download.php

ncpu?=$(shell grep processor /proc/cpuinfo  | wc -l)


address?=0x02000000
# 0x00000000

name?=lsproduo


default: config all

%:
	@echo "todo: $@"

all: ./arch/arm/tools/mach-types
	time fakeroot ${MAKE} \
	 oldconfig \
	 menuconfig \
	 deb-pkg \
	 uimage \
	  ARCH=${ARCH} \
	  CROSS_COMPILE=${CROSS_COMPILE} \
	  KBUILD_DEBARCH=${KBUILD_DEBARCH} \
	  KALLSYMS_EXTRA_PASS=1 \
 #eol

up: arch/arm/tools/mach-types

arch/arm/tools/mach-types:
	wget -O "$@" "${arch_url}"

setup: setup/debian


setup/debian:
	sudo aptitude install \
	 xmlto libncurses5-dev git make time fakeroot module-init-tools \
	 uboot-mkimage devio sudo \
	 hddtemp screen rsync \
 #eol 


uboot.dat:
	devio > $@ 'wl 0xe3a01c06,4' 'wl 0xe3811031,4' 
#	# debian ok vs  'wl 0xe3a01c07,4' 'wl 0xe3811027,4'


${uimage}.uboot: uboot.dat ${uimage}
	cat uboot.dat ${uimage} > ${uimage}.uboot

${uimage}.buffalo: ${uimage}.uboot 
	mkimage -A arm -O linux -T kernel -C none -a 0x00008000 -e 0x00008000 -n "${name}" -d $< $@



#eof
