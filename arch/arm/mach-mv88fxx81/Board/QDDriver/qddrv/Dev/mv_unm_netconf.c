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
/*******************************************************************************
* 
* FILENAME:    $Workfile: mv_unm_netconf.c $ 
* 
* DESCRIPTION:
*       	This file holds common definition for dynamic network configuration OS WRRAPER
*	        A unimac manager wrraper to support VLAN names and MACs properties,
*               and to init the net-config structure 
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "mv_unm_netconf.h"
#include "mv_unimac.h"
extern GT_QD_DEV* qd_dev;

/* is the net-config has been inited allready? */
static MV_BOOL inited = false;

/* the null-mac is a comparison helper to find un initialized MAC addresses */
static unsigned char null_mac[GT_ETHERNET_HEADER_SIZE] = {0};

/* the net-config (nc) file, are structures that have been parsed from a 'file' 
 * the setNetConfig initialize all of those structures
 * the three structures holds the net-conf that have already parsed
 */
static unsigned char mv_nc_file_macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]; 
static char          mv_nc_file_names[MV_UNM_MAX_VID][MAX_VLAN_NAME]; 
static MV_UNM_CONFIG file_cnf;

/* the net-config (nc) , are structures holds VALID info for netconf
 * the getNetConfig functions fill those structures from the 
 * allready-parsed nc_file structures
 */
static unsigned char	mv_nc_macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE];
static MV_8	        mv_nc_vlan_names[MV_UNM_MAX_VID][MAX_VLAN_NAME];



/*
 * ----------------------------------------------------------------------------
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
VOID mv_nc_GetVIDName(IN MV_UNM_VID vid,OUT unsigned int *pNameLength,OUT char **pszVlanName) {
  *pNameLength = strlen(mv_nc_vlan_names[vid]);
  *pszVlanName = mv_nc_vlan_names[vid];  
}

/*
 * ----------------------------------------------------------------------------
 * This function returns the MAC of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its mac.
 *
 * Outputs:
 * A pointer to the mac address (bytes array) of this vlan.
 *
 */
unsigned char* mv_nc_GetMacOfVlan(IN MV_UNM_VID vid) {
  return mv_nc_macs[vid];
}


/*
 * ----------------------------------------------------------------------------
 * This function prints the port association table of the unimac manger.
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
void mv_nc_printConf(void)
{
  MV_UNM_CONFIG	cfg;
  GT_STATUS		status = mvUnmGetNetConfig(&cfg);
  int i;
  
  if (status == GT_OK) {
	mvOsPrintf ("Port association table:\n"); 
	mvOsPrintf("---------------------------------------\n");
	mvOsPrintf("| Port |");
	for (i = 0; i < (qd_dev->numOfPorts); i++) {
	  mvOsPrintf("%4d |", i);
	}		
	mvOsPrintf("\n| VID  |");
	for (i = 0; i < (qd_dev->numOfPorts); i++) {
	  mvOsPrintf("%4d |", cfg.vidOfPort[i]);
	}
	mvOsPrintf("\n---------------------------------------\n");
  }
  else {
    mvOsPrintf("Configuration error %d.\n", status);
  }
}
#endif /* ETH_DBG_INFO */

/*
 * ----------------------------------------------------------------------------
 * printUnimacStructs - prints a given configuration.
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
static void printUnimacStructs(OUT MV_UNM_CONFIG* _cnf, 
			char _names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
			unsigned char _macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]) 
{
  int i;
  for(i=0;i<MV_UNM_MAX_VID;i++) {
    mvOsPrintf("[%d] - MAC=%x%x%x%x%x%x Name=%s\n",i, 
	       _macs[i][0], 
	       _macs[i][1],
	       _macs[i][2],
	       _macs[i][3],
	       _macs[i][4],
	       _macs[i][5], 
	       _names[i]);
  }
  for(i=0;i<qd_dev->numOfPorts;i++) {
    mvOsPrintf(" %d", _cnf->vidOfPort[i]);
  }
  mvOsPrintf("\n");
}
#endif /* ETH_DBG_INFO */

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
						unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE])

{
  /* init all structures to zero */
  memset(mv_nc_file_macs, 0, sizeof(mv_nc_file_macs));
  memset(mv_nc_file_names, 0, sizeof(mv_nc_file_names));; 
  memset(&file_cnf, 0, sizeof(file_cnf));
  
  /* set net-conf by structure copy 
   * this is only updates the 'file' structures, thos will be parsed
   * in the getNetConfig to initialize unimac manger
   */
  memcpy(mv_nc_file_macs, macs, sizeof(mv_nc_file_macs));
  memcpy(mv_nc_file_names, names, sizeof(mv_nc_file_names)); 
  memcpy(&file_cnf, unmConfig, sizeof(file_cnf));
  
  inited = true;

#ifdef ETH_DBG_INFO
  printUnimacStructs(&file_cnf, mv_nc_file_names, mv_nc_file_macs );
#endif /* ETH_DBG_INFO */

  return (GT_OK);

}

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
GT_STATUS  getNetConfig(OUT MV_UNM_CONFIG* unmConfig)
{
  int i;

  /* Let's fill it with the following defaults for now:
     WAN - port 0.
     LAN - ports 1,2,3,4.
  */
  memset( mv_nc_macs, 0 , sizeof(mv_nc_macs) );
  memset( mv_nc_vlan_names, 0 , sizeof(mv_nc_vlan_names) );

  if(!inited) 
    return GT_FAIL;

  memcpy(unmConfig, &file_cnf, sizeof(MV_UNM_CONFIG));

  /* init all VLAN fields */
  /* We are using two set of structures here
   * 1. The: mv_nc_conf_file_macs, mv_nc_conf_file_names, mv_nc_unmConfig
   *    those three conf structs are being filled by an IOCTL call of a configuration application
   *
   * 2. mv_nc_macs, mv_nc_vlan_names, cnf, that are being updated here from the structures above
   *    here at the context of UNM initialize, the system gets ready to work.
   */
  
  for( i = 0 ; i < MV_UNM_MAX_VID ; i++) {
    if(strlen(mv_nc_file_names[i]) > 0 ) {
      memcpy(mv_nc_vlan_names[i], mv_nc_file_names[i], strlen(mv_nc_file_names[i]));
      if( i > 0 && i < MV_UNM_VID_ISOLATED ) { 
		memcpy(mv_nc_macs[i], mv_nc_file_macs[i],  GT_ETHERNET_HEADER_SIZE);
		if( memcmp(mv_nc_file_macs[i], null_mac, GT_ETHERNET_HEADER_SIZE) == 0  ) {
		  mvOsPrintf("Error - No MAC ADDR is set for VLAN WAN\n");
		  return GT_FAIL;  
		}
      }
    }
  }
#ifdef ETH_DBG_INFO
  printUnimacStructs(unmConfig, mv_nc_vlan_names, mv_nc_macs);  
#endif /* ETH_DBG_INFO */
  return (GT_OK);
}

