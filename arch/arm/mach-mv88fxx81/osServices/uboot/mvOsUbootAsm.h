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
 
#ifndef _MVOSUBOOTASM_H_
#define _MVOSUBOOTASM_H_


#include "mvCommon.h"
 
#if defined(MV_MIPS)
 #define CPU_FAMILY   MIPS
 #include "asm.h"

#elif defined (MV_PPC)
 #define CPU_FAMILY   PPC
 #include <config.h>
 #include <74xx_7xx.h>
 #include <ppc_asm.tmpl>
 #include <asm/cache.h>
 #include <asm/mmu.h>
 #include <ppc_defs.h>

#elif defined (MV_ARM)
 #define CPU_FAMILY   ARM
 #include <config.h>

/* BE/ LE swap for Asm */
#if defined(MV_CPU_LE)

#define htoll(x)        x
#define HTOLL(sr,tr)

#elif defined(MV_CPU_BE)

#define htoll(x) ((((x) & 0x00ff) << 24) | \
                  (((x) & 0xff00) <<  8) | \
                  (((x) >> 8)  & 0xff00) | \
                  (((x) >> 24) & 0x00ff))


#define HTOLL(sr,temp)                  /*sr   = A  ,B  ,C  ,D    */\
        eor temp, sr, sr, ROR #16 ;     /*temp = A^C,B^D,C^A,D^B  */\
        bic temp, temp, #0xFF0000 ;     /*temp = A^C,0  ,C^A,D^B  */\
        mov sr, sr, ROR #8 ;            /*sr   = D  ,A  ,B  ,C    */\
        eor sr, sr, temp, LSR #8        /*sr   = D  ,C  ,B  ,A    */

#endif

#define MV_REG_READ_ASM(toReg, tmpReg, regOffs)         \
        ldr     tmpReg, =(INTER_REGS_BASE + regOffs) ;  \
        ldr     toReg, [tmpReg]                      ;  \
        HTOLL(toReg,tmpReg)

#define MV_REG_WRITE_ASM(fromReg, tmpReg, regOffs)      \
        HTOLL(fromReg,tmpReg)                        ;  \
        ldr     tmpReg, =(INTER_REGS_BASE + regOffs) ;  \
        str     fromReg, [tmpReg]

#define MV_DV_REG_READ_ASM(toReg, tmpReg, regOffs)      \
        ldr     tmpReg, =(CFG_DFL_MV_REGS + regOffs) ;  \
        ldr     toReg, [tmpReg]                      ;  \
        HTOLL(toReg,tmpReg)

#define MV_DV_REG_WRITE_ASM(fromReg, tmpReg, regOffs)  	\
        HTOLL(fromReg,tmpReg)                        ;	\
        ldr     tmpReg, =(CFG_DFL_MV_REGS + regOffs) ;	\
        str     fromReg, [tmpReg]                         

#else
 #error "CPU type not selected"
#endif
 
		
#endif /* _MVOSUBOOTASM_H_ */

