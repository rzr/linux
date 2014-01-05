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
#include <gtDrvConfig.h>
#include <gtHwCntl.h>
#include "mvEthPhy.h"
#include "mvOs.h"

#include "mv_platform.h"
#include "mvBoardEnvSpec.h"
/*
 * A system configuration structure
 * It used to configure the QD driver with configuration data
 * and with platform specific implementation functions
 */
GT_SYS_CONFIG cfg;

/*
 * The QD device.
 * This struct is a logical representation of the QD switch HW device.
 */
GT_QD_DEV qddev; 
GT_QD_DEV* qd_dev;

/*
 * A phy patch for deviceId == GT_88E6063
 */
static GT_STATUS phyPatch(GT_QD_DEV *dev)
{
  GT_U32 u32Data;
  /*
   * Set Bit2 of Register 29 of any phy
   */
  if (gsysReadMiiReg(dev, dev->baseRegAddr,29,&u32Data) != GT_OK) {
    return GT_FAIL;
  }

  if (gsysWriteMiiReg(dev, (GT_U32)dev->baseRegAddr,29,(GT_U16)(u32Data|0x4)) != GT_OK) {
    return GT_FAIL;
  }

  /*
   * ReSet Bit6 of Register 30 of any phy
   */
  if (gsysReadMiiReg(dev,dev->baseRegAddr,30,&u32Data) != GT_OK) {
    return GT_FAIL;
  }

  if (gsysWriteMiiReg(dev, (GT_U32)dev->baseRegAddr,30,(GT_U16)(u32Data&(~0x40))) != GT_OK) {
    return GT_FAIL;
  }
  return GT_OK;
}


static GT_STATUS qdStart(GT_QD_DEV* dev)
{
  GT_STATUS status;
    mvOsPrintf("%s: CPU port 0x%x \n", __FUNCTION__, cfg.cpuPortNum);

  
  if ((status = qdLoadDriver(&cfg, dev)) != GT_OK) {
    mvOsPrintf("qdLoadDriver failed: status = 0x%x\n", status);
    return status;
  }
  
  /*
   *  start the QuarterDeck
   */

  /* bchaikin: hack to restore the default value corrupted by MIPSBoot ver 1.0 */
  if ((status = hwSetGlobalRegField(dev, QD_REG_GLOBAL_CONTROL, 0, 16, 0x81)) != GT_OK) {
    mvOsPrintf("hwSetGlobalRegField failed.\n");
    return status;
  }
  
  if (dev->deviceId == GT_88E6063) {
    phyPatch(dev);
  }

  /* to which VID should we set the CPU_PORT? (1 is temporary)*/
  if ((status = gvlnSetPortVid(dev, cfg.cpuPortNum, 1)) != GT_OK) {
    mvOsPrintf("gprtSetPortVid fail for CPU port.\n");
    return status;
  }
  
#ifdef QD_TRAILER_MODE

  /* set ingress and egress trailer mode*/
  gprtSetIngressMode(dev, cfg.cpuPortNum, GT_TRAILER_INGRESS);	
  gprtSetTrailerMode(dev, cfg.cpuPortNum, GT_TRUE);
#endif
  
#ifdef QD_HEADER_MODE
   gsysSetOldHader(dev, GT_TRUE);
  if (GT_OK != gprtSetHeaderMode(dev, cfg.cpuPortNum, GT_TRUE))
  {
    mvOsPrintf("Error: gprtSetHeaderMode failed \n");
  }
#endif

  return GT_OK;
}
GT_BOOL ReadMiiWrap(GT_QD_DEV* dev,
		    unsigned int portNumber,
		    unsigned int MIIReg,
		    unsigned int* value)
{
    if (mvEthPhyRegRead(portNumber, MIIReg , (MV_U16 *)value) == MV_OK)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}
GT_BOOL WriteMiiWrap(GT_QD_DEV* dev,
		    unsigned int portNumber,
		    unsigned int MIIReg,
		    unsigned int data)
{
    if (mvEthPhyRegWrite(portNumber, MIIReg ,data) == MV_OK)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

GT_STATUS qdInit(void)
{
  GT_STATUS status = GT_OK;	
  unsigned int i;
    mvOsPrintf("%s \n", __FUNCTION__);

  /*
   *  Register all the required functions to QuarterDeck Driver.
   */
  cfg.BSPFunctions.readMii   = ReadMiiWrap;
  cfg.BSPFunctions.writeMii  = WriteMiiWrap;
#ifdef USE_SEMAPHORE
  cfg.BSPFunctions.semCreate = osSemCreate;
  cfg.BSPFunctions.semDelete = osSemDelete;
  cfg.BSPFunctions.semTake   = osSemWait;
  cfg.BSPFunctions.semGive   = osSemSignal;
#else /* USE_SEMAPHORE */
  cfg.BSPFunctions.semCreate = NULL;
  cfg.BSPFunctions.semDelete = NULL;
  cfg.BSPFunctions.semTake   = NULL;
  cfg.BSPFunctions.semGive   = NULL;
#endif /* USE_SEMAPHORE */
  
  cfg.initPorts = GT_TRUE;
    if (mvBoardIdGet() == RD_88F5181L_VOIP_GE)
    {
        cfg.cpuPortNum = 3;
        cfg.mode.scanMode = SMI_MANUAL_MODE;
    }
    else
    {
        cfg.cpuPortNum = 5;
    }
    
  qd_dev = &qddev; /* internal */
  status = qdStart(qd_dev);
  if (GT_OK != status) {
    mvOsPrintf("Internal switch startup failure: status = 0x%x\n", status);
    return status;
  }
  if (mvBoardIdGet() == RD_88F5181L_VOIP_GE)
  {
      gsysSetPPUEn(qd_dev, GT_TRUE);
  } 
  for (i = 0; i < qd_dev->numOfPorts; i++) {
    /* default port prio to three */
    gcosSetPortDefaultTc(qd_dev, i, 3);       
    /* disable IP TOS Prio */
    gqosIpPrioMapEn(qd_dev, i, GT_FALSE);  
    /* disable QOS Prio */
    gqosUserPrioMapEn(qd_dev, i, GT_FALSE);
    /* Force flow control for all ports */
    gprtSetForceFc(qd_dev, i, GT_FALSE);
  }

  gprtClearAllCtr(qd_dev);
  gprtSetCtrMode(qd_dev, 0);

  for (i=0; i<qd_dev->numOfPorts; i++) {
    gprtSetMcRateLimit(qd_dev, i, GT_MC_100_PERCENT_RL);
  }

#ifdef QD_DEBUG
  for (i = 0; i < qd_dev->numOfPorts;; i++) {
    short sdata;
    
    hwReadPortReg(qd_dev, i, 0x4, &sdata);
    mvOsPrintf("Control reg for port[%d] is: %x\n",i, sdata);
    
    hwReadPortReg(qd_dev, i, 0x0, &sdata);
    mvOsPrintf("Status reg for port[%d] is: %x\n",i, sdata);
  }

#endif /* QD_DEBUG */
  mvOsPrintf("Switch driver initialized\n");
  return status;    
}


static const char* qdPortStpStates[] = {
  "DISABLED",
  "BLOCKING",
  "LEARNING",
  "FORWARDING"
};

static char* qdPortListToStr(GT_LPORT* portList,
			     int portListNum,
			     char* portListStr)
{
  int port, idx, strIdx = 0;
  
  for (idx = 0; idx < portListNum; idx++) {
    port = portList[idx];
    sprintf(&portListStr[strIdx], "%d,", port);
    strIdx = strlen(portListStr);
  }  
  portListStr[strIdx] = '\0';
  return portListStr;
}


void qdStatus(GT_QD_DEV* dev) {
  int               port;
  GT_BOOL           linkState;
  GT_PORT_STP_STATE stpState;
  GT_PORT_STAT      counters;
  GT_U16            pvid;
  GT_LPORT          portList[MAX_SWITCH_PORTS];
  GT_U8             portNum;
  char              portListStr[100];
  unsigned int      phy, reg, val;

  mvOsPrintf("Port register table:\n");
  mvOsPrintf("reg\\phy  ");
  for (phy = 0x18; phy < 0x20; phy++) {
    mvOsPrintf("%4x ", phy - 0x18);
  }
  mvOsPrintf("\n");
  
  for (reg = 0; reg < 32; reg++) {
    mvOsPrintf("%04x:    ", reg);
    for (phy = 0x18; phy < 0x20; phy++) {
      ReadMiiWrap(0, phy, reg, &val);
      mvOsPrintf("%04x ", val);
    }
    mvOsPrintf("\n");
  }

  mvOsPrintf("Summary table:\n");
  mvOsPrintf("Port  Link   PVID    Group       State       RxCntr      TxCntr\n\n");
  for (port = 0; port < qd_dev->numOfPorts; port++) {
      gprtGetLinkState(dev, port, &linkState);
      gstpGetPortState(dev, port, &stpState);
      gprtGetPortCtr(dev,port, &counters);
      gstpGetPortState(dev, port, &stpState);
      gvlnGetPortVid(dev, port, &pvid);
      gvlnGetPortVlanPorts(dev, port, portList, &portNum);
      qdPortListToStr(portList, portNum, portListStr); 
      mvOsPrintf(" %d.   %4s    %d     %-10s  %-10s   0x%-8x  0x%-8x\n",
		 port, (linkState==GT_TRUE) ? "UP" : "DOWN",
		 pvid, portListStr, qdPortStpStates[stpState],
		 counters.rxCtr, counters.txCtr);
  }
}
