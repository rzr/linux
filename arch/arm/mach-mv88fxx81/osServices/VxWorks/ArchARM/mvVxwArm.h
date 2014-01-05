/*******************************************************************************
*                Copyright 2004, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
*******************************************************************************/
/*******************************************************************************
* mvOsCpuArchLib.c - Marvell CPU architecture library
*
* DESCRIPTION:
*       This library introduce Marvell API for OS dependent CPU architecture 
*       APIs. This library introduce single CPU architecture services APKI 
*       cross OS.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/


#ifndef __INCmvLinuxPpch
#define __INCmvLinuxPpch

#include "vxWorks.h"
#include "mvCommon.h"

#define CPU_PHY_MEM(x)			    (MV_U32)x
#define CPU_MEMIO_CACHED_ADDR(x)    (void*)x
#define CPU_MEMIO_UNCACHED_ADDR(x)	(void*)x


/* CPU architecture dependent 32, 16, 8 bit read/write IO addresses */
#define MV_MEMIO32_WRITE(addr, data)	\
    ((*((volatile unsigned int*)(addr))) = ((unsigned int)(data)))

#define MV_MEMIO32_READ(addr)       	\
    ((*((volatile unsigned int*)(addr))))

#define MV_MEMIO16_WRITE(addr, data)	\
    ((*((volatile unsigned short*)(addr))) = ((unsigned short)(data)))

#define MV_MEMIO16_READ(addr)       	\
    ((*((volatile unsigned short*)(addr))))

#define MV_MEMIO8_WRITE(addr, data) 	\
    ((*((volatile unsigned char*)(addr))) = ((unsigned char)(data)))

#define MV_MEMIO8_READ(addr)        	\
    ((*((volatile unsigned char*)(addr))))


/* No Fast Swap implementation (in assembler) for ARM */
#define MV_32BIT_LE_FAST(val)            MV_32BIT_LE(val)
#define MV_16BIT_LE_FAST(val)            MV_16BIT_LE(val)
#define MV_32BIT_BE_FAST(val)            MV_32BIT_BE(val)
#define MV_16BIT_BE_FAST(val)            MV_16BIT_BE(val)
    
/* 32 and 16 bit read/write in big/little endian mode */

/* 16bit write in little endian mode */
#define MV_MEMIO_LE16_WRITE(addr, data) \
        MV_MEMIO16_WRITE(addr, MV_16BIT_LE_FAST(data))

/* 16bit read in little endian mode */
static __inline MV_U16 MV_MEMIO_LE16_READ(MV_U32 addr)
{
	MV_U16 data;

	data= (MV_U16)MV_MEMIO16_READ(addr);

	return (MV_U16)MV_16BIT_LE_FAST(data);
}

/* 32bit write in little endian mode */
#define MV_MEMIO_LE32_WRITE(addr, data) \
        MV_MEMIO32_WRITE(addr, MV_32BIT_LE_FAST(data))

/* 32bit read in little endian mode */
static __inline MV_U32 MV_MEMIO_LE32_READ(MV_U32 addr)
{
	MV_U32 data;

	data= (MV_U32)MV_MEMIO32_READ(addr);

	return (MV_U32)MV_32BIT_LE_FAST(data);
}


/* CPU cache information */
#define CPU_I_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */
#define CPU_D_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */


/* Data cache flush one line */
#define mvOsCacheLineFlushInv(addr)								\
{                                                               \
  _WRS_ASM ("mcr p15, 0, %0, c7, c14, 1" : : "r" (addr));\
}

#define mvOsCacheLineInv(addr)									\
{                                                               \
  _WRS_ASM ("mcr p15, 0, %0, c7, c6, 1" : : "r" (addr)); \
}


/* Flush CPU pipe */
#define CPU_PIPE_FLUSH

/* ARM architecture APIs */


MV_U32  mvOsCpuRevGet (MV_VOID);
MV_U32  mvOsCpuPartGet (MV_VOID);
MV_U32  mvOsCpuArchGet (MV_VOID);
MV_U32  mvOsCpuVarGet (MV_VOID);
MV_U32  mvOsCpuAsciiGet (MV_VOID);


#endif /* __INCmvLinuxPpch */
