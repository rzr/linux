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
 *     This file is the OS Layer header, for uClinux.
 */

#ifndef _MV_OS_LINUX_H_
#define _MV_OS_LINUX_H_

/*
 * LINUX includes
 */
#include <linux/kernel.h>

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ip.h>
#include <linux/mii.h>

#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/pgtable.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>

#include <asm/88E6318/mv88E6318.h>
#include <asm/88E6318/mv88E6318int.h>
#include <asm/88E6318/mv88E6318regs.h>

#include "msApi.h"
#include "mv_types.h"
#include "mv_common.h"
#include "mv_platform.h"

#define INLINE	inline

#ifdef MV_DEBUG
#define ASSERT(assert) { do { if(!(assert)) { BUG(); } }while(0); }
#else
#define ASSERT(assert) 
#endif 

#define IN
#define OUT

#define INLINE inline

#define gtOsPrintf				printk

/*
 *  Physical memory access
 */
static INLINE GT_U32 gtOsIoMemReadWord(GT_U32 hpaddr)	
{
	return *(volatile GT_U32*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteWord(GT_U32 hpaddr, GT_U32 data) 
{
    *(volatile GT_U32*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

static INLINE GT_U16 gtOsIoMemReadShort(GT_U32 hpaddr)	
{
	return *(volatile GT_U16*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteShort(GT_U32 hpaddr, GT_U16 data) 
{
    *(volatile GT_U16*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

static INLINE GT_U8 gtOsIoMemReadByte(GT_U32 hpaddr)	
{
	return *(volatile GT_U8*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteByte(GT_U32 hpaddr, GT_U8 data) 
{
    *(volatile GT_U8*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

#endif /* _MV_OS_LINUX_H_ */
