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
#ifndef _FF_NETCNF_H_
#define _FF_NETCNF_H_
//#define ETH_DBG_INFO
#include <mvOs.h>
#include <mv_unimac.h>
/*#include <mv_eth.h>*/

/*
 * This function returns the name of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its name.
 *
 * Outputs:
 * pNameLength - the length of this string's name.
 * pszVlanName - a const pointer to the name.
 *
 * NOTICE: this function returns an unsafe pointer to the string itself
 *         it doesn't copy nor allocate.
 */
VOID           mv_nc_GetVIDName(IN MV_UNM_VID vid,OUT unsigned int *pNameLength,OUT char **pszVlanName);

/*
 * This function returns the MAC of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its mac.
 *
 * Outputs:
 * A pointer to the mac address (bytes array) of this vlan.
 *
 */
unsigned char* mv_nc_GetMacOfVlan(IN MV_UNM_VID vid);

/*
 * ----------------------------------------------------------------------------
 * GT_STATUS  getNetConfig(OUT MV_UNM_CONFIG* unmConfig)
 *
 * This function is a part of the unimca manager API.
 * The unimac manager API (mv_unimac_mgr.c) requires a get function 
 * in order to init the net-conf data.
 * While being loaded, the unimac manager request for its init 
 * configuration data with that function.
 *
 * Inputs:
 *
 * Outputs:
 * unmConfig - A pointer to the net config to be filled with the init data.
 *
 */
GT_STATUS      getNetConfig(OUT MV_UNM_CONFIG* unmConfig);


/*
 * ----------------------------------------------------------------------------
 * GT_STATUS  setNetConfig(OUT MV_UNM_CONFIG* unmConfig, 
 *						char names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
 *  					unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE])
 *
 * The initialization phase calls this set function in order to 
 * init private fields of this modules.
 * the private fields holds the net-conf data.
 * a caller to the 'get' function will retreive this data (see above).
 *
 * Inputs:
 * unmConfig - A pointer to the net config with the init data.
 * names     - An array of the vlan names in the net-conf.
 * macs      - An array of mac addresses in the net-conf.
 *
 * Outputs:
 * GT_STATUS - the status of the operation.
 *
 */
GT_STATUS  setNetConfig(OUT MV_UNM_CONFIG* unmConfig, 
						char names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
						unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]);

/*
 * A debug function to print the configuration in the unimac manager.
 * This function should be used to test that the get config, called by the manager
 * inited the manager with the correct data. 
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
void           mv_nc_printConf(void);
#endif

#endif /* _FHNETCNF_H_ */
