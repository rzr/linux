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
* qdModule.c
*
* DESCRIPTION:
*		Defines the entry point for the QD module
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
#include "mvOs.h"

#include "qdModule.h"
#include "qdCtrl.h"
#include "msIoctl.h"
/*
 * #define IOCTL_DBG
 *
 */


/************************************************************************/
/* file io API                                                          */
/*                                                                      */
/* The io API is a proc-fs implementation of ioctl                      */
/************************************************************************/
static struct proc_dir_entry *evb_resource_dump;

/*
 * The lengh of the result buffer
 */
MV_U32 evb_resource_dump_result_len;

/*
 * A 1024 bytes, (in dwords because of alignment) of the result buffer.  
 */
MV_U32 dwevb_resource_dump_result[1024/sizeof(MV_U32)];

/*
 * The result buffer pointer
 */
static char *evb_resource_dump_result = (char*)dwevb_resource_dump_result ;

/********************************************************************
 * evb_resource_dump_write -
 *
 * When written to the /proc/resource_dump file this function is called
 *
 * Inputs: file / data are not used. Buffer and count are the pointer
 *         and length of the input string
 * Returns: Read from GT register
 * Outputs: count
 *********************************************************************/
static int evb_resource_dump_write (struct file *file, const char *buffer, unsigned long count, void *data) 
{
  MV_BOOL rc;
  unsigned int lenIn = 0 , code;

  evb_resource_dump_result_len = 0;
  memset(evb_resource_dump_result, 0, sizeof(dwevb_resource_dump_result));

  code = *((unsigned int*)buffer);
  if(!code)
    return count;
  lenIn = *((unsigned int*)(buffer + sizeof(code) ));
#ifdef IOCTL_DBG
  printk("Got IOCTL for code=%x, len=%d count = %d\n", code, lenIn, count);
#endif /* IOCTL_DBG */
#if 1 /*saeed*/
  rc = UNM_IOControl( 0, code, (char*)(buffer + sizeof(code) + sizeof(lenIn) ), lenIn, 
		      evb_resource_dump_result,    
		      sizeof(dwevb_resource_dump_result) , 
		      &evb_resource_dump_result_len 
		      );
#endif
  if(rc == MV_FALSE) {
    evb_resource_dump_result_len = 0xFFFFFFFF;
    printk("Error for IOCTL code %x\n", code);
  }
#ifdef IOCTL_DBG
  else {
    int i;
    printk("Success for IOCTL code %x outlen = %d\n", code, evb_resource_dump_result_len);
    for(i=0;i<evb_resource_dump_result_len; i++) {
      printk("%x ",evb_resource_dump_result[i]);
    }
    printk("\n"); 
  }
#endif /* IOCTL_DBG */
  return count;
}

/********************************************************************
 * evb_resource_dump_read -
 *
 * When read from the /proc/resource_dump file this function is called
 *
 * Inputs: buffer_location and buffer_length and zero are not used.
 *         buffer is the pointer where to post the result
 * Returns: N/A
 * Outputs: length of string posted
 *********************************************************************/
static int evb_resource_dump_read (char *buffer, char **buffer_location, off_t offset, 
				   int buffer_length, int *zero, void *ptr) 
{
  if (offset > 0)
    return 0;
  /* first four bytes are the len of the out buffer */
  memcpy(buffer, (char*)&evb_resource_dump_result_len, sizeof(evb_resource_dump_result_len) );

  /* in offset of 4 bytes the out buffer begins */
  memcpy((buffer + sizeof (evb_resource_dump_result_len )), 
	 evb_resource_dump_result, 
	 sizeof(dwevb_resource_dump_result)
	 );
  
  return ( evb_resource_dump_result_len + sizeof(evb_resource_dump_result_len));
}



/************************************************************************/
/* module API                                                           */
/************************************************************************/
/*
 * The qd start actually strarts the qd operation.
 * It calls the qdInit() to the the actual work (see qdInit.c).
 */
int qdModuleStart(void)
{
  GT_STATUS status;
  if( (status=qdInit()) != GT_OK) {
    DBG_INFO(("\n qdModuleInit: Cannot start the QD switch!\n"));
    return -1;
  }
	
  DBG_INFO(("\n qdModuleInit is done!\n\n"));	
   return 0;
}

/*
 * At the entry point we init only the IOCTL hook.
 * An init IOCTL call will triger the real initialization of
 * the QD, with the start function above
 */
int qdEntryPoint(void)
{

  /* start_regdump_memdump -
   *
   * Register the /proc/rgcfgio file at the /proc filesystem
   */
  evb_resource_dump = create_proc_entry (RG_IO_FILENAME , 0666 , &proc_root);
  if(!evb_resource_dump)
    panic("Can't allocate UNM FILE-IO device\n");
  evb_resource_dump->read_proc = evb_resource_dump_read;
  evb_resource_dump->write_proc = evb_resource_dump_write;
  evb_resource_dump->nlink = 1;


  DBG_INFO(("\n qdEntryPoint is done!\n\n"));	
  return 0;
}

/*
 * Exit point of the module.
 */
void qdExitPoint(void)
{
  printk("QD Switch driver exited!\n");
}
