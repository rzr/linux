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

#include <common.h>
#include <malloc.h>
#include "mvTypes.h"

MV_U32 mvOsIoVirtToPhy( void* pDev, void* pVirtAddr )
{
    return (MV_U32)pVirtAddr; /* ronen - need to update for CIV */
}

void* mvOsIoUncachedMalloc( void* pDev, MV_U32 size, MV_U32* pPhyAddr )
{
    *pPhyAddr = (MV_U32)malloc(size);
    return (void *)(*pPhyAddr);
}

void mvOsIoUncachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    free(pVirtAddr);
}

void* mvOsIoCachedMalloc( void* pDev, MV_U32 size, MV_U32* pPhyAddr )
{
    *pPhyAddr = (MV_U32)malloc(size);
    return (void *)(*pPhyAddr);
}

void mvOsIoCachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    free(pVirtAddr);
}

MV_U32 mvOsCacheFlush( void* pDev, void* p, int size )
{
    return (MV_U32)p;/* ronen - need to be filled */ 
}

MV_U32 mvOsCacheInvalidate( void* pDev, void* p, int size )
{
    return (MV_U32)p;/* ronen - need to be filled */
}

int mvOsRand(void)
{
    return 0;
}

int mvOsStrCmp(const char *str1,const char *str2)
{

	do
	{
		if ((*str1++) != (*str2++)) return 1; /* not equal */
	
	}
	while ((*str1 != '\0') && (*str2 != '\0'));

	if (*str1 != *str2) return 1; /* not equal */

	/* equal */
	return 0;

}

