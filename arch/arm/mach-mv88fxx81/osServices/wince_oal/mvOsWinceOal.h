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

#ifndef _MV_OS_WINCE_H_
#define _MV_OS_WINCE_H_

#include <windows.h>
#include "oal.h"



void mvOsPrintf(const char *str, ...);
unsigned int mvOsSPrintf (unsigned char *pBuf, const unsigned char *sz, ...);



/* 
maen: should check ifstrtol  it is exported to the kernel , 
another solution is using atoi : 

#define strtol(nptr,endptr,base) atoi(nptr)
*/

long strtol( const char *nptr, char **endptr, int base );
#define strtol
#define INLINE             __inline




#define mvOsTaskLock            /* maen: only in Os Services */
#define mvOsTaskUnlock          /* maen: only in Os Services *//

#define mvOsIntLock             /* maen: later */
#define mvOsIntUnlock           /* maen: later */

#define mvOsDelay(ms)				OALStall(1000 * ms)

#define mvOsIoVirtToPhy(pDev, pVirtAddr)   (MV_U32)OALVAtoPA((pVirtAddr))
        

#if defined(MV_MIPS)
#   include "mvWinceOalMips.h"
#elif defined (MV_ARM)
#   include "mvWinceOalArm.h"
#else
#   error "CPU type not selected"
#endif

#endif /*_MV_OS_WINCE_H_*/
