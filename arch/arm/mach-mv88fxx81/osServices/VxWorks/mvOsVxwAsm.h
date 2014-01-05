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

#ifndef __INCmvOsVxwAsmh
#define __INCmvOsVxwAsmh

#define _ASMLANGUAGE

#include "VxWorks.h"
           
#include "asm.h"


/* BE/ LE swap for Asm */
#if defined(MV_CPU_LE)

#define htoll(x)    x
#define HTOLL(sr,tr)

#elif defined(MV_CPU_BE)

#define htoll(x) 	LONGSWAP(x)


#define HTOLL(sr,temp)                  /*sr   = A  ,B  ,C  ,D    */\
        eor temp, sr, sr, ROR $16 ;     /*temp = A^C,B^D,C^A,D^B  */\
        bic temp, temp, $0xFF0000 ;     /*temp = A^C,0  ,C^A,D^B  */\
        mov sr, sr, ROR $8 ;            /*sr   = D  ,A  ,B  ,C    */\
        eor sr, sr, temp, LSR $8        /*sr   = D  ,C  ,B  ,A    */

#endif

#define MV_REG_READ_ASM(toReg, tmpReg, regOffs)                 \
                ldr     tmpReg, =(INTER_REGS_BASE + regOffs) ;  \
        ldr     toReg, [tmpReg]                      ;  \
        HTOLL(toReg,tmpReg)

#define MV_REG_WRITE_ASM(fromReg, tmpReg, regOffs)      \
        HTOLL(fromReg,tmpReg)                        ;  \
        ldr     tmpReg, =(INTER_REGS_BASE + regOffs) ;  \
        str     fromReg, [tmpReg]

#endif /* __INCmvOsVxwAsmh */
