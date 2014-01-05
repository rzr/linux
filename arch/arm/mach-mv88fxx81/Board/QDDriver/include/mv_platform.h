/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

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
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/*
 *
 * DESCRIPTION:
 *     This file defines the support required for CPU and BSP independence.
 */

#ifndef _MV_PLATFORM_H_
#define _MV_PLATFORM_H_


#include "msApiTypes.h"
#include "mv_common.h"
#include "mv_regs.h"

/*-------------------------------------------------*/
/* Trailers and Headers settings                   */
/*-------------------------------------------------*/
#undef TRAILERS
#define HEADERS

#ifdef TRAILERS
#define QD_TRAILER_MODE
#endif 

#ifdef HEADERS
#define QD_HEADER_MODE
#endif


/*-------------------------------------------------*/
/* Types for IOCTL implementation                  */
/*-------------------------------------------------*/

#define  STATUS_SUCCESS (0)
#define  STATUS_UNSUCCESSFUL (-1)

typedef enum {
  false =0,
  true
} bool;

typedef void VOID;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned int DWORD;
typedef char WCHAR;
#define MAX_VLAN_NAME                   (20)



#if defined(MV88E6318) /* MIPS Firehawk */
#  define PLATFORM_GTREG_BASE 0xb4000000 /* uncached */
#  define GT_OS_IOMEM_PREFIX  0xa0000000
#else /* ARM Firefox */ 
#  define PLATFORM_GTREG_BASE 0x80000000    
#  define GT_OS_IOMEM_PREFIX  0x00000000
#endif

/*  Access Device BUS for internal usage only */

#define PLATFORM_DEV_READ_CHAR(offset) *(volatile GT_U8*)((offset)|PLATFORM_DEV_BASE|GT_OS_IOMEM_PREFIX);

#define PLATFORM_DEV_WRITE_CHAR(offset, data) *(volatile GT_U8*)((offset)|PLATFORM_DEV_BASE|GT_OS_IOMEM_PREFIX) = (data);


/* Access to FF registers (Read/Write) */
static inline void gtOsGtRegWrite(GT_U32 gtreg, GT_U32 data)
{
    ((volatile GT_U32)*((volatile GT_U32*)(PLATFORM_GTREG_BASE|GT_OS_IOMEM_PREFIX|gtreg))) = data;
}

static inline GT_U32 gtOsGtRegRead(GT_U32 gtreg)
{
	return ( (volatile GT_U32)*((volatile GT_U32*)(PLATFORM_GTREG_BASE|GT_OS_IOMEM_PREFIX|gtreg)) );
}

#endif /* _MV_PLATFORM_H_ */
