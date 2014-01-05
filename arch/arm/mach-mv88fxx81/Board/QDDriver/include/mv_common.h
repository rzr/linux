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
 *     This file defines general-purpose macros.
 */


#ifndef MV_COMMON_H
#define MV_COMMON_H




#define GT_ALIGN_UP(num, aln)   (((num)&((aln)-1)) ? (((num)+(aln))&~((aln)-1)) : (num))
#define GT_ALIGN_DOWN(num, aln) ((num)&~((aln)-1))

#define GT_BYTE0(s) ((s)&0xff)
#define GT_BYTE1(s) (((s)>>8)&0xff)
#define GT_BYTE2(s) (((s)>>16)&0xff)
#define GT_BYTE3(s) (((s)>>24)&0xff)
#define GT_HWORD0(s) ((s)&0xffff)
#define GT_HWORD1(s) (((s)>>16)&0xffff)

#define GT_BYTESWAP2(s) ((GT_BYTE0(s)<<8)|GT_BYTE1(s))

#define GT_BYTESWAP4(s) ((GT_BYTE0(s)<<24)|(GT_BYTE1(s)<<16)|(GT_BYTE2(s)<<8)|GT_BYTE3(s))

#define GT_HWORDSWAP(s) ((GT_HWORD0(s)<<16)|GT_HWORD1(s))

#define GT_LOAD_32_BITS(pBuf)                   \
    (GT_U32)((((pBuf)[0]<<24)&0xff000000) |     \
             (((pBuf)[1]<<16)&0x00ff0000) |     \
             (((pBuf)[2]<<8 )&0x0000ff00) |     \
             (((pBuf)[3])    &0x000000ff))

#define GT_LOAD_32_BITS_WITH_OFFSET(pBuf, offset)   \
    (GT_U32)( ((pBuf)[0]<<(24+(offset))) |          \
              ((pBuf)[1]<<(16+(offset))) |          \
              ((pBuf)[2]<<(8+(offset)))  |          \
              ((pBuf)[3]<<((offset)))    |          \
              ((pBuf)[4]>>(8-(offset))) )

#define GT_STORE_32_BITS(pBuf, val32)       \
do{                                         \
    (pBuf)[0] = (GT_U8)((val32)>>24);       \
    (pBuf)[1] = (GT_U8)((val32)>>16);       \
    (pBuf)[2] = (GT_U8)((val32)>>8);        \
    (pBuf)[3] = (GT_U8)(val32);             \
} while (0);

#define GT_LOAD_16_BITS(pBuf)               \
    (GT_U16)( (((pBuf)[0]<<8)&0xff00) |     \
              (((pBuf)[1])   &0x00ff))


#define GT_STORE_16_BITS(pBuf, val16)       \
do{                                         \
    (pBuf)[0] = (GT_U8)((val16)>>8);        \
    (pBuf)[1] = (GT_U8)(val16);             \
} while (0);

/*---------------------------------------------------------------------------*/
/*           BIT FIELDS MANIPULATION MACROS                                  */
/*---------------------------------------------------------------------------*/
#define GT_BIT_MASK(x)       (1<<(x))     /* integer which its 'x' bit is set */

/*---------------------------------------------------------------------------*/
/* checks wheter bit 'x' in 'a' is set and than returns TRUE,                */
/* otherwise return FALSE.                                                   */
/*---------------------------------------------------------------------------*/
#define GT_CHKBIT(a, x)     (((a) & GT_BIT_MASK(x)) >> (x))     

/*---------------------------------------------------------------------------*/
/* Clear (reset) bit 'x' in integer 'a'                                      */
/*---------------------------------------------------------------------------*/
#define GT_CLRBIT(a, x)     ((a) &= ~(GT_BIT_MASK(x)))

/*---------------------------------------------------------------------------*/
/* SET bit 'x' in integer 'a'                                                */
/*---------------------------------------------------------------------------*/
#define GT_SETBIT(a, x)     ((a) |= GT_BIT_MASK(x))

/*---------------------------------------------------------------------------*/
/*   INVERT bit 'x' in integer 'a'.                                          */
/*---------------------------------------------------------------------------*/
#define GT_INVBIT(a, x)   ((a) = (a) ^ GT_BIT_MASK(x))

/*---------------------------------------------------------------------------*/
/* Get the min between 'a' or 'b'                                             */
/*---------------------------------------------------------------------------*/
#define GT_MIN(a,b)    (((a) < (b)) ? (a) : (b)) 

/*---------------------------------------------------------------------------*/
/* Get the max between 'a' or 'b'                                             */
/*---------------------------------------------------------------------------*/
#define GT_MAX(a,b)    (((a) < (b)) ? (b) : (a)) 


#endif /* MV_COMMON_H */
