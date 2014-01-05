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
/******************************************************************************
* 
* FILENAME:    $Workfile: qdIoctl.c $ 
* 
* qdIoctl.c
*
* DESCRIPTION:
*       QD IOCTL API
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include "mvTypes.h"

#include "msApiDefs.h"
#include "msApiPrototype.h"
#include "qdModule.h"
#include "mv_unm_netconf.h"
#include "msIoctl.h"
/*
 * #define IOCTL_DEBUG
 *
 */

#define ERROR_SUCCESS           (0)
#define ERROR_NOT_SUPPORTED     (-1)
#define ERROR_INVALID_PARAMETER (-2)
#define ERROR_GEN_FAILURE       (-3)


extern GT_QD_DEV* qd_dev;
extern int mv_eth_start(void);
extern void qdStatus(GT_QD_DEV* dev);

static MV_BOOL initialized = /*false*/true;

static void SetLastError(MV_U32 dwErr)
{
  static MV_U32 dwLastError = 0;

  dwLastError = dwErr;
}


/************************************************************************/
/* IOCTL API                                                            */
/************************************************************************/
static MV_BOOL qdUnmNetIoctl(MV_U32  hOpenContext,
			  MV_U32  dwCode,
			  MV_U8 *pInBuf,
			  MV_U32  InBufLen,
			  MV_U8 *pOutBuf,
			  MV_U32  OutBufLen,
			  MV_U32 *pdwBytesTransferred);
static GT_STATUS UNM_GetVlanNames(MV_U8 *pOutBuf,
				  MV_U32  OutBufLen,
				  MV_U32 *pdwBytesTransferred);
static GT_STATUS UNM_GetVlanParams(MV_U8 *pInBuf,
				   MV_U32  InBufLen,
				   MV_U8 *pOutBuf,
				   MV_U32  OutBufLen,
				   MV_U32 *pdwBytesTransferred);
static GT_STATUS UNM_GetPortVlan(MV_U32 port,
				 MV_U8 *pOutBuf,
				 MV_U32  OutBufLen,
				 MV_U32 *pdwBytesTransferred);
static GT_STATUS UNM_DisassocPort(MV_U32 port);
static GT_STATUS UNM_AssocPort(MV_U32 port,
			       WCHAR* szVlanName);

MV_BOOL qdUnmNetIoctl(MV_U32  hOpenContext,
		   MV_U32  dwCode,
		   MV_U8 *pInBuf,
		   MV_U32  InBufLen,
		   MV_U8 *pOutBuf,
		   MV_U32  OutBufLen,
		   MV_U32 *pdwBytesTransferred)
{
  MV_BOOL bRc = false;
  GT_STATUS rc;
  MV_U32 port;

  if (!pdwBytesTransferred) {
    return false;
  }
  *pdwBytesTransferred = 0;

  switch(dwCode) {
    
  case IOCTL_UNM_READ_REG:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    ((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data =
      *(GT_U32*)((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data;
    bRc = true;
    *pdwBytesTransferred = sizeof(GT_U32);
    break;
    
  case IOCTL_UNM_WRITE_REG:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    *(GT_U32*)((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data =
      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data;
    bRc = true;
    *pdwBytesTransferred = 0;
    break;
    
  case IOCTL_UNM_STATUS:
    if (OutBufLen < sizeof(MV_BOOL)) {
      break;
    }
    *pdwBytesTransferred = sizeof(MV_BOOL);
    *(MV_BOOL*)pOutBuf = initialized;
    bRc = true;
    break;
    
  case IOCTL_UNM_INIT:
    if (initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    rc = setNetConfig(&(((GT_IOCTL_PARAM*)pInBuf)->FirstParam.netconf),
		      (((GT_IOCTL_PARAM*)pInBuf)->SecondParam.vlan_names),
		      (((GT_IOCTL_PARAM*)pInBuf)->ThirdParam.macs)); 
    if (rc != GT_OK) {
      break;
    }
    /* init net-device and qd-device */
    mv_eth_start();
    initialized = true;
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_VLAN_NAMES:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < 2)) {
      break;
    }
    rc = UNM_GetVlanNames(pOutBuf, OutBufLen, pdwBytesTransferred);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_VLAN_PARMS:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    rc = UNM_GetVlanParams( pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred);
    if( rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_PORT_VLAN:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < sizeof(GT_IOCTL_PARAM))
	|| (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == qd_dev->cpuPortNum) || (port > qd_dev->numOfPorts)) {
      break;
    }
    rc = UNM_GetPortVlan(port, pOutBuf, OutBufLen, pdwBytesTransferred);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_ASSOC_PORT:
    *pdwBytesTransferred = 0;
    if (!initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      printk("ASSOC: InBufLen != sizeof(GT_IOCTL_PARAM) \n");
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == qd_dev->cpuPortNum) || (port > qd_dev->numOfPorts)) {
      printk("ASSOC: port = 5 || port > 6 \n");
      break;
    }
    rc = UNM_AssocPort(port, ((GT_IOCTL_PARAM*)pInBuf)->SecondParam.szVlanName);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
      
  case IOCTL_UNM_DISASSSOC_PORT:
    if(!initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == qd_dev->cpuPortNum) || (port > qd_dev->numOfPorts)) {
      break;
    }
    rc = UNM_DisassocPort(port);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  default:
    break;
  }
  return bRc;
}


MV_BOOL qdSystemConfig(MV_U32  hOpenContext,
  MV_U32  Ioctl,
	       MV_U8 *pInBuf,
	       MV_U32  InBufLen, 
	       MV_U8 *pOutBuf,
	       MV_U32  OutBufLen,
	       MV_U32 * pdwBytesTransferred
	       )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;
  int devId;

  switch (Ioctl)
    {
    case IOCTL_sysConfig: 
      if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
      qdStatus(qd_dev);
      memcpy( &( ((GT_IOCTL_PARAM*)pOutBuf)->FirstParam.qd_dev), qd_dev, sizeof(GT_QD_DEV));
      break;

    case IOCTL_gsysReadMiiReg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
      if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
      devId = (((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data < 0x10)? 0: 1;
      if(gsysReadMiiReg(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
			) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gsysWriteMiiReg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      devId = (((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data < 0x10)? 0: 1;
      if(gsysWriteMiiReg(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data,
			 ((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.u16Data
			 ) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gtVersion:
      if (InBufLen < VERSION_MAX_LEN)
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gtVersion((GT_VERSION*)pInBuf) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = strlen(pInBuf);
      break;

    case IOCTL_gLoadDriver:
      if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
      if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      if(qdLoadDriver(&((PGT_IOCTL_PARAM)pInBuf)->FirstParam.sysCfg,
		     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.qd_dev
		     ) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_QD_DEV);
      break;

    case IOCTL_gUnloadDriver:
      if(qdUnloadDriver(qd_dev) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gSysEnable:
      if(sysEnable(qd_dev) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

MV_BOOL
qdFdbIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gfdbSetAtuSize: 
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbSetAtuSize(qd_dev,((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuSize) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbGetAgingTimeRange:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAgingTimeRange(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data,
			       &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.u32Data
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 2*sizeof(GT_U32);
      break;

    case IOCTL_gfdbSetAgingTimeout:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbSetAgingTimeout(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbGetAtuDynamicCount:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuDynamicCount(qd_dev,
				&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
				) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gfdbGetAtuEntryFirst:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuEntryFirst(qd_dev,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbGetAtuEntryNext:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuEntryNext(qd_dev,
			     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
	     sizeof(GT_ATU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbFindAtuMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbFindAtuMacEntry(qd_dev,
			     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
			     &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
	     sizeof(GT_ATU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbFlush:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbFlush(qd_dev,
		   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.flushCmd
		   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbAddMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbAddMacEntry(qd_dev,
			 &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbDelMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbDelMacEntry(qd_dev,
			 &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.etherAddr
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbLearnEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbLearnEnable(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbDelAtuEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbDelAtuEntry(qd_dev,
			 &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbFlushInDB:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbFlushInDB(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.flushCmd,
	                 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdStpIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gstpSetMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpSetMode(qd_dev,
		     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
		     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstpSetPortState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpSetPortState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.stpState
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstpGetPortState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpGetPortState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.stpState
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_PORT_STP_STATE);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdVlanIoctl(
	    MV_U32  hOpenContext,
	    MV_U32  Ioctl,
	    MV_U8 *pInBuf,
	    MV_U32  InBufLen, 
	    MV_U8 *pOutBuf,
	    MV_U32  OutBufLen,
	    MV_U32 * pdwBytesTransferred
	    )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl) {
  case IOCTL_gprtSetEgressMode: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetEgressMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.egressMode
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gprtGetEgressMode:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtGetEgressMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.egressMode
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_EGRESS_MODE);
    break;

  case IOCTL_gprtSetVlanTunnel: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetVlanTunnel(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gprtGetVlanTunnel:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtGetVlanTunnel(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;


  case IOCTL_gvlnSetPortVlanPorts: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVlanPorts(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.portList,
			    ((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVlanPorts:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVlanPorts(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.portList,
			    &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
    break;

  case IOCTL_gvlnSetPortUserPriLsb: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortUserPriLsb(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			     ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortUserPriLsb:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortUserPriLsb(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			     ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;


  case IOCTL_gvlnSetPortVid: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVid(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u16Data
		      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVid:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVid(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
		      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_U16);
    break;

  case IOCTL_gvlnSetPortVlanDBNum:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVlanDBNum(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVlanDBNum:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVlanDBNum(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_U8);
    break;

  case IOCTL_gvlnSetPortVlanDot1qMode:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVlanDot1qMode(qd_dev,
				((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				((PGT_IOCTL_PARAM)pInBuf)->SecondParam.dot1QMode
				) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVlanDot1qMode:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVlanDot1qMode(qd_dev,
				((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.dot1QMode
				) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_DOT1Q_MODE);
    break;

  case IOCTL_gvlnSetPortVlanForceDefaultVID:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVlanForceDefaultVID(qd_dev,
				      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
				      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVlanForceDefaultVID:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVlanForceDefaultVID(qd_dev,
				      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;

  default:
    break;
  }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdSysEventIoctl(
		MV_U32  hOpenContext,
		MV_U32  Ioctl,
		MV_U8 *pInBuf,
		MV_U32  InBufLen, 
		MV_U8 *pOutBuf,
		MV_U32  OutBufLen,
		MV_U32 * pdwBytesTransferred
		)
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_eventSetActive:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(eventSetActive(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_eventGetIntStatus:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(eventGetIntStatus(qd_dev,
			   &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

MV_BOOL
qdPhyCtrlIoctl(
	       MV_U32  hOpenContext,
	       MV_U32  Ioctl,
	       MV_U8 *pInBuf,
	       MV_U32  InBufLen, 
	       MV_U8 *pOutBuf,
	       MV_U32  OutBufLen,
	       MV_U32 * pdwBytesTransferred
	       )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtPhyReset:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPhyReset(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortLoopback:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortLoopback(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortSpeed:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortSpeed(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortAutoNegEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortAutoNegEnable(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortPowerDown:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortPowerDown(qd_dev,
			   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			   ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortRestartAutoNeg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortRestartAutoNeg(qd_dev,
				((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port
				) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortDuplexMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortDuplexMode(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortAutoMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortAutoMode(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.phyAutoMode
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPause(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

MV_BOOL
qdPhyIntIoctl(
	      MV_U32  hOpenContext,
	      MV_U32  Ioctl,
	      MV_U8 *pInBuf,
	      MV_U32  InBufLen, 
	      MV_U8 *pOutBuf,
	      MV_U32  OutBufLen,
	      MV_U32 * pdwBytesTransferred
	      )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtPhyIntEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPhyIntEnable(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u16Data
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetPhyIntStatus:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyIntStatus(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    case IOCTL_gprtGetPhyIntPortSummary:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyIntPortSummary(qd_dev,
				  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
				  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

MV_BOOL
qdPortCtrlIoctl(
		MV_U32  hOpenContext,
		MV_U32  Ioctl,
		MV_U8 *pInBuf,
		MV_U32  InBufLen, 
		MV_U8 *pOutBuf,
		MV_U32  OutBufLen,
		MV_U32 * pdwBytesTransferred
		)
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtSetForceFc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetForceFc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetForceFc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetForceFc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtSetTrailerMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetTrailerMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetTrailerMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetTrailerMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtSetIngressMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetIngressMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.ingressMode
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetIngressMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetIngressMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.ingressMode
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_INGRESS_MODE);
      break;


    case IOCTL_gprtSetMcRateLimit:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetMcRateLimit(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.mcRate
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetMcRateLimit:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetMcRateLimit(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.mcRate
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_MC_RATE);
      break;

  case IOCTL_gprtSetHeaderMode: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetHeaderMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gprtGetHeaderMode:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }

    if(gprtGetHeaderMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;

  case IOCTL_gprtSetIGMPSnoop:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetIGMPSnoop(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;
    
  case IOCTL_gprtGetIGMPSnoop:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtGetIGMPSnoop(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;
    
  case IOCTL_gprtSetDuplex: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetDuplex(qd_dev,
		     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
		     ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdPortStatsIoctl(
		 MV_U32  hOpenContext,
		 MV_U32  Ioctl,
		 MV_U8 *pInBuf,
		 MV_U32  InBufLen, 
		 MV_U8 *pOutBuf,
		 MV_U32  OutBufLen,
		 MV_U32 * pdwBytesTransferred
		 )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtSetCtrMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetCtrMode(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.ctrMode
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtClearAllCtr:

      if(gprtClearAllCtr(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetPortCtr:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPortCtr(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.portStat
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_PORT_STAT);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdPortStatusIoctl(
		  MV_U32  hOpenContext,
		  MV_U32  Ioctl,
		  MV_U8 *pInBuf,
		  MV_U32  InBufLen, 
		  MV_U8 *pOutBuf,
		  MV_U32  OutBufLen,
		  MV_U32 * pdwBytesTransferred
		  )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtGetPartnerLinkPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPartnerLinkPause(qd_dev,
				 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetSelfLinkPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetSelfLinkPause(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetResolve:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetResolve(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetLinkState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetLinkState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetPortMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPortMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetPhyMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyMode(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetDuplex:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetDuplex(qd_dev,
		       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
		       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetSpeed:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetSpeed(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdQoSMapIoctl(
	      MV_U32  hOpenContext,
	      MV_U32  Ioctl,
	      MV_U8 *pInBuf,
	      MV_U32  InBufLen, 
	      MV_U8 *pOutBuf,
	      MV_U32  OutBufLen,
	      MV_U32 * pdwBytesTransferred
	      )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gcosSetPortDefaultTc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetPortDefaultTc(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gcosGetPortDefaultTc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosGetPortDefaultTc(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U8);
      break;

    case IOCTL_gqosSetPrioMapRule:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosSetPrioMapRule(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetPrioMapRule:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetPrioMapRule(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gqosIpPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosIpPrioMapEn(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetIpPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetIpPrioMapEn(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gqosUserPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosUserPrioMapEn(qd_dev,
			   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			   ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetUserPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetUserPrioMapEn(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;


    case IOCTL_gcosSetUserPrio2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetUserPrio2Tc(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gcosGetUserPrio2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosGetUserPrio2Tc(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U8);
      break;

    case IOCTL_gcosSetDscp2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetDscp2Tc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gcosGetDscp2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosGetDscp2Tc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U8);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



MV_BOOL
qdSysCtrlIoctl(
	       MV_U32  hOpenContext,
	       MV_U32  Ioctl,
	       MV_U8 *pInBuf,
	       MV_U32  InBufLen, 
	       MV_U8 *pOutBuf,
	       MV_U32  OutBufLen,
	       MV_U32 * pdwBytesTransferred
	       )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gsysSwReset:

      if(gsysSwReset(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysSetDiscardExcessive:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetDiscardExcessive(qd_dev,
				 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetDiscardExcessive:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetDiscardExcessive(qd_dev,
				 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetSchedulingMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetSchedulingMode(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetSchedulingMode:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetSchedulingMode(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetMaxFrameSize:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetMaxFrameSize(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetMaxFrameSize:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetMaxFrameSize(qd_dev,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysReLoad:

      if(gsysReLoad(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysSetWatchDog:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetWatchDog(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetWatchDog:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetWatchDog(qd_dev,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetDuplexPauseMac:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetDuplexPauseMac(qd_dev,
			       &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.etherAddr
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetDuplexPauseMac:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetDuplexPauseMac(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.etherAddr
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ETHERADDR);
      break;

    case IOCTL_gsysSetPerPortDuplexPauseMac:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetPerPortDuplexPauseMac(qd_dev,
				      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
				      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetPerPortDuplexPauseMac:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetPerPortDuplexPauseMac(qd_dev,
				      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdStatsSysIoctl(
		MV_U32  hOpenContext,
		MV_U32  Ioctl,
		MV_U8 *pInBuf,
		MV_U32  InBufLen, 
		MV_U8 *pOutBuf,
		MV_U32  OutBufLen,
		MV_U32 * pdwBytesTransferred
		)
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gsysGetSW_Mode:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      if(gsysGetSW_Mode(qd_dev,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.swMode
			) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_SW_MODE);
      break;

    case IOCTL_gsysGetInitReady:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      if(gsysGetInitReady(qd_dev,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			  ) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



MV_BOOL
qdPavIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gpavSetPAV:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gpavSetPAV(qd_dev,
		    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u16Data
		    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;
    
    case IOCTL_gpavGetPAV:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gpavGetPAV(qd_dev,
		    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
		    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    case IOCTL_gpavSetIngressMonitor:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gpavSetIngressMonitor(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gpavGetIngressMonitor:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gpavGetIngressMonitor(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



MV_BOOL
qdPrcIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_grcSetLimitMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetLimitMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.rateLimitMode
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;
    
    case IOCTL_grcGetLimitMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetLimitMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.rateLimitMode
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_RATE_LIMIT_MODE);
      break;

    case IOCTL_grcSetEgressRate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetEgressRate(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.egressRate
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;
    
    case IOCTL_grcGetEgressRate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetEgressRate(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.egressRate
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_EGRESS_RATE);
      break;

    case IOCTL_grcSetPri3Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetPri3Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_grcGetPri3Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetPri3Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_grcSetPri2Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetPri2Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_grcGetPri2Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetPri2Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_grcSetPri1Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetPri1Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_grcGetPri1Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetPri1Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_grcSetPri0Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetPri0Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_grcGetPri0Rate:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetPri0Rate(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_grcSetBytesCount:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcSetBytesCount(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData,
			  ((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.boolData,
			  ((PGT_IOCTL_PARAM)pInBuf)->FourthParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_grcGetBytesCount:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(grcGetBytesCount(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData,
			  &((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.boolData,
			  &((PGT_IOCTL_PARAM)pInBuf)->FourthParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData,
	     &((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData,
	     sizeof(GT_BOOL)
	     );

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.boolData,
	     &((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.boolData,
	     sizeof(GT_BOOL)
	     );

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->ThirdParam.boolData,
	     &((PGT_IOCTL_PARAM)pInBuf)->FourthParam.boolData,
	     sizeof(GT_BOOL)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = (3 * sizeof(GT_BOOL));
      break;
      
    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


MV_BOOL
qdVtuIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gvtuGetIntStatus:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gvtuGetIntStatus(qd_dev,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.vtuIntStatus
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_VTU_INT_STATUS);
      break;

    case IOCTL_gvtuGetEntryCount:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gvtuGetEntryCount(qd_dev,
			   &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gvtuGetEntryFirst:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gvtuGetEntryFirst(qd_dev,
			   &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.vtuEntry
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_VTU_ENTRY);
      break;

    case IOCTL_gvtuGetEntryNext:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gvtuGetEntryNext(qd_dev,
			  &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.vtuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry,
	     sizeof(GT_VTU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_VTU_ENTRY);
      break;

    case IOCTL_gvtuFindVidEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gvtuFindVidEntry(qd_dev,
			  &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry,
			  &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.vtuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry,
	     sizeof(GT_VTU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_VTU_ENTRY);
      break;

    case IOCTL_gvtuFlush:
      if(gvtuFlush(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gvtuAddEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gvtuAddEntry(qd_dev,
		      &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gvtuDelEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gvtuDelEntry(qd_dev,
		      &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.vtuEntry
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



MV_BOOL
qdStatsRmonIoctl(
		 MV_U32  hOpenContext,
		 MV_U32  Ioctl,
		 MV_U8 *pInBuf,
		 MV_U32  InBufLen, 
		 MV_U8 *pOutBuf,
		 MV_U32  OutBufLen,
		 MV_U32 * pdwBytesTransferred
		 )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gstatsFlushAll:
      if(gstatsFlushAll(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstatsFlushPort:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gstatsFlushPort(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstatsGetPortCounter:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gstatsGetPortCounter(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.statsCounter,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gstatsGetPortAllCounters:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gstatsGetPortAllCounters(qd_dev,
				  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.statsCounterSet
				  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_STATS_COUNTER_SET);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



MV_BOOL
qdVctIoctl(
	   MV_U32  hOpenContext,
	   MV_U32  Ioctl,
	   MV_U8 *pInBuf,
	   MV_U32  InBufLen, 
	   MV_U8 *pOutBuf,
	   MV_U32  OutBufLen,
	   MV_U32 * pdwBytesTransferred
	   )
{
  MV_U32  dwErr = ERROR_NOT_SUPPORTED;
  MV_BOOL   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gvctGetCableDiag:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gvctGetCableDiag(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.cableStatus
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_CABLE_STATUS);
      break;

    case IOCTL_gvctGet1000BTExtendedStatus:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gvctGet1000BTExtendedStatus(qd_dev,
				     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.extStatus
				     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_1000BT_EXTENDED_STATUS);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }
 
 return bRc;
}





MV_BOOL
UNM_IOControl(
	      MV_U32  hOpenContext,
	      MV_U32  Ioctl,
	      MV_U8 *pInBuf,
	      MV_U32  InBufLen, 
	      MV_U8 *pOutBuf,
	      MV_U32  OutBufLen,
	      MV_U32 *pdwRet
	      )
{
  MV_BOOL   bRc = false;
    
  switch (SUB_FUNC_MASK & GET_FUNC_FROM_CTL_CODE(Ioctl)) {
  case SYS_CFG_FUNC_MASK:
    bRc = qdSystemConfig(hOpenContext, Ioctl,pInBuf, InBufLen, 
			 pOutBuf,OutBufLen,pdwRet);
    break;
      
  case FDB_FUNC_MASK:
    bRc = qdFdbIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen, pdwRet);
    break;
      
  case STP_FUNC_MASK:
    bRc = qdStpIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case VLAN_FUNC_MASK:
    bRc = qdVlanIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		      pOutBuf, OutBufLen,pdwRet);       

    break;
      
  case SYS_EVENT_FUNC_MASK:
    bRc = qdSysEventIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			  pOutBuf, OutBufLen,pdwRet);       
    break;

  case PHY_CTRL_FUNC_MASK:
    bRc = qdPhyCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			 pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PHY_INT_FUNC_MASK:
    bRc = qdPhyIntIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_CTRL_FUNC_MASK:
    bRc = qdPortCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			  pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_STATS_FUNC_MASK:
    bRc = qdPortStatsIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			   pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_STATUS_FUNC_MASK:
    bRc = qdPortStatusIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			    pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case QOS_FUNC_MASK:
    bRc = qdQoSMapIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case SYS_CTRL_FUNC_MASK:
    bRc = qdSysCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			 pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PAV_FUNC_MASK:
    bRc = qdPavIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;

  case PRC_FUNC_MASK:
    bRc = qdPrcIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;

  case VTU_FUNC_MASK:
    bRc = qdVtuIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;

  case STATS_RMON_FUNC_MASK:
    bRc = qdStatsRmonIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			   pOutBuf, OutBufLen,pdwRet);       
    break;

  case STATS_SYS_FUNC_MASK:
    bRc = qdStatsSysIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			  pOutBuf, OutBufLen,pdwRet);       
    break;

  case VCT_FUNC_MASK:
    bRc = qdVctIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;

  case UNM_NET_FUNC_MASK:
    bRc = qdUnmNetIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;

  default:
    printk("Unsupported IOCTL code\n");
    break;
  }
    
  return bRc;
}


/* IOCTL - replies from UNM
 *
 * we need to access the global NETCONFIG (NetConfig) structure
 * we will do it here
 * 
 * its not so OOD, but in future NetConf lib, QD and IM will be one DLL!
 */
GT_STATUS UNM_GetVlanNames(MV_U8 *pOutBuf,  MV_U32  OutBufLen, MV_U32 *pdwBytesTransferred)
{
  GT_STATUS rc = GT_OK;
  UINT i;
  MV_U32 bytesCtr = 0, bytesPerName;
  MV_U8 *pOutTemp = pOutBuf;
  WCHAR szNullChar[1] = { '\0' }; 
  WCHAR *pszVlanName; 

  do {
    
    if( OutBufLen < 2 ) {
      *pdwBytesTransferred = 0;
      rc = GT_FAIL;
      break;
    }
		
    for( i=1; i <= mvUnmGetNumOfVlans() ; i++ ) {
      mv_nc_GetVIDName(i, &bytesPerName, &pszVlanName);
      bytesPerName += 1; /* for the '\0' wchar */
      bytesCtr += bytesPerName;
      if(bytesCtr > OutBufLen ) {
	rc = GT_FAIL;
	*pdwBytesTransferred = 0;
	break;
      }
      memcpy(pOutTemp, pszVlanName, bytesPerName );
      pOutTemp[bytesPerName-1] = '\0';
      pOutTemp += bytesPerName;
    }
    if(rc != GT_OK)
      break;
		
    /* okay, now lets add the last '\0' */
    if( bytesCtr == 0 ) {
      ((WCHAR*)pOutBuf)[0] = '\0';
      *pdwBytesTransferred = 1;
      break;
    }
		
    bytesCtr += 1;
    if(bytesCtr > OutBufLen ) {
      rc = GT_FAIL;
      *pdwBytesTransferred = 0;
      break;
    }
    memcpy(pOutTemp, szNullChar, 1);
    *pdwBytesTransferred = bytesCtr;
  }while(0);
  return rc;

}

GT_STATUS 
UNM_GetVlanParams(MV_U8 *pInBuf,
		  MV_U32  InBufLen,
		  MV_U8 *pOutBuf,
		  MV_U32  OutBufLen,
		  MV_U32 *pdwBytesTransferred 
		  )
{
  MV_U32 bytesPerName;
  GT_STATUS rc = GT_OK;
  UINT i;
  MV_BOOL found = false;
  WCHAR *pszVlanName;   
  GT_IOCTL_PARAM *pOutIoctlParam;
  do {
    for( i=1; i <= MV_UNM_MAX_VID; i++ ) {
      if( i <= mvUnmGetNumOfVlans() ) {
	mv_nc_GetVIDName(i, &bytesPerName, &pszVlanName);
	if( !memcmp( pInBuf, pszVlanName, InBufLen) ) {
	  found = true;
	  break;
	}
      }
    }
    if(!found) {
      rc = GT_FAIL;
      break;
    }
    pOutIoctlParam = (GT_IOCTL_PARAM *)pOutBuf;
    memmove( pOutIoctlParam->FirstParam.etherAddr.arEther , 
	     mv_nc_GetMacOfVlan(i),
	     GT_ETHERNET_HEADER_SIZE  );
    /* memmove( pOutBuf, &(NetConfig.CpuVlans[i].NetParams), sizeof(VLAN_NET_PARAMS) ); */
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
  }while(0);
  return rc;
}

GT_STATUS 
UNM_GetPortVlan( MV_U32  port,
		 MV_U8 *pOutBuf,
		 MV_U32  OutBufLen,
		 MV_U32 *pdwBytesTransferred 
		 )
{
  MV_U32 bytesPerName;
  WCHAR *pszVlanName; 

  GT_STATUS rc = GT_OK;
  MV_U32 vid;
  GT_IOCTL_PARAM *pOutIoctlParam;
  do {
    
    vid = (MV_U32)mvUnmGetVidOfPort(port);
    if( vid == 0 ) {
      /* this port is not belong to any VLAN */
      *pdwBytesTransferred = 0;
      break;
    }

    pOutIoctlParam = (GT_IOCTL_PARAM *)pOutBuf;
    mv_nc_GetVIDName(vid, &bytesPerName, &pszVlanName);
    /* memcpy(pOutBuf, NetConfig.CpuVlans[vid].szVlanName, NetConfig.CpuVlans[vid].NameLength + 2 ); */
    memmove( pOutIoctlParam->SecondParam.szVlanName, pszVlanName, bytesPerName + 1);
    pOutIoctlParam->SecondParam.szVlanName[bytesPerName] = '\0';
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM); /* NetConfig.CpuVlans[vid].NameLength + 1; */

  }while(0);
  return rc;
}



GT_STATUS 
UNM_DisassocPort( MV_U32 port )
{
  GT_STATUS status;
  MV_UNM_VID vidFrom;
  GT_STATUS rc = GT_OK;

  do {
    /* 1. check validity */
    if( port == qd_dev->cpuPortNum || port > qd_dev->numOfPorts )
      {
	rc = GT_FAIL;
	break;
      }
		
    vidFrom = (MV_U32)mvUnmGetVidOfPort(port);

    mvOsAssert( vidFrom <= mvUnmGetNumOfVlans() );
		
    /* if its allready free just return a success */
    if( vidFrom == 0 )
      {
	rc = GT_OK;
	break;		
      }

    /* in order to free a port we need to do the follwoing:
     * QD
     *    1. close this port in the switch
     *    2. if its connected to the cpu, disconnect it from cpu table
     *    3. erase its VLAN table (????)
     * IM
     *    4. find its vlan
     *    5. adjust the trailer of its vlan
     * NETCONF
     *    6. adjust netconf structure
     *    7. update registry (???)
     *		
     * Done with in unimac manager
     * gstpSetPortState(qd_dev, port, GT_PORT_DISABLE);
     *	
     * we need to update netconf and recalculate LAN_PORTS_MODE
     * this port is not belong to any vlan now
     */	
    status = mvUnmPortMoveTo(port, MV_UNM_VID_ISOLATED);
    if(status != GT_OK)
      {
	rc = GT_FAIL;
	/* TODO: rollback???? */
	break;
		
      }
    /* TODO: update registry!!! */

			
  }while(0);
	
  return rc;

}
GT_STATUS 
UNM_AssocPort( MV_U32 port, WCHAR* szVlanName)
{
  
  MV_UNM_VID vidTo = 0, vidFrom = 0;
  GT_STATUS rc = GT_OK;
  UINT i, uiNumOfVlans;
  WCHAR *pszVlnaName;
  UINT uiVlanNameLen;

  do {
    uiNumOfVlans = mvUnmGetNumOfVlans();
    /* 1. check validity */
    if( port == qd_dev->cpuPortNum || port > qd_dev->numOfPorts )
      {
	rc = GT_FAIL;
	break;
      }
    /* do we have this VLAN? */
    for( i=1 ; i <= uiNumOfVlans; i++ )
      {
	mv_nc_GetVIDName(i, &uiVlanNameLen, &pszVlnaName);
	if( !memcmp( szVlanName, pszVlnaName, uiVlanNameLen) ) {
	  vidTo = i;
	}
      }
		
    if( vidTo == 0 ) /* couldn't find the VLAN name to add to */
      {
	rc = GT_FAIL;
	break;		
      }

    vidFrom = (MV_U32)mvUnmGetVidOfPort(port);
    mvOsAssert( vidFrom <= mvUnmGetNumOfVlans() );
    if( vidFrom == vidTo)
      {
	rc = GT_OK;
	break;		
      }
		
		
    /* if its not free we first need to free the port */
    if( vidFrom != 0 )
      {
	rc = UNM_DisassocPort(port);
	if( rc != GT_OK )
	  {
	    rc = GT_FAIL;
	    break;		
	  }
      }
    /* okay, we have a free port and a valid vidTo to add it to
     * in order to free a port we need to do the follwoing:
     * NETCONF
     *    1. adjust netconf structure
     * IM
     *    2. find its vlan
     *    3. adjust the trailer of its vlan
     * QD
     *    1. update the VLAN table of this port
     *	  2. connect it to the cpu
     *    3. open this port in the switch
     * NETCONF
     *    7. update registry (???)
     *	
     * we need to update netconf and recalculate LAN_PORTS_MODE
     * this port is not belong to any vlan now
     */
    rc = mvUnmPortMoveTo(port, vidTo);
    if(rc != GT_OK)
      break;



    /* gstpSetPortState(qd_dev,port, GT_PORT_FORWARDING); */

    /* TODO: update registry!!! */
			
  }while(0);
	
  return rc;


}




