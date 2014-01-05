/*
 *  Driver routines for BUFFALO Platform
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


#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#include "kernevntProc.h"
/* Globals */
// same as Buffalo Kernel Ver.
#define BUFCORE_VERSION "0.16"

/* Module parameters */
MODULE_AUTHOR("BUFFALO");
MODULE_DESCRIPTION("Buffalo Platform Linux Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(BUFCORE_VERSION);

/* Definitions */
//#define DEBUG

#define USE_PROC_BUFFALO

// ----------------------------------------------------

/* Definitions for DEBUG */
#ifdef DEBUG
 #define FUNCTRACE(x)  x

#else
 #define FUNCTRACE(x) 

#endif

/* Function prototypes */

/* variables */
static struct proc_dir_entry *proc_buffalo;

#define DEBUG_PROC
#ifdef DEBUG_PROC
//----------------------------------------------------------------------
static int debug_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
#if 0
	volatile struct tagReg {
		unsigned MainIntCause;
		unsigned MainIrqIntMask;
		unsigned MainFiqIntMask;
	} *intreg;

	unsigned base=0xF1000000;
	volatile unsigned *gpio=(unsigned *)(base+0x10100);
	
	intreg = (struct tagReg *)(base + 0x20200);
	
	len += sprintf(buf+len,"cause=%lx irqmask=%lx fiqmask=%lx\n "
		,intreg->MainIntCause, intreg->MainIrqIntMask,intreg->MainFiqIntMask );
	
	len += sprintf(buf+len,"GPIO:OutReg=0x%04x\nDOutEn=0x%04x\nDBlink=0x%04x\nDatainActLow=0x%04x\nDIn=0x%04x\nInt=0x%04x\nMask=0x%03x\nLMask=0x%04x\n "
		,gpio[0],gpio[1],gpio[2],gpio[3],gpio[4],gpio[5],gpio[6],gpio[7] );
#endif

	unsigned base=0xF1000000;
	volatile unsigned *gpio=(unsigned *)(base+0x10100);

	len += sprintf(buf + len, "GPP_DATA_OUT_REG(0)=0x%08x\n", gpio[0]);
	len += sprintf(buf + len, "GPP_DATA_OUT_EN_REG(0)=0x%08x\n", gpio[1]);
	len += sprintf(buf + len, "GPP_BLINK_EN_REG(0)=0x%08x\n", gpio[2]);
	len += sprintf(buf + len, "GPP_DATA_IN_POL_REG(0)=0x%08x\n", gpio[3]);
	len += sprintf(buf + len, "GPP_DATA_IN_REG(0)=0x%08x\n", gpio[4]);
	len += sprintf(buf + len, "GPP_INT_CAUSE_REG(0)=0x%08x\n", gpio[5]);
	len += sprintf(buf + len, "GPP_INT_MASK_REG(0)=0x%08x\n", gpio[6]);
	len += sprintf(buf + len, "GPP_INT_LVL_REG(0)=0x%08x\n", gpio[7]);
	
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

#endif

/* Functions */
//----------------------------------------------------------------------
char * getdate(void)
{
	static char newstr[32];
	char sMonth[8];
	unsigned char i;
	int iDay,iYear,iMonth=0;
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                            "Sep", "Oct", "Nov", "Dec"};

	
	sscanf(__DATE__,"%s %d %d",sMonth,&iDay,&iYear);
	
	for (i = 0; i < 12; i++){
		if (!strcmp(sMonth, months[i])){
			iMonth = i + 1;
			break;
		}
	}
	sprintf(newstr,"%d/%02d/%02d",iYear,iMonth,iDay);
	return newstr;
}

extern char saved_command_line[];

//----------------------------------------------------------------------
static int kernelfw_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
	char *p,bootver[16];
	
	//printk("cmd:%s\n",saved_command_line);
	p=strstr(saved_command_line,"BOOTVER=");
	if (p){
		strncpy(bootver,p,sizeof(bootver));
		p=strstr(bootver," ");
		if (p){
			*p=0;
		}
	}
#ifdef CONFIG_BUFFALO_LINKSTATION_LSGL
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-GL(IESADA)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_HSWDHTGL_R1)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=HS-WDHTGL/R1(KOHREI)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-WTGL/R1(KOGEN)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWSGL_R1)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-WSGL/R1(SUJIN)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWHGL_R1)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-WHGL/R1(KEIKOU)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V3)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-WTGL/R1-V3(SEIMU)\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V2)
	len += sprintf(buf+len,"SERIES=LinkStation\n");
	len += sprintf(buf+len,"PRODUCTNAME=LS-WTGL/R1-V2(CHUUAI)\n");
#elif defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
	len += sprintf(buf+len,"SERIES=TeraStation\n");
 #if defined CONFIG_BUFFALO_TERASTATION_R1
	len += sprintf(buf+len,"PRODUCTNAME=TS-HTGL/R1(IEMOCHI)\n");
 #elif defined CONFIG_BUFFALO_TERASTATION_R5
	len += sprintf(buf+len,"PRODUCTNAME=TS-HTGL/R5(YOSHINOBU)\n");
 #else
   #error not defined
 #endif
#else
	#error not defined
#endif

	len += sprintf(buf+len,"VERSION=%s\n",BUFCORE_VERSION);
	len += sprintf(buf+len,"SUBVERSION=FLASH 0.00\n");
#ifdef CONFIG_BUFFALO_LINKSTATION_LSGL
	len += sprintf(buf+len,"PRODUCTID=0x00000009\n");
#elif defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
 #if defined CONFIG_BUFFALO_TERASTATION_R1
 	len += sprintf(buf+len,"PRODUCTID=0x00002004\n");
 #elif defined CONFIG_BUFFALO_TERASTATION_R5
 	len += sprintf(buf+len,"PRODUCTID=0x00002005\n");
 #else
   #error not defined
 #endif
#elif defined(CONFIG_BUFFALO_LINKSTATION_HSWDHTGL_R1)
	len += sprintf(buf+len,"PRODUCTID=0x00003000\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1)
	len += sprintf(buf+len,"PRODUCTID=0x00003001\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWSGL_R1)
	len += sprintf(buf+len,"PRODUCTID=0x00003002\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWHGL_R1)
	len += sprintf(buf+len,"PRODUCTID=0x00003003\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V3)
	len += sprintf(buf+len,"PRODUCTID=0x00003004\n");
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V2)
	len += sprintf(buf+len,"PRODUCTID=0x00003005\n");
#else
	#error not defined
#endif
	
	len += sprintf(buf+len,"BUILDDATE=%s %s\n",getdate(),__TIME__);
	len += sprintf(buf+len,"%s\n",bootver);
	
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}
//----------------------------------------------------------------------
static int enetinfo_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
	int egiga_buffalo_proc_string(char *buf); // arch/arm/mach-mv88fxx81/egiga/mv_e_main.c

	len = egiga_buffalo_proc_string(buf);
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

static int enetinfo_write_proc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	//printk("buffer=%s\n", buffer);
	if(strncmp(buffer, "auto", 4) == 0){
		//printk("setting to auto...\n");
		egiga_buffalo_change_configuration((unsigned short)0);
	}else if(strncmp(buffer, "master", 6) == 0){
		//printk("setting to master...\n");
		egiga_buffalo_change_configuration((unsigned short)1);
	}else if(strncmp(buffer, "slave", 5) == 0){
		//printk("setting to slave...\n");
		egiga_buffalo_change_configuration((unsigned short)2);
	}else{
		//printk("no effect\n");
	}

	return count;
}
//----------------------------------------------------------------------
static int micon_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
	
	len = sprintf(buf,"on\n");
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

//----------------------------------------------------------------------
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
static int lcd_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
	
	len = sprintf(buf,"on\n");
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}
#endif

//----------------------------------------------------------------------
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
static int board_info_read_proc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	char *buf = page;
	off_t begin = 0;
	char szBoardName[30];

	mvBoardNameGet(szBoardName);
	len += sprintf(buf + len, "BoardId=%02x\n", mvBoardIdGet());
	len += sprintf(buf + len, "BoardName=%s\n", szBoardName);
	len += sprintf(buf + len, "BoardStrap=%02x\n", BuffaloBoardGetStrapStatus());

	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}
#endif
//----------------------------------------------------------------------
int __init buffaloDriver_init (void)
{
	struct proc_dir_entry *generic;
	struct proc_dir_entry *enet;
	FUNCTRACE(printk(">%s\n",__FUNCTION__));
	
	proc_buffalo = proc_mkdir("buffalo", 0);
	if (!proc_buffalo) {
		printk (KERN_ERR "cannot init /proc/buffalo\n");
		return -ENOMEM;
	}
	generic = create_proc_info_entry ("buffalo/firmware", 0, 0, kernelfw_read_proc);
#ifdef DEBUG_PROC
	generic = create_proc_info_entry ("buffalo/debug", 0, 0, debug_read_proc);
#endif
	enet = create_proc_entry ("buffalo/enet", 0, 0);
	enet->read_proc = &enetinfo_read_proc;
	enet->write_proc= &enetinfo_write_proc;

#ifdef CONFIG_BUFFALO_USE_MICON
	generic = create_proc_info_entry ("buffalo/micon", 0, 0, micon_read_proc);
#endif
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
	generic = create_proc_info_entry ("buffalo/board_info", 0, 0, board_info_read_proc);
#endif
	BuffaloKernevnt_init();
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
	generic = create_proc_info_entry ("buffalo/lcd", 0, 0, lcd_read_proc);
#endif
	return 0;
}

//----------------------------------------------------------------------
void buffaloDriver_exit(void)
{
	FUNCTRACE(printk(">%s\n",__FUNCTION__));
	
	BuffaloKernevnt_exit();
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
	remove_proc_entry("buffalo/board_info", 0);
#endif
#if defined CONFIG_BUFFALO_USE_MICON
	remove_proc_entry("buffalo/micon", 0);
#endif
	remove_proc_entry("buffalo/enet", 0);
#ifdef DEBUG_PROC
	remove_proc_entry("buffalo/debug", 0);
#endif
	remove_proc_entry("buffalo/firmware", 0);
	remove_proc_entry ("buffalo", 0);
}

module_init(buffaloDriver_init);
module_exit(buffaloDriver_exit);
