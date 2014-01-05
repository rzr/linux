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

#ifndef MV_OS_VXW_H
#define MV_OS_VXW_H

/*
 * VxWorks
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cacheLib.h>
#include <taskLib.h>
#include <intLib.h>


#include "mvTypes.h"
#include "mvCommon.h"

#if defined(MV_MIPS)
#   include "mvVxwMips.h"
#elif defined (MV_PPC)
#   include "mvVxwPpc.h"
#elif defined (MV_ARM)
#   include "mvVxwArm.h"
#else
#   error "CPU type not selected"
#endif    


#define INLINE	__inline
#define __TASKCONV

#ifdef MV_RT_DEBUG
#define mvOsAssert(cond)        assert(cond)
#else
#define mvOsAssert(cond)
#endif /* MV_RT_DEBUG */

#define mvOsOutput              mvOsVxwPrintf
#define mvOsPrintf              mvOsVxwPrintf
#define mvOsSPrintf             sprintf

#define mvOsStrCmp		strcmp

#define mvOsTaskLock            taskLock
#define mvOsTaskUnlock          taskUnlock

#define mvOsIntLock             intLock
#define mvOsIntUnlock           intUnlock

#define mvOsMalloc				malloc
#define mvOsRealloc				realloc
#define mvOsFree				free
#define mvOsDelay				sysMsDelay
#define mvOsUDelay              sysUsDelay

#define mvOsIoVirtToPhy(pDev, pVirtAddr)   CPU_PHY_MEM(pVirtAddr)

/* Function declaration */
/* Task Managment */

MV_STATUS   mvOsTaskCreate(char  *name,
                          unsigned long prio,
                          unsigned long stack,
                          unsigned (__TASKCONV *start_addr)(void*),
                          void   *arglist,
                          unsigned long *tid);
MV_STATUS   mvOsTaskIdent (char *name, unsigned long *tid);
MV_STATUS   mvOsTaskDelete(unsigned long tid);
MV_STATUS   mvOsTaskSuspend(unsigned long tid);
MV_STATUS   mvOsTaskResume(unsigned long tid);

/* Queues Managment */

MV_STATUS   mvOsQueueCreate(char *name, unsigned long size, unsigned long *qid);
MV_STATUS   mvOsQueueDelete(unsigned long qid);
MV_STATUS   mvOsQueueWait(unsigned long qid, void* msg, unsigned long time_out);
MV_STATUS   mvOsQueueSend(unsigned long qid, void* msg);


/* Cache Managment */
        
static INLINE MV_ULONG    mvOsCacheFlush(void* pDev, void* pVirtAddr, int size)
{
    cacheFlush(DATA_CACHE, (pVirtAddr), (size));
    return CPU_PHY_MEM(pVirtAddr);
}

static INLINE MV_ULONG   mvOsCacheInvalidate (void* pDev, void* pVirtAddr, int size)
{
    cacheInvalidate(DATA_CACHE, (pVirtAddr), (size));
    return CPU_PHY_MEM(pVirtAddr);
}


void mvOsVxwPrintf(const char *  fmt, ...);
void sysMsDelay(UINT ms);
void sysUsDelay(UINT us);

#endif /* MV_OS_VXW_H */
