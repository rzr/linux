/*
 *  LinkStation/TeraStation UPS port Driver
 *
 *  Copyright (C)  BUFFALO INC.
 *
 *  This software may be used and distributed according to the terms of
 *  the GNU General Public License (GPL), incorporated herein by reference.
 *  Drivers based on or derived from this code fall under the GPL and must
 *  retain the authorship, copyright and license notice.  This file is not
 *  a complete program and may only be used when the entire operating
 *  system is licensed under the GPL.
 *
 */
#include <linux/config.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_BUFFALO_USE_UPS

#include "BuffaloGpio.h"

#define bzero(p,sz) memset(p,0,sz)

#define USE_SERIAL_UPS
#define USE_UPS_BL_SIGNAL
#define USE_UPS_RECOVER

//#define DEBUG_UPS

/* Function prototypes */


/* variables */
static struct proc_dir_entry *proc_buffalo_ups;

#ifdef DEBUG_UPS
//----------------------------------------------------------------------
static int BuffaloMppConfig_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;

	unsigned base=0xF1000000;
	volatile unsigned int *mpp_reg=(unsigned *)(base+0x10000);
	
	len += sprintf(buf, "Dumped base = 0x10000\noffset 0\t%x\noffset 4\t%x\noffset 8\t%x\n",
		mpp_reg[0], mpp_reg[1], mpp_reg[2]);
	
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

#endif
//----------------------------------------------------------------------
#if defined USE_SERIAL_UPS
static int BuffaloUpsReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	
	int len = 0;
	
	unsigned int PinStat = BuffaloGpio_UPSPortScan();

	if(PinStat){
		len = sprintf(page, "on\n");
	}else{
		len = sprintf(page, "off\n");
	}
	
	*start = page + offset;
	len -= offset;
	
	if(len > length)
		len = length;
	
	return (len);
	
}

//--------------------------------------------------------------
static int BuffaloUpsWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_UPSPortEnable();
	}else if(strncmp(buffer, "off", 3) == 0 ){
		BuffaloGpio_UPSPortDisable();
	}
	
	return count;
}

#if defined DEBUG_UPS
//--------------------------------------------------------------
static int BuffaloGPIOPortReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	
	int len=0;
	
	unsigned int ret = BuffaloGpio_PortScan();
	len = sprintf(page, "ret = 0x%hx\n", ret);
	
	*start = page + offset;
	len -= offset;
	
	if(len>length)
		len = length;
	
	return len;
	
}
#endif // end of DEBUG_UPS
#endif // end of USE_SERIAL_UPS


#if defined USE_UPS_BL_SIGNAL
//--------------------------------------------------------------
static int BuffaloOMUpsUseBLReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	
	int len = 0;
	unsigned int PinStat = BuffaloGpio_UPSOmronBLUseStatus();
	
	if(PinStat){
		len = sprintf(page, "on\n");
	}else{
		len = sprintf(page, "off\n");
	}
	
	*start = page + offset;
	len -= offset;
	
	if(len > length)
		len = length;
	
	return (len);
}

//--------------------------------------------------------------
static int BuffaloOMUpsUseBLWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	
	if(strncmp(buffer, "on", 2) == 0){
		BuffaloGpio_UPSOmronBLEnable();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_UPSOmronBLDisable();
	}
	
	return count;
}

//--------------------------------------------------------------
static int BuffaloOMUpsBLReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{

	int len = 0;
	unsigned int PinStat = BuffaloGpio_UPSOmronBLGetStatus();
	printk("Pinstat=%x\n", PinStat);
	
	if(PinStat){
		len = sprintf(page, "on\n");
	}else{
		len = sprintf(page, "off\n");
	}
	
	*start = page + offset;
	len -= offset;
	
	if(len > length)
		len = length;
	
	return (len);
	
}
#endif // end of USE_UPS_BL_SIGNAL

#if defined USE_UPS_RECOVER
//--------------------------------------------------------------
static void BuffaloUpsRecoverInit(void){
	
	BuffaloRtc_UPSRecoverInit();
	printk("BUFFALO UPS Recover Function Initialized.\n");

}

//--------------------------------------------------------------
static int BuffaloUpsRecoverWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){
	int8_t OnTarget=0;	

	if(strncmp(buffer, "apc_on", 6) == 0){
		OnTarget = RECOVER_TARGET_UPS_APC;	
	}else if(strncmp(buffer, "omron_on", 8) == 0){
		OnTarget = RECOVER_TARGET_UPS_OMR;
//		printk("%s %s>on entered\n", __FILE__, __FUNCTION__);
	}else if(strncmp(buffer, "usb_on", 6) == 0){
		OnTarget = RECOVER_TARGET_UPS_USB;
	}else if(strncmp(buffer, "off", 3) ==0){
//		printk("%s %s>off entered\n", __FILE__, __FUNCTION__);
		BuffaloRtc_UPSRecoverDisable();
	}
	if(OnTarget){
		BuffaloRtc_UPSRecoverEnable(OnTarget);
	}
	return count;
}

//--------------------------------------------------------------
static int BuffaloUpsRecoverReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data){
	
	int len = 0;
	int ret = BuffaloRtc_UPSRecoverReadStatus();
	
	switch(ret){
	case RECOVER_TARGET_UPS_APC:
		len = sprintf(page, "apc_on\n");
		break;
	case RECOVER_TARGET_UPS_OMR:
		len = sprintf(page, "omron_on\n");
		break;
	case RECOVER_TARGET_UPS_USB:
		len = sprintf(page, "usb_on\n");
		break;
	default:
		len = sprintf(page, "off\n");
		break;
	}
	
	*start = page + offset;
	len -= offset;
	
	if(len > length)
		len=length;
	
	return (len);
}
#endif // end of USE_UPS_RECOVER


//--------------------------------------------------------------
/*
 * Initialize driver.
 */
int __init BuffaloUpsdrv_init (void)
{
#if defined USE_SERIAL_UPS
	struct proc_dir_entry *generic_ups;
#endif
#if defined USE_UPS_BL_SIGNAL
	struct proc_dir_entry *generic_BL;
	struct proc_dir_entry *generic_BL_stat;
#endif
#if defined USE_UPS_RECOVER
	struct proc_dir_entry *generic_ups_recover;
#endif

	printk("UPSDRV (C) BUFFALO INC. V.1.00 installed.\n"); 
	
	proc_buffalo_ups = proc_mkdir("buffalo/ups", 0);

#if defined USE_SERIAL_UPS
	generic_ups = create_proc_entry ("buffalo/ups/ups_serial", 0, 0);
	generic_ups->read_proc=&BuffaloUpsReadProc;
	generic_ups->write_proc=&BuffaloUpsWriteProc;
	
	#if defined DEBUG_UPS
		struct proc_dir_entry *generic_ups_debug;
		struct proc_dir_entry *generic_mpp_config;

		generic_ups_debug = create_proc_entry("buffalo/ups/ups_debug", 0, 0);
		generic_ups_debug->read_proc=&BuffaloGPIOPortReadProc;
		
		generic_mpp_config = create_proc_entry("buffalo/ups/mpp_config", 0, 0);
		generic_mpp_config->read_proc=&BuffaloMppConfig_read_proc;
	#endif
	
#endif //end of USE_SERIAL_UPS

#if defined USE_UPS_BL_SIGNAL
	generic_BL = create_proc_entry("buffalo/ups/omr_bl_use", 0, 0);
	generic_BL->read_proc=&BuffaloOMUpsUseBLReadProc;
	generic_BL->write_proc=&BuffaloOMUpsUseBLWriteProc;
	
	generic_BL_stat = create_proc_entry("buffalo/ups/omr_bl_status", 0, 0);
	generic_BL_stat->read_proc=&BuffaloOMUpsBLReadProc;
#endif

#if defined USE_UPS_RECOVER
	BuffaloUpsRecoverInit();
	generic_ups_recover = create_proc_entry("buffalo/ups/ups_recover", 0, 0);
	generic_ups_recover->read_proc=&BuffaloUpsRecoverReadProc;
	generic_ups_recover->write_proc=&BuffaloUpsRecoverWriteProc;
#endif
	
	return 0;
}

//--------------------------------------------------------------
void BuffaloUpsdrv_exit(void)
{
	printk("UPSDRV removed.");
	
#if defined USE_SERIAL_UPS
	remove_proc_entry("buffalo/ups/ups_serial", 0);
	#if defined DEBUG_UPS
		remove_proc_entry("buffalo/ups/ups_debug", 0);
		remove_proc_entry("buffalo/ups/mpp_config", 0);
	#endif
#endif

#if defined USE_UPS_BL_SIGNAL
	remove_proc_entry("buffalo/ups/omr_bl_use", 0);
	remove_proc_entry("buffalo/ups/omr_bl_status", 0);
#endif

#if defined USE_UPS_RECOVER
	remove_proc_entry("buffalo/ups/ups_recover",0);
#endif
	remove_proc_entry("buffalo/ups", 0);
}

module_init(BuffaloUpsdrv_init);
module_exit(BuffaloUpsdrv_exit);
MODULE_LICENSE("GPL");

#endif  //CONFIG_BUFFALO_USE_UPS

