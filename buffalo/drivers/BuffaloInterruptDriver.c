
#include <linux/config.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

#include "kernevnt.h"
#include "BuffaloGpio.h"

#define AUTHOR	"(C) BUFFALO INC."
#define DRIVER_NAME	"Buffalo CPU Inerupts Driver"
#define BUFFALO_DRIVER_VER	"0.01 alpha1"

MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_LICENSE("GPL");
MODULE_VERSION(BUFFALO_DRIVER_VER);

#define CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER

//#define CONFIG_BUFFALO_DEBUG_GPIO_INTERRUPT_DRIVER
#ifdef CONFIG_BUFFALO_DEBUG_GPIO_INTERRUPT_DRIVER
	#define TRACE(x)	x
#else
	#define TRACE(x)
#endif

#define POLLING_COUNTS_PER_1SEC	10

#if !defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
static int g_irq=0;
static int g_ShortNotified=0;
static int g_Notified=0;

#define PSW_IRQ	(32 + BIT_POWER)
#define PSW_POL_MSEC	(HZ / POLLING_COUNTS_PER_1SEC) 	// 50 mili second
#define PSW_TOTAL_WAIT	(1 * HZ)			// 3 seccond
#define PSW_WAIT_COUNT	(PSW_TOTAL_WAIT / PSW_POL_MSEC)	// TOTAL_WAIT / PSW_POL_MSEC

#define PSW_SHORT_WAIT_COUNT	(0 * PSW_POL_MSEC) // 0.3(6 * 0.05) second wait

static struct timer_list PSWPollingTimer;
static void PollingTimerGoOn(void);
static void PollingTimerStop(void);
#endif

// for init sw
#if defined(CONFIG_BUFFALO_USE_SWITCH_INIT)
 static int g_irq_init=0;
 int Initializing=0;

 #define INIT_IRQ (32 + BIT_INIT)

 #define INIT_POL_MSEC	(HZ / POLLING_COUNTS_PER_1SEC)
 #define INIT_TOTAL_WAIT	(5 * HZ)
 #define INIT_WAIT_COUNT	(INIT_TOTAL_WAIT / INIT_POL_MSEC)

 static struct timer_list INITPollingTimer;
 static void PollingTimerGoOnInit(void);
 static void PollingTimerStopInit(void);
#endif // CONFIG_BUFFALO_USE_SWITCH_INIT

// for Func sw
#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
 static int irq_func=0;
 static int func_pushed;
 #define FUNC_IRQ (32 + BIT_FUNC)

 #define FUNC_POL_MSEC		(HZ / POLLING_COUNTS_PER_1SEC)
 #define FUNC_TOTAL_WAIT	(1 * HZ)
 #define FUNC_WAIT_COUNT	(0)
 #define FUNC_LONG_WAIT_COUNT	(FUNC_TOTAL_WAIT / FUNC_POL_MSEC)
 #define FUNC_PUSHED		0x01
 #define FUNC_LONG_PUSHED	0x02

 static struct timer_list FuncPollingTimer;
 static void PollingTimerGoOnFunc(void);
 static void PollingTimerStopFunc(void);
#endif // CONFIG_BUFFALO_USE_SWITCH_FUNC

#if !defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
static int PollingPowerSWStatus(unsigned long data){
	
	TRACE(printk(">%s, data=%u, PSW_WAIT_COUNT=%d\n", __FUNCTION__, data, PSW_WAIT_COUNT));
	unsigned int PowerStat=buffalo_gpio_read();

	if(PowerStat & BIT(BIT_POWER)){
		if((data > PSW_WAIT_COUNT) && (g_Notified == 0)){
			#if 0
			// Polling timer must be stopped at power switch release timing.
			// and irq enabling timing is same to above.
			PollingTimerStop();
			if(g_irq)
				enable_irq(g_irq);
			#endif
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("PSW_pushed");
#else

#endif
			g_Notified = 1;
			// keep polling until button would be released.
			PollingTimerGoOn();
			TRACE(printk("shutdown start\n"));
		}else if((data > PSW_SHORT_WAIT_COUNT) && (g_ShortNotified == 0)){
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("PSW_short_pushed");
			g_ShortNotified = 1;
#endif
			PollingTimerGoOn();
		}else{	
			PollingTimerGoOn();
		}
	}else{
		PollingTimerStop();
		if(g_irq)
			enable_irq(g_irq);
	}

	return 0;
}

static void PollingTimerStop(void){
	del_timer(&PSWPollingTimer);
	g_ShortNotified = 0;
	g_Notified = 0;
}

static void PollingTimerGoOn(void){
	PSWPollingTimer.expires=(jiffies + PSW_POL_MSEC);
	PSWPollingTimer.data+=1;
	add_timer(&PSWPollingTimer);
}

static void PollingTimerStart(void){
	init_timer(&PSWPollingTimer);
	PSWPollingTimer.expires=(jiffies + PSW_POL_MSEC);
	PSWPollingTimer.function=&PollingPowerSWStatus;
	PSWPollingTimer.data=0;
	g_ShortNotified = 0;
	g_Notified = 0;
	add_timer(&PSWPollingTimer);
}

static int CpuIntActivate_read_proc(char *page, char **start, off_t offset, int length)
{
	const char *msg="PowerIntAct\n";
	
	if(offset>0)
		return 0;

	*start=page+offset;
	strcpy(page, msg);

	if(g_irq){
		disable_irq(g_irq);
		enable_irq(g_irq);
	}

	return (strlen(msg));
}


static int PowSwInterrupts(int irq, void *dev_id, struct pt_regs *reg)
{
	TRACE(printk(">%s\n", __FUNCTION__));
	//Process to checking power sw pushed or not.

	disable_irq(irq);
	g_irq=irq;

	// Setup PollingPowerSWStatus;
	PollingTimerStart();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}
#endif


#if defined(CONFIG_BUFFALO_USE_SWITCH_INIT)
static int
PollingINITSWStatus(unsigned long data){
	
	TRACE(printk(">%s, data=%u, INIT_WAIT_COUNT=%d\n", __FUNCTION__, data, INIT_WAIT_COUNT));
	unsigned int InitStat=buffalo_gpio_read();

	if(InitStat & BIT(BIT_INIT)){
		if((data > INIT_WAIT_COUNT) && (Initializing == 0)){
			#if 0
			//PollingTimer must be stopped at button released timing.
			PollingTimerStopInit();

			//keep irq disabling until PollingTimer stopping.
			if(g_irq_init)
				enable_irq(g_irq_init);
			#endif
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("INITSW_pushed");
#else

#endif
			Initializing = 1;
			// keep Polling untile button would be released.
			PollingTimerGoOnInit();
			TRACE(printk("initialize start\n"));
		}else{	
			PollingTimerGoOnInit();
		}
	}else{
		PollingTimerStopInit();
		if(g_irq_init)
			enable_irq(g_irq_init);
	}

	return 0;
}


static void
PollingTimerStopInit(void){
	del_timer(&INITPollingTimer);
	Initializing=0;
}


static void
PollingTimerGoOnInit(void){
	INITPollingTimer.expires=(jiffies + INIT_POL_MSEC);
	INITPollingTimer.data+=1;
	add_timer(&INITPollingTimer);
}


static void
PollingTimerStartInit(void){
	init_timer(&INITPollingTimer);
	INITPollingTimer.expires=(jiffies + INIT_POL_MSEC);
	INITPollingTimer.function=&PollingINITSWStatus;
	INITPollingTimer.data=0;
	Initializing=0;
	add_timer(&INITPollingTimer);
}


static int
InitSwInterrupts(int irq, void *dev_id, struct pt_regs *reg)
{
	TRACE(printk(">%s\n", __FUNCTION__));

	disable_irq(irq);
	g_irq_init=irq;

	PollingTimerStartInit();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}
#endif //CONFIG_BUFFALO_USE_SWITCH_INIT

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
static int
PollingFuncSWStatus(unsigned long data)
{
	TRACE(printk(">%s, data=%u, FUNC_WAIT_COUNT=%d\n", __FUNCTION__, data, FUNC_WAIT_COUNT));
	unsigned int FuncStat=buffalo_gpio_read();

	if (FuncStat & BIT(BIT_FUNC)){
		func_pushed |= FUNC_PUSHED;
		if ((data > FUNC_LONG_WAIT_COUNT) && !(func_pushed & FUNC_LONG_PUSHED)){
			func_pushed |= FUNC_LONG_PUSHED;
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("FUNCSW_long_pushed");
#endif
		}
		PollingTimerGoOnFunc();
	}else{
		PollingTimerStopFunc();
		if (irq_func)
			enable_irq(irq_func);
		
		if ((data > FUNC_WAIT_COUNT) && !(func_pushed ^ FUNC_PUSHED)) {
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("FUNCSW_pushed");
#endif
		}
		func_pushed=0;
	}

	return 0;
}

static void
PollingTimerStopFunc(void)
{
	del_timer(&FuncPollingTimer);
}

static void
PollingTimerGoOnFunc(void)
{
	FuncPollingTimer.expires=(jiffies + FUNC_POL_MSEC);
	FuncPollingTimer.data+=1;
	add_timer(&FuncPollingTimer);
}

static void
PollingTimerStartFunc(void)
{
	init_timer(&FuncPollingTimer);
	FuncPollingTimer.expires=(jiffies + FUNC_POL_MSEC);
	FuncPollingTimer.function=&PollingFuncSWStatus;
	FuncPollingTimer.data=0;
	add_timer(&FuncPollingTimer);
}

static int
FuncSwInterrupts(int irq, void *dev_id, struct pt_regs *reg)
{
	TRACE(printk(">%s\n", __FUNCTION__));

	disable_irq(irq);
	irq_func=irq;

	PollingTimerStartFunc();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}
#endif /* CONFIG_BUFFALO_USE_SWITCH_FUNC */

/*
 * Initialize driver.
 */
int __init BuffaloCpuInterrupts_init(void)
{
	TRACE(printk(">%s\n", __FUNCTION__));

#if !defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	struct proc_dir_entry *buffalo_cpu_int_proc;

	if((buffalo_cpu_int_proc = create_proc_info_entry("buffalo/PowerSWInt_en", 0, 0, CpuIntActivate_read_proc))==0){
		proc_mkdir("buffalo", 0);
		buffalo_cpu_int_proc = create_proc_info_entry("buffalo/PowerSWInt_en", 0, 0, CpuIntActivate_read_proc);
	}

	BuffaloGpio_CPUInterruptsSetup();
	request_irq(PSW_IRQ, PowSwInterrupts, 0, "PowSw", NULL);
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_INIT)
	BuffaloGpio_CPUInterruptsSetupInit();
	request_irq(INIT_IRQ, InitSwInterrupts, 0, "InitSw", NULL);
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	BuffaloGpio_CPUInterruptsSetupFunc();
	request_irq(FUNC_IRQ, FuncSwInterrupts, 0, "FuncSw", NULL);
#endif

	printk("%s %s Ver.%s installed.\n", DRIVER_NAME, AUTHOR, BUFFALO_DRIVER_VER);
	return 0;
}

void BuffaloCpuInterrupts_exit(void)
{
	TRACE(printk(">%s\n", __FUNCTION__));
#if !defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	free_irq(PSW_IRQ, NULL);
	PollingTimerStop();
#endif
	remove_proc_entry("buffalo/PowerSWInt_en", 0);
	printk("%s %s uninstalled.\n", DRIVER_NAME, BUFFALO_DRIVER_VER);
}

module_init(BuffaloCpuInterrupts_init);
module_exit(BuffaloCpuInterrupts_exit);

