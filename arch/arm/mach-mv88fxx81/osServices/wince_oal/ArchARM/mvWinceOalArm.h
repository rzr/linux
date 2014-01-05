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


#ifndef __INCmvWinceArmh
#define __INCmvWinceArmh

#include <windows.h>
#include "mvTypes.h"
#include "oal.h"

#define CPU_PHY_MEM(x)			    (MV_U32)x
#define CPU_MEMIO_CACHED_ADDR(x)    OALPAtoVA((UINT32)x,TRUE)
#define CPU_MEMIO_UNCACHED_ADDR(x)	OALPAtoVA((UINT32)x,FALSE)


/* CPU architecture dependent 32, 16, 8 bit read/write IO addresses */
#define MV_MEMIO32_WRITE(addr, data)	\
    ((*((volatile unsigned int*)(CPU_MEMIO_UNCACHED_ADDR(addr)))) = ((unsigned int)(data)))

#define MV_MEMIO32_READ(addr)       	\
    ((*((volatile unsigned int*)(CPU_MEMIO_UNCACHED_ADDR(addr)))))

#define MV_MEMIO16_WRITE(addr, data)	\
    ((*((volatile unsigned short*)(CPU_MEMIO_UNCACHED_ADDR(addr)))) = ((unsigned short)(data)))

#define MV_MEMIO16_READ(addr)       	\
    ((*((volatile unsigned short*)(CPU_MEMIO_UNCACHED_ADDR(addr)))))

#define MV_MEMIO8_WRITE(addr, data) 	\
    ((*((volatile unsigned char*)(CPU_MEMIO_UNCACHED_ADDR(addr)))) = ((unsigned char)(data)))

#define MV_MEMIO8_READ(addr)        	\
    ((*((volatile unsigned char*)(CPU_MEMIO_UNCACHED_ADDR(addr)))))


#define MV_BYTE_SWAP_32BIT_FAST(value)  MV_BYTE_SWAP_32BIT(value)
#define MV_BYTE_SWAP_16BIT_FAST(value)  MV_BYTE_SWAP_16BIT(value)


/* 32bit read in little endian mode */
#if defined(MV_CPU_LE)
#   define MV_32BIT_LE_FAST(val)            (val)
#   define MV_16BIT_LE_FAST(val)            (val)
#elif defined(MV_CPU_BE)
#   define MV_32BIT_LE_FAST(val)            MV_BYTE_SWAP_32BIT_FAST(val)
#   define MV_16BIT_LE_FAST(val)            MV_BYTE_SWAP_16BIT_FAST(val)
#else
    #error "CPU endianess isn't defined!!\n"
#endif
    
/* 32bit read in little endian mode */

#if defined(MV_CPU_LE)
    #define MV_MEMIO_LE32_READ(addr) MV_MEMIO32_READ(addr)
#elif defined(MV_CPU_BE)
    #define MV_MEMIO_LE32_READ(addr)  MV_32BIT_LE(MV_MEMIO32_READ(addr))
#else
    #error "CPU endianess isn't defined!!\n"
#endif


#define MV_MEMIO_LE32_WRITE(addr, data) \
        MV_MEMIO32_WRITE(addr, MV_32BIT_LE_FAST(data))

/* CPU cache information */
#define CPU_I_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */
#define CPU_D_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */

/* maen later */
/* Data cache flush one line */
#define mvOsCacheLineFlushInv(addr)
#define mvOsCacheLineInv(addr)


/* Flush CPU pipe */
#define CPU_PIPE_FLUSH

/* ARM architecture APIs */


MV_U32  mvOsCpuRevGet (MV_VOID);
MV_U32  mvOsCpuPartGet (MV_VOID);
MV_U32  mvOsCpuArchGet (MV_VOID);
MV_U32  mvOsCpuVarGet (MV_VOID);
MV_U32  mvOsCpuAsciiGet (MV_VOID);


#endif /* __INCmvWinceArmh */
