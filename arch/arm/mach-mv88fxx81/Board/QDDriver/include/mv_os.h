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
 *     This file is the OS Layer header, provided for OS independence.
 */

#ifndef MV_OS_H
#define MV_OS_H


/*************/
/* Constants */
/*************/

#define GT_OS_WAIT_FOREVER				0


/*************/
/* Datatypes */
/*************/

typedef enum {
	GT_OS_OK		= 0,
	GT_OS_FAIL		= 1, 
	GT_OS_TIMEOUT	= 2
} GT_OS_STATUS;


/*********************/
/* OS Specific Stuff */
/*********************/

#if defined(CISCO_IOS)

#include "mv_os_ios.h"

#elif defined(_WIN32_WCE)

#include "mv_os_wince.h"

#elif defined(_VXWORKS)

#include "mv_os_vx.h"

#elif defined(NETGX)

#include "mv_os_netgx.h"

#elif defined(WIN32)

#include "mv_os_win32.h"

#elif defined(LINUX)

#include "mv_os_linux.h"

#else /* No OS */

#include "mv_os_none.h"

#endif /* OS specific */


#ifdef HOST_LE /* HOST works in Little Endian mode */

#define gtOsWordHtoN(data)   GT_BYTESWAP4(data)
#define gtOsShortHtoN(data)  GT_BYTESWAP2(data)

#else   /* HOST_BE - HOST works in Big Endian mode */

#define gtOsWordHtoN(data)   (data)
#define gtOsShortHtoN(data)  (data)

#endif  /* HOST_LE || HOST_BE */

/*	Useful functions to deal with Physical memory using OS dependent inline
 *	functions defined in the file mv_os_???.h
 */

/*  Set physical memory with specified value. Use this function instead of
 *  memset when physical memory should be set.
 */
static INLINE void gtOsSetPhysMem(GT_U32 hpaddr, GT_U8 val, int size)
{
	int	i;

	for(i=0; i<size; i++)
	{
		gtOsIoMemWriteByte( (hpaddr+i), val);
	}
}

/*	Copy data from Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Physical memory to Local (virtual) memory. 
 */
static INLINE void gtOsCopyFromPhysMem(GT_U8* localAddr, GT_U32 hpaddr, int size)
{
	int		i;
	GT_U8	byte;

	for(i=0; i<size; i++)
	{
		byte = gtOsIoMemReadByte(hpaddr+i);
		localAddr[i] = byte;
	}
}

/*	Copy data to Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Local (virtual) memory to Physical memory. 
 */
static INLINE void gtOsCopyToPhysMem(GT_U32 hpaddr, const GT_U8* localAddr, int size)
{
	int		i;

	for(i=0; i<size; i++)
	{
		gtOsIoMemWriteByte( (hpaddr+i), localAddr[i]);
	}
}


/***********/
/* General */
/***********/

/* gtOsInit
 *
 * DESCRIPTION:
 *     Creates and initializes all the internal components of the OS Layer.
 *
 * RETURN VALUES:
 *     GT_OS_OK -- if succeeds.
 */
#ifndef gtOsInit
long gtOsInit(void);
#endif

/* gtOsFinish
 *
 * DESCRIPTION:
 *     Clears and deletes all the internal components of the OS Layer.
 *
 * RETURN VALUES:
 *     GT_OS_OK -- if succeeds.
 */
#ifndef gtOsFinish 
long gtOsFinish(void);
#endif

/* gtOsSleep
 *
 * DESCRIPTION:
 *     Sends the current task to sleep.
 *
 * INPUTS:
 *     mils -- number of miliseconds to sleep.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSleep
void	gtOsSleep(unsigned long mils);
#endif




/*********/
/* Misc. */
/*********/

/* gtOsGetCurrentTime
 *
 * DESCRIPTION:
 *     Returns current system's up-time.
 *
 * INPUTS:
 *     None.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     current up-time in miliseconds.
 */
#ifndef gtOsGetCurrentTime
unsigned long gtOsGetCurrentTime(void);
#endif

/* Returns last error number */
#ifndef gtOsGetErrNo
unsigned long	gtOsGetErrNo(void);
#endif

/* Returns random 32-bit number. */
#ifndef gtOsRand
unsigned int	gtOsRand(void);
#endif

/*****************/
/* File Transfer */
/*****************/

/* gtOsOpenFile
 *
 * DESCRIPTION:
 *     Opens a file on a remote machine.
 *
 * INPUTS:
 *     machine  -- name or IP-address string of the file server.
 *     path     -- path on the machine to the directory of the file.
 *     filename -- name of file to be opened.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     Handle to the opened file, or NULL on failure.
 */
#ifndef gtOsOpenFile
void* gtOsOpenFile(char* machine, char* path, char* filename);
#endif

/* gtOsCloseFile
 *
 * DESCRIPTION:
 *     Closes a file opened by gtOsOpenFile.
 *
 * INPUTS:
 *     hfile -- file handle.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     None.
 */
#ifndef gtOsCloseFile
void gtOsCloseFile(void* hfile);
#endif

/* gtOsGetFileLine
 *
 * DESCRIPTION:
 *     Reads up to len bytes from the file into the buffer, terminated with zero.
 *
 * INPUTS:
 *     buf   -- buffer pointer.
 *     len   -- buffer length (in bytes).
 *     hfile -- file handle.
 *
 * RETURN VALUES:
 *     The given 'buf' pointer.
 */
#ifndef gtOsGetFileLine
char* gtOsGetFileLine(void* hfile, char* buf, int len);
#endif

/**************/
/* Semaphores */
/**************/

/* gtOsSemCreate
 *
 * DESCRIPTION:
 *     Creates a semaphore.
 *
 * INPUTS:
 *     name  -- semaphore name.
 *     init  -- init value of semaphore counter.
 *     count -- max counter value (must be positive).
 *
 * OUTPUTS:
 *     smid -- pointer to semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemCreate
long gtOsSemCreate(char *name,unsigned long init,unsigned long count,
					 unsigned long *smid);
#endif


/* gtOsSemDelete
 *
 * DESCRIPTION:
 *     Deletes a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemDelete
long gtOsSemDelete(unsigned long smid);
#endif

/* gtOsSemWait
 *
 * DESCRIPTION:
 *     Waits on a semaphore.
 *
 * INPUTS:
 *     smid     -- semaphore ID.
 *     time_out -- time out in miliseconds, or 0 to wait forever.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemWait
long gtOsSemWait(unsigned long smid, unsigned long time_out);
#endif

/* gtOsSemSignal
 *
 * DESCRIPTION:
 *     Signals a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemSignal
long gtOsSemSignal(unsigned long smid);
#endif

#endif /* MV_OS_H */
