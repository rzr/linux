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
 * DESCRIPTION:
 *     This file is the OS Layer header, provided for OS independence.
 */

#ifndef MV_UNIMAC_H
#define MV_UNIMAC_H

#include "msApi.h"
#include "mv_platform.h"

#define MV_UNM_VID_DISABLED		0
#define MV_UNM_VID_ISOLATED		(MAX_SWITCH_PORTS+1)

#define	MV_UNM_MAX_VID			(MV_UNM_VID_ISOLATED+1)

typedef int MV_UNM_VID;

typedef struct _MV_UNM_CONFIG 
{
    MV_UNM_VID	vidOfPort[MAX_SWITCH_PORTS];     
} MV_UNM_CONFIG;


/*******************************************************************************
*Description:
*	User application function that supplies configuration to UniMAC Manager
*  The function receives a optional pointer to configuration array 
*	Parameters:
*		unmConfig: pointer to VLAN mapping configuration
*					if this parameter is NULL the default parameters
* Return value:
*		GT_OK	on success
*		GT_FAIL otherwise
********************************************************************************/

extern GT_STATUS getNetConfig(OUT MV_UNM_CONFIG* unmConfig);


/*******************************************************************************
*Description:
*	UniMAC manager intialization function
*  The function receives a optional pointer to configuration array and also 
*	pointers to save and restore configurqation functions
*	Parameters:
*		None
* Return value:
*		GT_SUCCESS		on success
*		GT_BAD_PARAM	if if getNetConfig() returns invalid configuration
*		GT_FAIL			otherwise
********************************************************************************/
GT_STATUS  mvUnmInitialize(void);

/*******************************************************************************
*Description:
*	Retrieve current network configuration
*  The function receives a pointer to return configuration 
*	Parameters:
*		config: pointer to VLAN mapping configuration
* Return value:
*		GT_OK	on success
*		GT_FAIL on error
********************************************************************************/
GT_STATUS	mvUnmGetNetConfig(OUT MV_UNM_CONFIG *config);


/*******************************************************************************
*Description:
*	Move port from isolated VLAN to specific VLAN
*	Parameters:
*		portId:  port number 
*		vlanId:	 VLAN id
* Return value:
* GT_OK			on success
* GT_BAD_PARAM	if a port number is invalid or the port 
*						already belongs to some VLAN
* GT_FAIL		on other failures
*
********************************************************************************/
 
GT_STATUS	mvUnmPortMoveTo(IN int qdPort, IN const MV_UNM_VID vlanId);

/* Create port based VLAN */
GT_STATUS mvUnmCreateVlan(int vid, GT_U32 vlanPortMask);

/* Return port mask belong this vid */
GT_U32		mvUnmGetPortMaskOfVid(MV_UNM_VID vid);
                                   

/* Return vid, that port belong to */
MV_UNM_VID	mvUnmGetVidOfPort(int port);

/* Return total number of vids, without ISOLATED and DISABLED  */
int			mvUnmGetNumOfVlans(void);



#endif /* MV_UNIMAC_H */
