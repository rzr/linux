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
* file_name - mvOsVxWorks.c
*
* DESCRIPTION: VxWorks system dependent source files
*
* DEPENDENCIES:
*	mvOsVxWorks.h
*
*******************************************************************************/

/* includes */

#include <stdio.h>
#include <vxWorks.h>
#include "mvOs.h"
#include "mvOsSVxw.h"

/* Defines */

/* Typedefs */


MV_U16 mvSwapShort(MV_U16 data)
{
    return MV_CPU_TO_LE16(data);
}
MV_U32 mvSwapWord(MV_U32 data)
{
    return MV_CPU_TO_LE32(data);
}
/* Delay function in micro seconds resolution */
void mvMicroSecondsDelay(MV_VOID_PTR pAdapter, MV_U32 usecs)
{
    int num, delay;

    if (usecs == 0)
    {
        taskDelay (0);

    }
    else
    {
        num = sysClkRateGet();
        delay = (num * usecs) / 1000000;
        if (delay == 0)
            delay = 1;
        taskDelay(delay);
    }
}

/* System logging function */
#if 0
    #ifdef MV_DEBUG_LOG
        #if 1
int debugLevel = 0xff;
        #else
int debugLevel = 0x40;
        #endif

int mvLogMsg(MV_U8 level, MV_CHAR_PTR format, ...)
{
    if (level & debugLevel)
    {
        char buff[2048];
        va_list args;
        int i;

        va_start(args, format);
        i = vsprintf(buff, format, args);
        va_end(args);

        return printf(buff);
    }
}
    #endif
#endif

unsigned int sataDebugIdMask = 0;
unsigned int sataDebugTypeMask = 0;

