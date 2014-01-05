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
 * FILENAME:    $Workfile: qdFFmii.c $ 
 * 
 * DESCRIPTION: SMI access routines for Firefox board
 *     
 */
#include "mv_platform.h"
#include "mv_os.h"


/*
 * For each platform, all we need is 
 * 1) Assigning functions into 
 * 		fgtReadMii : to read MII registers, and
 * 		fgtWriteMii : to write MII registers.
 *
 * 2) Register Interrupt (Not Defined Yet.)
*/

/* 
 *  Firefox Specific Definition
 */
#define SMI_OP_CODE_BIT_READ                    1
#define SMI_OP_CODE_BIT_WRITE                   0
#define SMI_BUSY                                1<<28
#define READ_VALID                              1<<27

#define SMI_TIMEOUT_COUNTER				1000

/*****************************************************************************
*
* GT_BOOL qdFFReadMii (GT_QD_DEV* dev, unsigned int portNumber , 
*                      unsigned int MIIReg, unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/
GT_BOOL ffReadMii(GT_QD_DEV* dev,
		  unsigned int portNumber,
		  unsigned int MIIReg,
		  unsigned int* value)
{
  GT_U32       smiReg;
  unsigned int phyAddr;
  unsigned int timeOut = SMI_TIMEOUT_COUNTER; /* in 100MS units */
  int          i;
  
  /* first check that it is not busy */
  smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);	
  if (smiReg & SMI_BUSY) {
    for (i = 0; i < SMI_TIMEOUT_COUNTER; i++)
      ;
    do {
      smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
      if (timeOut-- < 1) {
	printk("ffReadMii: timeout expired\n");
	return GT_FALSE;
      }
    } while (smiReg & SMI_BUSY);
  }

  /* not busy */
  phyAddr = portNumber;
  smiReg = 
    (phyAddr << 16)
    | (SMI_OP_CODE_BIT_READ << 26)
    | (MIIReg << 21);
  
  gtOsGtRegWrite(GT_REG_ETHER_SMI_REG, smiReg);
  timeOut = SMI_TIMEOUT_COUNTER; /* initialize the time out var again */
  smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
  if (!(smiReg & READ_VALID)) {
      for (i = 0; i < SMI_TIMEOUT_COUNTER; i++)
	;
      do {
	smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
	if (timeOut-- < 1) {
	  printk("ffReadMii: timeout expired\n");
	  return GT_FALSE;
	}
      } while (!(smiReg & READ_VALID));
  }
  *value = (unsigned int)(smiReg & 0xffff);    
  
  return GT_TRUE;
}


/*****************************************************************************
* 
* GT_BOOL qdFFWriteMii (GT_QD_DEV* dev, unsigned int portNumber , 
*                       unsigned int MIIReg, unsigned int value)
* 
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/
GT_BOOL ffWriteMii(GT_QD_DEV* dev,
		   unsigned int portNumber,
		   unsigned int MIIReg,
                   unsigned int value)
{
  GT_U32       smiReg;
  unsigned int phyAddr;
  unsigned int timeOut = SMI_TIMEOUT_COUNTER; /* in 100MS units */
  int          i;

  /* first check that it is not busy */	
  smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
  if (smiReg & SMI_BUSY) {
    for (i = 0; i < SMI_TIMEOUT_COUNTER; i++)
      ;
    do {
      smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
      if (timeOut-- < 1) {
	printk("ffWriteMii: timeout expired\n");
	return GT_FALSE;
      }			
    } while (smiReg & SMI_BUSY);
  }

  /* not busy */
  phyAddr = portNumber;
  
  smiReg = 0; /* make sure no garbage value in reserved bits */
  smiReg =
    smiReg
    | (phyAddr << 16)
    | (SMI_OP_CODE_BIT_WRITE << 26)
    | (MIIReg << 21)
    | (value & 0xffff);	
  gtOsGtRegWrite(GT_REG_ETHER_SMI_REG, smiReg);	
  
  return GT_TRUE;
}
