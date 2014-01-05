/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.

*******************************************************************************/

#ifndef _MV_OS_UBOOT_H_
#define _MV_OS_UBOOT_H_

#include <common.h>
#include <malloc.h>

#define INLINE             inline
#define mvOsSPrintf        sprintf

/* In order to minimize image size printf, is defined as NULL */

#ifdef MV_RT_DEBUG
#   define mvOsPrintf      printf
#else
#   define mvOsPrintf(fmt,args...)         
#endif /* MV_RT_DEBUG */

#define mvOsOutput         printf 
#define mvOsMalloc         malloc
#define mvOsFree           free
#define mvOsDelay(us)      udelay(us*1000)
#define mvOsSleep(us)      mvOsDelay(us)
#define mvOsTaskLock()
#define mvOsTaskUnlock()
#define mvOsIntLock()       0
#define mvOsIntUnlock(key)     

#define strtol             simple_strtoul

#define mvOsDivide(num, div)        \
({                                  \
    int i=0, rem=(num);             \
                                    \
    while(rem >= (div))             \
    {                               \
        rem -= (div);               \
        i++;                        \
    }                               \
    (i);                            \
})

#define mvOsReminder(num, div)      \
({                                  \
    int rem = (num);                \
                                    \
    while(rem >= (div))             \
        rem -= (div);               \
    (rem);                          \
})


 
#if defined(MV_MIPS)
#   include "mvUbootMips.h"
#elif defined (MV_PPC)
#   include "mvUbootPpc.h"
#elif defined (MV_ARM)
#   include "mvUbootArm.h"
#else
#   error "CPU type not selected"
#endif

#endif /*_MV_OS_UBOOT_H_*/
