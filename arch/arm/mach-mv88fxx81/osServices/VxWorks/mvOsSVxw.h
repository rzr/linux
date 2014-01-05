/*******************************************************************************
*                   Copyright 2002, GALILEO TECHNOLOGY, LTD.                   *
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
* (MJKK), GALILEO TECHNOLOGY LTD. (GTL) AND GALILEO TECHNOLOGY, INC. (GTI).    *
********************************************************************************
* mvOsVxWorks.h - O.S. interface header file for VxWorks
*
* DESCRIPTION:
*       This header file contains OS dependent definition under VxWorks
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*******************************************************************************/

#ifndef __INCmvOsVxWorksh
#define __INCmvOsVxWorksh

/* Includes */
#include <vxWorks.h>
#include <objLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <cacheLib.h>
#include "mvOs.h"


/* Definitions */
#define MV_API_SUPPROT_PORT_MULTIPLIER


/* System dependent macro for flushing CPU write cache */
#define MV_CPU_WRITE_BUFFER_FLUSH()		cachePipeFlush()

/* System dependent little endian from / to CPU conversions */
#if (_BYTE_ORDER == _BIG_ENDIAN)
#	define MV_CPU_TO_LE16(x)	MV_16BIT_LE(x)
#	define MV_CPU_TO_LE32(x)	MV_32BIT_LE(x)

#	define MV_LE16_TO_CPU(x)	MV_16BIT_LE(x)
#	define MV_LE32_TO_CPU(x)	MV_32BIT_LE(x)
#else
#	define MV_CPU_TO_LE16(x)	(x)
#	define MV_CPU_TO_LE32(x)	(x)

#	define MV_LE16_TO_CPU(x)	(x)
#	define MV_LE32_TO_CPU(x)	(x)
#endif

/* System dependent register read / write in byte/word/dword variants */
/* Write 32/16/8 bit NonCacheable */

#define MV_WRITE_CHAR(address, data)                                           \
        ((*((volatile unsigned char *)(address)))=             \
        ((unsigned char)(data)))

#define MV_WRITE_SHORT(address, data)                                          \
        ((*((volatile unsigned short *)(address))) =           \
        ((unsigned short)(data)))

#define MV_WRITE_WORD(address, data)                                           \
        ((*((volatile unsigned int *)(address))) =             \
        ((unsigned int)(data)))

/* Read 32/16/8 bit NonCacheable - returns data direct. */

#define MV_READCHAR(address)                                                   \
        ((*((volatile unsigned char *)(address))))

#define MV_READSHORT(address)                                                  \
        ((*((volatile unsigned short *)(address))))

#define MV_READWORD(address)                                                   \
        ((*((volatile unsigned int *)(address))))
#define	MV_REG_WRITE_BYTE(base, offset, val)  MV_WRITE_CHAR(base + offset, val)
#define	MV_REG_WRITE_WORD(base, offset, val)	MV_WRITE_SHORT(base + offset, MV_CPU_TO_LE16(val))   	
#define	MV_REG_WRITE_DWORD(base, offset, val)	MV_WRITE_WORD(base + offset, MV_CPU_TO_LE32(val))
#define	MV_REG_READ_BYTE(base, offset)	MV_READCHAR(base + offset)
#define	MV_REG_READ_WORD(base, offset)  mvSwapShort(MV_READSHORT(base + offset))
#define	MV_REG_READ_DWORD(base, offset)	mvSwapWord(MV_READWORD(base + offset))

/* Typedefs    */
#ifdef MV_FALSE
#undef MV_FALSE
#endif
#ifdef MV_TRUE
#undef MV_TRUE
#endif

typedef enum mvBoolean{MV_FALSE = 0, MV_TRUE} MV_BOOLEAN;

/* System dependant typedefs */
typedef void			*MV_VOID_PTR;
typedef unsigned char	*MV_U8_PTR;
typedef unsigned short	*MV_U16_PTR;
typedef unsigned long 	*MV_U32_PTR;
typedef char			*MV_CHAR_PTR;
typedef unsigned int	MV_BUS_ADDR_T;

/* Structures  */
/* System dependent structure */
typedef struct mvOsSemaphore
{
	SEM_ID sem_id;
	int		owner;
	int		line;
	int		rel_line;
} MV_OS_SEMAPHORE;


#define mvOsSemTake(pSem)
#define mvOsSemRelease(pSem)
#define mvOsSemInit(pSem)	(MV_TRUE)

/* Delay function in micro seconds resolution */
void mvMicroSecondsDelay(MV_VOID_PTR pAdapter, MV_U32);

/*#define MV_LOG_DEBUG*/
#define MV_LOG_ERROR
#include "mvLog.h"

MV_U16 mvSwapShort(MV_U16 data);
MV_U32 mvSwapWord(MV_U32 data);


extern unsigned int sataDebugIdMask;
extern unsigned int sataDebugTypeMask;
#define MV_IAL_LOG_ID       3

#endif /* __INCmvOsVxWorksh */

