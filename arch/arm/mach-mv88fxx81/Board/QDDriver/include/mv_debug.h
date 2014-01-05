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
 *     This file defines few types and functions useful for debug purposes.
 */
#ifndef MV_DEBUG_H
#define MV_DEBUG_H


#include "mv_types.h"


/*	time measurement structure used to check how much time pass between
 *  two points
 */
typedef struct {
    char            name[20];	/* name of the entry */
    unsigned long   begin;		/* time measured on begin point */
    unsigned long   end;		/* time measured on end point */
    unsigned long   total;		/* Accumulated time */
    unsigned long   left;		/* The rest measurement actions */
    unsigned long   count;		/* Maximum measurement actions */
    unsigned long   min;		/* Minimum time from begin to end */
    unsigned long   max;		/* Maximum time from begin to end */
} MV_DEBUG_TIMES;

/* MACRO to print Debug messages */
#ifdef MV_DEBUG
#	define MV_DEBUG_PRINT	gtOsPrintf
#else
#       define MV_DEBUG_PRINT
#endif


/****** Error Recording ******/

/* Dump memory in specific format: 
 * address: X1X1X1X1 X2X2X2X2 ... X8X8X8X8 
 */
void      mvDebugMemoryDump(void* addr, int size);

/**** There are three functions deals with MV_DEBUG_TIMES structure ****/

/* Reset MV_DEBUG_TIMES entry */
void    mvDebugResetTimeEntry(MV_DEBUG_TIMES* pTimeEntry, int count, char* name);

/* Update MV_DEBUG_TIMES entry */
void    mvDebugUpdateTimeEntry(MV_DEBUG_TIMES* pTimeEntry);

/* Print out MV_DEBUG_TIMES entry */
void    mvDebugPrintTimeEntry(MV_DEBUG_TIMES* pTimeEntry, GT_BOOL isTitle);


/******** General ***********/

/* Change value of mvDebugPrint global variable */
void      mvDebugPrintEnable(GT_BOOL isEnable);


#endif /* MV_DEBUG_H */
