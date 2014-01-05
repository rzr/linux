#
# Makefile for the FHNETDRV module
#

TGT = fhnetdrv.o
QD = $(addprefix qddrv/, \
	Dev/qdInit.o \
	Dev/qdModule.o \
	Dev/qdIoctl.o \
	Dev/qdFFmii.o \
	src/driver/gtDrvConfig.o \
	src/driver/gtDrvEvents.o \
	src/driver/gtHwCntl.o \
	src/platform/gtDebug.o \
	src/platform/gtMiiSmiIf.o \
	src/platform/gtSem.o \
	src/platform/platformDeps.o \
	src/msapi/gtSysConfig.o \
	src/msapi/gtBrgFdb.o \
	src/msapi/gtBrgStp.o \
	src/msapi/gtBrgVlan.o \
	src/msapi/gtBrgVtu.o \
	src/msapi/gtEvents.o \
	src/msapi/gtPhyCtrl.o \
	src/msapi/gtPhyInt.o \
	src/msapi/gtPortCtrl.o \
	src/msapi/gtPortPav.o \
	src/msapi/gtPortRateCtrl.o \
	src/msapi/gtPortRmon.o \
	src/msapi/gtPortStat.o \
	src/msapi/gtPortStatus.o \
	src/msapi/gtQosMap.o \
	src/msapi/gtSysCtrl.o \
	src/msapi/gtSysStatus.o \
	src/msapi/gtUtils.o \
	src/msapi/gtVct.o \
	src/msapi/gtVersion.o \
	)
UNIMAC = $(addprefix unimac/, \
	mv_eth.o \
	mv_eth_addrtbl.o \
	mv_unm.o \
	mv_unm_netconf.o \
	)
OBJS = $(QD) $(UNIMAC)

DEBUG  = -DNDEBUG # -DDEBUG_QD 
DEBUG += -DETH_DBG_ERROR # -DETH_DBG_PKT # -DETH_DBG_TRACE # -DETH_DBG_INFO
OPT = -O2

CFLAGS = -I. -I./include -I./unimac \
	-I./qddrv/Include -I./qddrv/Include/h/driver \
	-I./qddrv/Include/h/platform -I./qddrv/Include/h/msApi

all: install

$(TGT): $(OBJS)
	$(LD) -r $^ -o $@

include ../Makefile.common
