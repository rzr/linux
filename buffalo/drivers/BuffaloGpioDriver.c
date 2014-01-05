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
#include "BuffaloGpio.h"
#include "kernevnt.h"

/* Globals */
#define BUFFALO_DRIVER_VER "0.02"
#define DRIVER_NAME	"Buffalo Gpio Control Driver"
#define AUTHOR	"(C) BUFFALO INC."

/* Module parameters */
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_LICENSE("GPL");
MODULE_VERSION(BUFFALO_DRIVER_VER);

/* Definitions */
//#define DEBUG

#define USE_PROC_BUFFALO

#define BUFFALO_LED_CONTROL_INFO_BLOCK_ENABLE
	
static struct timer_list AlarmLedTimer, InfoLedTimer, PowerLedTimer, FuncLedTimer;
struct led_t{
	int ErrorNumber;
	int Brighted;
	int Brighting;
};
	
static struct
buffalo_led_status_t{
	struct led_t alarm;
	struct led_t info;
	struct led_t power;
	struct led_t func;
} buffalo_led_status = {0};

#define MAX_ALARM_NUM	100
#define MAX_INFO_NUM	100
#define MAX_POWER_NUM	9
#define MAX_FUNC_NUM	9

#define LED_ON	1
#define LED_OFF	0

#define LED_PHASE_10	1
#define LED_PHASE_1		2
#define LED_PHASE_SHORT	3
#define LED_PHASE_LONG	4

#define BUFFALO_LED_WAIT_10		(HZ * 10 / 10)	// 1 sec
#define BUFFALO_LED_WAIT_1		(HZ * 5 / 10)	// 0.5 sec
#define BUFFALO_LED_WAIT_SHORT		(HZ * 3 / 10)	// 0.3 sec
#define BUFFALO_LED_WAIT_LONG		(HZ * 10 / 10)	// 1 sec

#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)

#define SW_POWER_NS	-1
#define SW_POWER_OFF	0
#define SW_POWER_ON	1
#define SW_POWER_AUTO	2

#define SW_POLLING_INTERVAL	1000	// 1sec

#define SW_PIN_STAT_OFF		0
#define SW_PIN_STAT_ON		(BIT(BIT_POWER))
#define SW_PIN_STAT_AUTO	(BIT(BIT_AUTO_POWER))

static struct timer_list BuffaloSwPollingTimer;

static unsigned int g_buffalo_sw_status = SW_POWER_NS;
static unsigned int g_buffalo_sw_polling_control = 0;

#endif

// ----------------------------------------------------

/* Definitions for DEBUG */
#ifdef DEBUG
 #define FUNCTRACE(x)  x
#else
 #define FUNCTRACE(x) 

#endif

#if defined LED_DEBUG
  #define LED_TRACE(x)	x
#else
  #define LED_TRACE(x)
#endif

#if defined PST_DEBUG
  #define PST_TRACE(x)	x
#else
  #define PST_TRACE(x)
#endif


static int
gpio_power_read_proc(char *page, char **start, off_t offset, int length)
{
	int len=0;
	off_t begin=0;
	unsigned int PowerStat=buffalo_gpio_read();

#if defined CONFIG_BUFFALO_USE_GPIO_DRIVER_DEBUG
	len += sprintf(page + len, "GpioStat=0x%04x\n", PowerStat);
	len += sprintf(page + len, "BIT(%d)=0x%04x\n", BIT_POWER, BIT(BIT_POWER));
#endif
	len += sprintf(page + len, "PowerStat=%d\n", (PowerStat & BIT(BIT_POWER))? 1:0);
	len += sprintf(page + len, "LedInfo=%d\n", (PowerStat & BIT(BIT_LED_INFO))? 0:1);
	len += sprintf(page + len, "LedAlarm=%d\n", (PowerStat & BIT(BIT_LED_ALARM))? 0:1);
	len += sprintf(page + len, "LedFunc=%d\n", (PowerStat & BIT(BIT_LED_FUNC))? 0:1);
	len += sprintf(page + len, "HddPower=%d\n", (PowerStat & BIT(BIT_HDD_PWR))? 0:1);
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


static int
BuffaloKatoi(const char *buffer){
	#define MAX_LENGTH	16
	char *szBuffer = NULL;
	int TmpNum=0;
	
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	
	if(sizeof(buffer) > MAX_LENGTH){
		LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
		return -1;
	}
	
	szBuffer=buffer;
	LED_TRACE(printk("%s > szBuffer=0x%08x\n", __FUNCTION__, szBuffer));
	LED_TRACE(printk("%s > *szBuffer=0x%08x\n", __FUNCTION__, *szBuffer));
	for(*szBuffer; *szBuffer!=NULL; *szBuffer++){
		LED_TRACE(printk("%s > *szBuffer=0x%08x, TmpNum=%d\n", __FUNCTION__, *szBuffer, TmpNum));
		if(*szBuffer>='0' && *szBuffer<='9')
			TmpNum=(TmpNum * 10) + (*szBuffer - '0');
	}
	
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
	return TmpNum;
}


static int
BuffaloLedGetNextPhase(struct led_t *led_stat){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));

	switch(led_stat->Brighting){
	case LED_ON:
		if((led_stat->ErrorNumber - led_stat->Brighted) > 0)
			return LED_PHASE_SHORT;
		else
			return LED_PHASE_LONG;
		break;
	case LED_OFF:
		LED_TRACE(printk("%s > led_stat->ErrorNumer=%d, led_stat->Brighted=%d\n", __FUNCTION__, led_stat->ErrorNumber, led_stat->Brighted));
		if((led_stat->ErrorNumber - led_stat->Brighted) >= 10)
			return LED_PHASE_10;
		else
			return LED_PHASE_1;
		break;
	default:
		return -1;
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
	return -1;
}


#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
 #if !defined(BIT_LED_ALARM)
  #error CONFIG_BUFFALO_USE_LED_ALARM is defined, but BIT_LED_ALARM is not defined.
 #endif


static void
BuffaloLed_AlarmStatChange(void){
	
	int NextPhase = BuffaloLedGetNextPhase(&(buffalo_led_status.alarm));
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	
	switch(NextPhase){
	case LED_PHASE_10:
		AlarmLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_10);
		BuffaloGpio_AlarmLedEnable();
		buffalo_led_status.alarm.Brighted += 10;
		buffalo_led_status.alarm.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_10\n", __FUNCTION__));
		break;
	case LED_PHASE_1:
		AlarmLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_1);
		BuffaloGpio_AlarmLedEnable();
		buffalo_led_status.alarm.Brighted += 1;
		buffalo_led_status.alarm.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_1\n", __FUNCTION__));
		break;
	case LED_PHASE_SHORT:
		AlarmLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
		BuffaloGpio_AlarmLedDisable();
		buffalo_led_status.alarm.Brighting = LED_OFF;
		LED_TRACE(printk("%s > change to LED_PHASE_SHORT\n", __FUNCTION__));
		break;
	case LED_PHASE_LONG:
		AlarmLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_LONG);
		BuffaloGpio_AlarmLedDisable();
		buffalo_led_status.alarm.Brighting = LED_OFF;
		buffalo_led_status.alarm.Brighted = 0;
		LED_TRACE(printk("%s > change to LED_PHASE_LONG\n", __FUNCTION__));
		break;
//	default:
//		return -1;
	}
	add_timer(&AlarmLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
//	return 0;
}


static void
BuffaloLed_AlarmBlinkStart(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	buffalo_led_status.alarm.Brighted = 0;
	buffalo_led_status.alarm.Brighting = LED_OFF;

	init_timer(&AlarmLedTimer);
	AlarmLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
	AlarmLedTimer.function = BuffaloLed_AlarmStatChange;
	add_timer(&AlarmLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
}


static void
BuffaloLed_AlarmBlinkStop(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	if(buffalo_led_status.alarm.ErrorNumber) del_timer(&AlarmLedTimer);
	buffalo_led_status.alarm.ErrorNumber = 0;
	buffalo_led_status.alarm.Brighted = 0;
	buffalo_led_status.alarm.Brighting = LED_OFF;
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

}
#endif //end of CONFIG_BUFFALO_USE_LED_ALARM



#if defined(CONFIG_BUFFALO_USE_LED_INFO)
 #if !defined(BIT_LED_INFO)
  #error CONFIG_BUFFALO_USE_LED_INFO is defined, but BIT_LED_INFO is not defined.
 #endif


static void
BuffaloLed_InfoStatChange(void){
	
	int NextPhase = BuffaloLedGetNextPhase(&(buffalo_led_status.info));
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	
	switch(NextPhase){
	case LED_PHASE_10:
		InfoLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_10);
#if defined BUFFALO_LED_CONTROL_INFO_BLOCK_ENABLE
		if(buffalo_led_status.alarm.ErrorNumber == 0) BuffaloGpio_InfoLedEnable();
		else BuffaloGpio_InfoLedDisable();
#else
		BuffaloGpio_InfoLedEnable();
#endif
		buffalo_led_status.info.Brighted += 10;
		buffalo_led_status.info.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_10\n", __FUNCTION__));
		break;
	case LED_PHASE_1:
		InfoLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_1);
#if defined BUFFALO_LED_CONTROL_INFO_BLOCK_ENABLE
		if(buffalo_led_status.alarm.ErrorNumber == 0) BuffaloGpio_InfoLedEnable();
		else BuffaloGpio_InfoLedDisable();
#else
		BuffaloGpio_InfoLedEnable();
#endif
		buffalo_led_status.info.Brighted += 1;
		buffalo_led_status.info.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_1\n", __FUNCTION__));
		break;
	case LED_PHASE_SHORT:
		InfoLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
		BuffaloGpio_InfoLedDisable();
		buffalo_led_status.info.Brighting = LED_OFF;
		LED_TRACE(printk("%s > change to LED_PHASE_SHORT\n", __FUNCTION__));
		break;
	case LED_PHASE_LONG:
		InfoLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_LONG);
		BuffaloGpio_InfoLedDisable();
		buffalo_led_status.info.Brighting = LED_OFF;
		buffalo_led_status.info.Brighted = 0;
		LED_TRACE(printk("%s > change to LED_PHASE_LONG\n", __FUNCTION__));
		break;
//	default:
//		return -1;
	}
	add_timer(&InfoLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
//	return 0;
}


static void
BuffaloLed_InfoBlinkStart(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	buffalo_led_status.info.Brighted = 0;
	buffalo_led_status.info.Brighting = LED_OFF;

	init_timer(&InfoLedTimer);
	InfoLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
	InfoLedTimer.function = BuffaloLed_InfoStatChange;
	add_timer(&InfoLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
}


static void
BuffaloLed_InfoBlinkStop(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	if(buffalo_led_status.info.ErrorNumber) del_timer(&InfoLedTimer);
	buffalo_led_status.info.ErrorNumber = 0;
	buffalo_led_status.info.Brighted = 0;
	buffalo_led_status.info.Brighting = LED_OFF;
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
}
#endif //CONFIG_BUFFALO_USE_LED_INFO


#if defined(CONFIG_BUFFALO_USE_LED_POWER)
 #if !defined(BIT_LED_PWR)
  #error CONFIG_BUFFALO_USE_LED_POWER is defined, but BIT_LED_PWR is not defined.
 #endif

static void
BuffaloLed_PowerStatChange(void){
	
	int NextPhase = BuffaloLedGetNextPhase(&(buffalo_led_status.power));
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	
	switch(NextPhase){
	case LED_PHASE_10:
		PowerLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_10);
		BuffaloGpio_PowerLedEnable();
		buffalo_led_status.power.Brighted += 10;
		buffalo_led_status.power.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_10\n", __FUNCTION__));
		break;
	case LED_PHASE_1:
		PowerLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_1);
		BuffaloGpio_PowerLedEnable();
		buffalo_led_status.power.Brighted += 1;
		buffalo_led_status.power.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_1\n", __FUNCTION__));
		break;
	case LED_PHASE_SHORT:
		PowerLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
		BuffaloGpio_PowerLedDisable();
		buffalo_led_status.power.Brighting = LED_OFF;
		LED_TRACE(printk("%s > change to LED_PHASE_SHORT\n", __FUNCTION__));
		break;
	case LED_PHASE_LONG:
		PowerLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_LONG);
		BuffaloGpio_PowerLedDisable();
		buffalo_led_status.power.Brighting = LED_OFF;
		buffalo_led_status.power.Brighted = 0;
		LED_TRACE(printk("%s > change to LED_PHASE_LONG\n", __FUNCTION__));
		break;
//	default:
//		return -1;
	}
	add_timer(&PowerLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
//	return 0;
}


static void
BuffaloLed_PowerBlinkStart(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	buffalo_led_status.power.Brighted = 0;
	buffalo_led_status.power.Brighting = LED_OFF;

	init_timer(&PowerLedTimer);
	PowerLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
	PowerLedTimer.function = BuffaloLed_PowerStatChange;
	add_timer(&PowerLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
}


static void
BuffaloLed_PowerBlinkStop(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	if(buffalo_led_status.power.ErrorNumber) del_timer(&PowerLedTimer);
	buffalo_led_status.power.ErrorNumber = 0;
	buffalo_led_status.power.Brighted = 0;
	buffalo_led_status.power.Brighting = LED_OFF;
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

}
#endif //CONFIG_BUFFALO_USE_LED_POWER

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
static void
BuffaloLed_FuncStatChange(void){
	
	int NextPhase = BuffaloLedGetNextPhase(&(buffalo_led_status.func));
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	
	switch(NextPhase){
	case LED_PHASE_10:
		FuncLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_10);
		BuffaloGpio_FuncLedEnable();
		buffalo_led_status.func.Brighted += 10;
		buffalo_led_status.func.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_10\n", __FUNCTION__));
		break;
	case LED_PHASE_1:
		FuncLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_1);
		BuffaloGpio_FuncLedEnable();
		buffalo_led_status.func.Brighted += 1;
		buffalo_led_status.func.Brighting = LED_ON;
		LED_TRACE(printk("%s > change to LED_PHASE_1\n", __FUNCTION__));
		break;
	case LED_PHASE_SHORT:
		FuncLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
		BuffaloGpio_FuncLedDisable();
		buffalo_led_status.func.Brighting = LED_OFF;
		LED_TRACE(printk("%s > change to LED_PHASE_SHORT\n", __FUNCTION__));
		break;
	case LED_PHASE_LONG:
		FuncLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_LONG);
		BuffaloGpio_FuncLedDisable();
		buffalo_led_status.func.Brighting = LED_OFF;
		buffalo_led_status.func.Brighted = 0;
		LED_TRACE(printk("%s > change to LED_PHASE_LONG\n", __FUNCTION__));
		break;
//	default:
//		return -1;
	}
	add_timer(&FuncLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
//	return 0;
}

static void
BuffaloLed_FuncBlinkStart(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	buffalo_led_status.func.Brighted = 0;
	buffalo_led_status.func.Brighting = LED_OFF;

	init_timer(&FuncLedTimer);
	FuncLedTimer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
	FuncLedTimer.function = BuffaloLed_FuncStatChange;
	add_timer(&FuncLedTimer);
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
}

static void
BuffaloLed_FuncBlinkStop(void){
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	if(buffalo_led_status.func.ErrorNumber) del_timer(&FuncLedTimer);
	buffalo_led_status.func.ErrorNumber = 0;
	buffalo_led_status.func.Brighted = 0;
	buffalo_led_status.func.Brighting = LED_OFF;
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

}
#endif // CONFIG_BUFFALO_USE_LED_FUNC

#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
 #if !defined(BIT_LED_ALARM)
  #error CONFIG_BUFFALO_USE_LED_ALARM is defined, but BIT_LED_ALARM is not defined.
 #endif


static int
BuffaloAlarmLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	if(buffalo_led_status.alarm.ErrorNumber == 0){
		unsigned int PinStat = buffalo_gpio_read();
		if(~PinStat & BIT(BIT_LED_ALARM))
			len = sprintf(page, "on\n");
		else
			len = sprintf(page, "off\n");
	}else{
		len = sprintf(page, "%d\n", buffalo_led_status.alarm.ErrorNumber);
	}
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


static int
BuffaloAlarmLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	int TmpNum=0;
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	TmpNum = BuffaloKatoi(buffer);
	LED_TRACE(printk("%s > TmpNum=%d\n", __FUNCTION__, TmpNum));
	
	// if the status buffalo_led_status.alarm.ErrorNumber is 0, force on is enable.
	if(TmpNum == 0){
		if(strncmp(buffer, "on", 2) == 0 ){
			LED_TRACE(printk("%s > Calling BuffaloGpio_AlarmLedEnable\n", __FUNCTION__));
			BuffaloGpio_AlarmLedEnable();
		}else if(strncmp(buffer, "off", 3) == 0){
			LED_TRACE(printk("%s > Calling BuffaloGpio_AlarmLedDisable\n", __FUNCTION__));
			BuffaloLed_AlarmBlinkStop();
			BuffaloGpio_AlarmLedDisable();
		}
	}else{
		if(buffalo_led_status.alarm.ErrorNumber ==0){
			if((TmpNum > 0) && (TmpNum <= MAX_ALARM_NUM)){
				buffalo_led_status.alarm.ErrorNumber = TmpNum;
				BuffaloLed_AlarmBlinkStart();
			}else if(TmpNum == 0){
				BuffaloLed_AlarmBlinkStop();
			}else{
				// Nothing to do...
			}
		}
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
	return count;
}


static int
BuffaloAlarmLedBlinkReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	unsigned int PinStat = buffalo_gpio_blink_reg_read();
	if(PinStat & BIT(BIT_LED_ALARM))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


static int
BuffaloAlarmLedBlinkWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_AlarmLedBlinkEnable();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_AlarmLedBlinkDisable();
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}
#endif // CONFIG_BUFFALO_USE_LED_ALARM


#if defined(CONFIG_BUFFALO_USE_LED_INFO)
 #if !defined(BIT_LED_INFO)
  #erorr CONFIG_BUFFALO_USE_LED_INFO is defined, but BIT_LED_INFO is not defined.
 #endif


static int
BuffaloInfoLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	
	if(buffalo_led_status.info.ErrorNumber == 0){
		unsigned int PinStat = buffalo_gpio_read();
		if(~PinStat & BIT(BIT_LED_INFO))
			len = sprintf(page, "on\n");
		else
			len = sprintf(page, "off\n");
	}else{
#if defined BUFFALO_LED_CONTROL_INFO_BLOCK_ENABLE
		if(buffalo_led_status.alarm.ErrorNumber != 0)
			len = sprintf(page, "%d(Blocked)\n", buffalo_led_status.info.ErrorNumber);
		else
			len = sprintf(page, "%d\n", buffalo_led_status.info.ErrorNumber);
#else
		len = sprintf(page, "%d\n", buffalo_led_status.info.ErrorNumber);
#endif
	}
	
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}


static int
BuffaloInfoLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	int TmpNum = 0;
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	TmpNum = BuffaloKatoi(buffer);
	LED_TRACE(printk("%s > TmpNum=%d\n", __FUNCTION__, TmpNum));
	
	// if the status buffalo_led_status.info.ErrorNumber is 0, force on is enable.
	if(TmpNum == 0){
		if(strncmp(buffer, "on", 2) == 0 ){
			BuffaloGpio_InfoLedEnable();
		}else if(strncmp(buffer, "off", 3) == 0){
			BuffaloLed_InfoBlinkStop();
			BuffaloGpio_InfoLedDisable();
		}
	}else{
		if(buffalo_led_status.info.ErrorNumber == 0){
			if((TmpNum > 0) && (TmpNum <= MAX_INFO_NUM)){
				buffalo_led_status.info.ErrorNumber = TmpNum;
				BuffaloLed_InfoBlinkStart();
			}else if(TmpNum == 0){
				BuffaloLed_InfoBlinkStop();
			}else{
				// Nothing to do...
			}
		}
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
		
	return count;
}


static int
BuffaloInfoLedBlinkReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	unsigned int PinStat = buffalo_gpio_blink_reg_read();
	if(PinStat & BIT(BIT_LED_INFO))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


static int
BuffaloInfoLedBlinkWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_InfoLedBlinkEnable();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_InfoLedBlinkDisable();
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}
#endif // CONFIG_BUFFALO_USE_LED_INFO


#if defined(CONFIG_BUFFALO_USE_LED_POWER)
 #if !defined(BIT_LED_PWR)
  #error CONFIG_BUFFALO_USE_LED_POWER is defined, but BIT_LED_PWR is not defined.
 #endif


static int
BuffaloPowerLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	if(buffalo_led_status.power.ErrorNumber == 0){
		unsigned int PinStat = buffalo_gpio_read();
		if(~PinStat & BIT(BIT_LED_PWR))
			len = sprintf(page, "on\n");
		else
			len = sprintf(page, "off\n");
	}else{
		len = sprintf(page, "%d\n", buffalo_led_status.power.ErrorNumber);
	}
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}


static int
BuffaloPowerLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	int TmpNum=0;
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	TmpNum = BuffaloKatoi(buffer);
	LED_TRACE(printk("%s > TmpNum=%d\n", __FUNCTION__, TmpNum));
	
	// if the status buffalo_led_status.power.ErrorNumber is 0, force on is enable.
	if(TmpNum == 0){

		if(strncmp(buffer, "on", 2) == 0 ){
			BuffaloGpio_PowerLedEnable();
		}else if(strncmp(buffer, "off", 3) == 0){
			BuffaloLed_PowerBlinkStop();
			BuffaloGpio_PowerLedDisable();
		}
	}else{
		if((TmpNum > 0) && (TmpNum <= MAX_POWER_NUM)){
			buffalo_led_status.power.ErrorNumber = TmpNum;
			BuffaloLed_PowerBlinkStart();
		}else if(TmpNum == 0){
			BuffaloLed_PowerBlinkStop();
		}else{
			// Nothing to do...
		}
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}


static int
BuffaloPowerLedBlinkReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	unsigned int PinStat = buffalo_gpio_blink_reg_read();
	if(PinStat & BIT(BIT_LED_PWR))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


static int
BuffaloPowerLedBlinkWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_PowerLedBlinkEnable();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_PowerLedBlinkDisable();
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}
#endif // CONFIG_BUFFALO_USE_LED_POWER

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
static int
BuffaloFuncLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	if(buffalo_led_status.func.ErrorNumber == 0){
		unsigned int PinStat = buffalo_gpio_read();
		if(~PinStat & BIT(BIT_LED_FUNC))
			len = sprintf(page, "on\n");
		else
			len = sprintf(page, "off\n");
	}else{
		len = sprintf(page, "%d\n", buffalo_led_status.func.ErrorNumber);
	}
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}

static int
BuffaloFuncLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	int TmpNum=0;
	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));
	TmpNum = BuffaloKatoi(buffer);
	LED_TRACE(printk("%s > TmpNum=%d\n", __FUNCTION__, TmpNum));
	
	// if the status buffalo_led_status.func.ErrorNumber is 0, force on is enable.
	if(TmpNum == 0){
		if(strncmp(buffer, "on", 2) == 0 ){
			BuffaloGpio_FuncLedEnable();
		}else if(strncmp(buffer, "off", 3) == 0){
			BuffaloLed_FuncBlinkStop();
			BuffaloGpio_FuncLedDisable();
		}
	}else{
		if((TmpNum > 0) && (TmpNum <= MAX_FUNC_NUM)){
			buffalo_led_status.func.ErrorNumber = TmpNum;
			BuffaloLed_FuncBlinkStart();
		}else if(TmpNum == 0){
			BuffaloLed_FuncBlinkStop();
		}else{
			// Nothing to do...
		}
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}

static int
BuffaloFuncLedBlinkReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	unsigned int PinStat = buffalo_gpio_blink_reg_read();
	if(PinStat & BIT(BIT_LED_FUNC))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}

static int
BuffaloFuncLedBlinkWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_FuncLedBlinkEnable();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_FuncLedBlinkDisable();
	}
	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));

	return count;
}
#endif // CONFIG_BUFFALO_USE_LED_FUNC

static int
BuffaloAllLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	if(~PinStat & BIT(BIT_LED_PWR))
		len += sprintf(page+len, "power led :on\n");
	else
		len += sprintf(page+len, "power led :off\n");
#endif

#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	if(~PinStat & BIT(BIT_LED_INFO))
		len += sprintf(page+len, "info  led :on\n");
	else
		len += sprintf(page+len, "info  led :off\n");
#endif

#if defined(CONFIG_BUFFALO_USE_LED_ALARM)	
	if(~PinStat & BIT(BIT_LED_ALARM))
		len += sprintf(page+len, "alarm led :on\n");
	else
		len += sprintf(page+len, "alarm led :off\n");
#endif

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	if(PinStat & BIT(BIT_LED_FUNC))
		len += sprintf(page+len, "func led :on\n");
	else
		len += sprintf(page+len, "func led :off\n");
#endif // CONFIG_BUFFALO_USE_LED_FUNC	

	if(len == 0)
		len += sprintf(page + len, "available led not found.\n");

	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}


static int
BuffaloAllLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_AllLedOn();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_AllLedOff();
	}
	return count;
}


#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
 #if !defined(BIT_HDD_PWR)
  #error CONFIG_BUFFALO_USE_HDD_POWER_CONTROL is defined, but BIT_HDD_PWR is not defined.
 #endif


static int
BuffaloHddReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_HDD_PWR))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}


static int
BuffaloHddWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_HddPowerOn();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_HddPowerOff();
	}
	return count;
}
#endif //CONFIG_BUFFALO_USE_HDD_POWER_CONTROL


#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
 #if !defined(BIT_USB_PWR)
  #error CONFIG_BUFFALO_USE_USB_POWER_CONTROL is defined, but BIT_USB_PWR is not defined.
 #endif

static int
BuffaloUsbReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_USB_PWR))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}


static int
BuffaloUsbWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_UsbPowerOn();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_UsbPowerOff();
	}
	return count;
}

#if defined(BIT_USB_PWR2)
static int
BuffaloUsbReadProc2(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_USB_PWR2))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}

static int
BuffaloUsbWriteProc2(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		BuffaloGpio_UsbPowerOn2();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_UsbPowerOff2();
	}
	return count;
}
#endif // BIT_USB_PWR2

#endif //CONFIG_BUFFALO_USE_USB_POWER_CONTROL


#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
 #if !defined(BIT_POWER)
  #error CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER is defined, but BIT_POWER is not defined.
 #endif

static int
BuffaloPowerReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_POWER))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}
#endif //CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER

static int
BuffaloSlideSwitchReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	len += sprintf(page, "1\n");
#else
	len += sprintf(page, "0\n");
#endif

	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}


#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
 #if !defined(BIT_AUTO_POWER)
  #error CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER is defined, but BIT_AUTO_POWER is not defined.
 #endif

static int
BuffaloAutoPowerReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_AUTO_POWER))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}

static int
BuffaloSwPollingCheck(void)
{
	unsigned int PinStat = buffalo_gpio_read();
	unsigned int present_sw_status = SW_POWER_NS;
	static char msg[32];

	FUNCTRACE(printk("PinStat = 0x%08x\n", PinStat));
	FUNCTRACE(printk("PinStat & (BIT(BIT_POWER) | BIT(BIT_AUTO_POWER)) = 0x%08x\n", (PinStat & (BIT(BIT_POWER) | BIT(BIT_AUTO_POWER)))));
	FUNCTRACE(printk("SW_PIN_STAT_OFF=0x%08x\n", SW_PIN_STAT_OFF));
	FUNCTRACE(printk("SW_PIN_STAT_ON=0x%08x\n", SW_PIN_STAT_ON));
	FUNCTRACE(printk("SW_PIN_STAT_AUTO=0x%08x\n", SW_PIN_STAT_AUTO));


	if(!(PinStat & BIT(BIT_POWER)) && !(PinStat & BIT(BIT_AUTO_POWER)))
		present_sw_status = SW_POWER_OFF;
	else if( (PinStat & BIT(BIT_POWER)) && !(PinStat & BIT(BIT_AUTO_POWER)))
		present_sw_status = SW_POWER_ON;
	else if(!(PinStat & BIT(BIT_POWER)) &&  (PinStat & BIT(BIT_AUTO_POWER)))
		present_sw_status = SW_POWER_AUTO;
	else
		present_sw_status = SW_POWER_NS;

	FUNCTRACE(printk("present_sw_status = 0x%08x\n", present_sw_status));
	FUNCTRACE(printk("g_buffalo_sw_status = 0x%08x\n", g_buffalo_sw_status));
	
	if(g_buffalo_sw_status != present_sw_status)
	{
		g_buffalo_sw_status = present_sw_status;
		memset(msg, 0, sizeof(msg));
		switch(g_buffalo_sw_status)
		{
			case SW_POWER_OFF:
#if 0
				FUNCTRACE(printk("%s> SW_POWER_OFF\n", __FUNCTION__));
#else
				printk("%s> SW_POWER_OFF\n", __FUNCTION__);
#endif
				buffalo_kernevnt_queuein("PSW_off");;
				break;
			case SW_POWER_ON:
#if 0
				FUNCTRACE(printk("%s> SW_POWER_ON\n", __FUNCTION__));
#else
				printk("%s> SW_POWER_ON\n", __FUNCTION__);
#endif
				buffalo_kernevnt_queuein("PSW_on");;
				break;
			case SW_POWER_AUTO:
#if 0
				FUNCTRACE(printk("%s> SW_POWER_AUTO\n", __FUNCTION__));
#else
				printk("%s> SW_POWER_AUTO\n", __FUNCTION__);
#endif
				buffalo_kernevnt_queuein("PSW_auto");;
				break;
			case SW_POWER_NS:
#if 0
				FUNCTRACE(printk("%s> SW_POWER_NS\n", __FUNCTION__));
#else
				printk("%s> SW_POWER_NS\n", __FUNCTION__);
#endif
				buffalo_kernevnt_queuein("PSW_unknown");;
				break;
			default:
				break;
		}
		FUNCTRACE(printk("%s\n", msg));
	}

	BuffaloSwPollingTimer.expires = (jiffies + SW_POLLING_INTERVAL);
	add_timer(&BuffaloSwPollingTimer);
	return 0;
}

static int
BuffaloSwPollingStop(void)
{
	del_timer(&BuffaloSwPollingTimer);
	return 0;
}

static int
BuffaloSwPollingStart(void)
{
	init_timer(&BuffaloSwPollingTimer);
	BuffaloSwPollingTimer.expires = (jiffies + SW_POLLING_INTERVAL);
	BuffaloSwPollingTimer.function = &BuffaloSwPollingCheck;
	BuffaloSwPollingTimer.data = 0;
	add_timer(&BuffaloSwPollingTimer);
	return 0;
}


static int
BuffaloSwPollingWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	FUNCTRACE(printk("%s> Entered\n", __FUNCTION__));
	if(strncmp(buffer, "on", 2) == 0 )
	{
		if(g_buffalo_sw_polling_control == 0)
		{
			g_buffalo_sw_polling_control = 1;
			g_buffalo_sw_status = SW_POWER_NS;
			BuffaloSwPollingStart();
		}
	}
	else if(strncmp(buffer, "off", 3) == 0)
	{
		if(g_buffalo_sw_polling_control == 1)
		{
			g_buffalo_sw_polling_control = 0;
			BuffaloSwPollingStop();
			g_buffalo_sw_status = SW_POWER_NS;
		}
	}
	return count;
}

static int
BuffaloSwPollingReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	FUNCTRACE(printk("%s> Entered\n", __FUNCTION__));

	if(g_buffalo_sw_polling_control == 1)
	{
		len = sprintf(page, "on\n");
	}
	else
	{
		len = sprintf(page, "off\n");
	}

	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);
}
#endif //CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER


#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
 #if !defined(BIT_FUNC)
  #error CONFIG_BUFFALO_USE_SWITCH_FUNC is defined, but BIT_FUNC is not defined.
 #endif

static int
BuffaloFuncReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int PinStat = buffalo_gpio_read();

	if(PinStat & BIT(BIT_FUNC))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}
#endif //CONFIG_BUFFALO_USE_SWITCH_FUNC


#if defined(CONFIG_BUFFALO_USE_LED_ETH)
 #if !defined(BIT_LED_ETH)
  #error CONFIG_BUFFALO_USE_LED_ETH is defined, but BIT_LED_ETH is not defined.
 #endif

static int
BuffaloEthLedReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	
	unsigned int PinStat = buffalo_gpio_read();
	if(PinStat & BIT(BIT_LED_ETH))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if(len > length)
		len = length;
	return (len);
}


static int
BuffaloEthLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){
	if(strncmp(buffer, "on", 2) == 0){
		BuffaloGpio_EthLedOn();
	}else if(strncmp(buffer, "off", 3) == 0){
		BuffaloGpio_EthLedOff();
	}
	return count;
}
#endif //CONFIG_BUFFALO_USE_LED_ETH


#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
 #if !defined(BIT_FAN_LCK)
  #error CONFIG_BUFFALO_USE_FAN_LOCK_DETECT is defined, but BIT_FAN_LCK is not defined.
 #endif

static int
BuffaloFanStatusReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	
	unsigned int PinStat = buffalo_gpio_read();
	FUNCTRACE(printk(" %s > ~PinStat & BIT(BIT_FAN_LCK) = %d\n", __FUNCTION__, (~PinStat & BIT(BIT_FAN_LCK))));
	if(~PinStat & BIT(BIT_FAN_LCK))
		len = sprintf(page, "Fine\n");
	else
		len = sprintf(page, "Stop\n");
	*start = page + (offset - begin);
	len -= (offset - begin);
	if(len > length)
		len = length;
	return (len);
}

static int
BuffaloFanStatusWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){
	if(strncmp(buffer, "on", 2) == 0){
		//Nothing to do ...
	}else if(strncmp(buffer, "off", 3) == 0){
		//Nothing to do ...
	}
	return count;
}
#endif //CONFIG_BUFFALO_USE_FAN_LOCK_DETECT


#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
 #if !defined(BIT_FAN_LOW) || !defined(BIT_FAN_HIGH)
  #erorr CONFIG_BUFFALO_USE_FAN_CONTROL is defined, but BIT_FAN_LOW and BIT_FAN_HIGH are not defined.
 #endif

static int
BuffaloFanControlWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "stop", strlen("stop")) == 0)
	{
		BuffaloGpio_FanStop();
	}
	else if(strncmp(buffer, "slow", strlen("slow")) == 0)
	{
		BuffaloGpio_FanSlow();
	}
	else if(strncmp(buffer, "fast", strlen("fast")) == 0)
	{
		BuffaloGpio_FanFast();
	}
	else if(strncmp(buffer, "full", strlen("full")) == 0)
	{
		BuffaloGpio_FanFull();
	}
	return count;
}

static int
BuffaloFanControlReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;

	unsigned int fan_status = BuffaloGpio_FanControlRead();

	switch(fan_status){
	case BUFFALO_GPIO_FAN_STOP:
		len += sprintf(page, "stop\n");
		break;
	case BUFFALO_GPIO_FAN_SLOW:
		len += sprintf(page, "slow\n");
		break;
	case BUFFALO_GPIO_FAN_FAST:
		len += sprintf(page, "fast\n");
		break;
	case BUFFALO_GPIO_FAN_FULL:
		len += sprintf(page, "full\n");
		break;
	default:
		break;
	}// end of switch

	*start = page + (offset - begin);
	len -= (offset - begin);
	if(len > length)
		len = length;
	return (len);
}
#endif //CONFIG_BUFFALO_USE_FAN_CONTROL

/* static int BuffaloFanStatusWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){ */
/* 	if(strncmp(buffer, "on", 2) == 0){ */
/* 		//Nothing to do ... */
/* 	}else if(strncmp(buffer, "off", 3) == 0){ */
/* 		//Nothing to do ... */
/* 	} */
/* 	return count; */
/* } */

static int BuffaloCpuStatusReadProc(char *page, char **start, off_t offset, int length)
{
	int len = 0;
	off_t begin = 0;
	unsigned int CpuStatus = BuffaloGpio_ChangePowerStatus(0);

	switch(CpuStatus){
	case MagicKeyReboot:
		len = sprintf(page, "reboot\n");
		break;
	case MagicKeyRebootUbootPassed:
		len = sprintf(page, "reboot_uboot_passed\n");
		break;
	case MagicKeyNormalState:
		len = sprintf(page, "normal_state\n");
		break;
	case MagicKeyHwPoff:
		len = sprintf(page, "hwpoff\n");
		break;
	case MagicKeySwPoff:
		len = sprintf(page, "swpoff\n");
		break;
	case MagicKeySWPoffUbootPassed:
		len = sprintf(page, "swpoff_uboot_passed\n");
		break;
	case MagicKeyFWUpdating:
		len = sprintf(page, "fwup\n");
		break;
	case MagicKeyUpsShutdown:
		len = sprintf(page, "ups_shutdown\n");
		break;
	default:
		len = sprintf(page, "Unknown(CpuStatus=%d)\n", CpuStatus);
		break;
	}

	*start = page + (offset - begin);
	len -= (offset - begin);
	if (len > length)
		len = length;
	return (len);

}

static int BuffaloCpuStatusWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){
	
	if(strncmp(buffer, "reboot", 6) == 0){
		PST_TRACE(printk("%s > setting reboot\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_REBOOTING);
	}else if(strncmp(buffer, "reboot_uboot_passed", 19) == 0){
		PST_TRACE(printk("%s > setting Reboot_uboot_passed\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_REBOOTING_UBOOT_PASSED);
	}else if(strncmp(buffer, "normal_state", 12) == 0){
		PST_TRACE(printk("%s > setting normal_state\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_NORMAL_STATE);
	}else if(strncmp(buffer, "hwpoff", 6) == 0){
		PST_TRACE(printk("%s > setting hwpoff\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_HW_POWER_OFF);
	}else if(strncmp(buffer, "swpoff", 6) == 0){
		PST_TRACE(printk("%s > setting swpoff\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_SW_POWER_OFF);
	}else if(strncmp(buffer, "swpoff_uboot_passed", 19) == 0){
		PST_TRACE(printk("%s > setting swpoff_uboot_passed\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_SW_POFF_UBOOT_PASSED);
	}else if(strncmp(buffer, "fwup", 4) == 0){
		PST_TRACE(printk("%s > setting fwup\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_FWUPDATING);
	}else if(strncmp(buffer, "ups_shutdown", 12) == 0){
		PST_TRACE(printk("%s > setting ups_shutdown\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_UPS_SHUTDOWN);
	}else{
		PST_TRACE(printk("%s > no meaning...(%s)\n", __FUNCTION__, buffer));
	}

	return count;
}

//----------------------------------------------------------------------
int __init buffaloLedDriver_init (void)
{
	struct proc_dir_entry *generic;
	struct proc_dir_entry *cpu_status_proc;
	struct proc_dir_entry *gpio_proc, *switch_proc, *fan_proc, *led_proc, *led_all_proc, *power_control_proc;

#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
	struct proc_dir_entry *hdd_ctrl_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	struct proc_dir_entry *usb_ctrl_proc;
 #if defined(BIT_USB_PWR2)
	struct proc_dir_entry *usb_ctrl_proc2;
 #endif
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	struct proc_dir_entry *power_proc;
#else

#endif
	struct proc_dir_entry *slide_switch_proc;

#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
	struct proc_dir_entry *auto_power_proc;
	struct proc_dir_entry *sw_polling_control_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	struct proc_dir_entry *func_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
	struct proc_dir_entry *fan_lock_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	struct proc_dir_entry *power_led_proc;
	struct proc_dir_entry *power_led_blink_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	struct proc_dir_entry *alarm_led_proc;
	struct proc_dir_entry *alarm_led_blink_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	struct proc_dir_entry *info_led_proc;
	struct proc_dir_entry *info_led_blink_proc; 
#endif

#if defined(CONFIG_BUFFALO_USE_LED_ETH)
	struct proc_dir_entry *eth_led_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	struct proc_dir_entry *func_led_proc;
	struct proc_dir_entry *func_led_blink_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
	struct proc_dir_entry *fan_control_proc;
#endif


	FUNCTRACE(printk("%s > Entered.\n",__FUNCTION__));

	BuffaloGpio_Init();
	gpio_proc = proc_mkdir("buffalo/gpio", 0);
	led_proc = proc_mkdir("buffalo/gpio/led", 0);
	switch_proc = proc_mkdir("buffalo/gpio/switch", 0);
	fan_proc = proc_mkdir("buffalo/gpio/fan", 0);
	power_control_proc = proc_mkdir("buffalo/gpio/power_control", 0);
	generic = create_proc_info_entry ("buffalo/power_sw", 0, 0, gpio_power_read_proc);

	led_all_proc = create_proc_entry("buffalo/gpio/led/all", 0, 0);
	led_all_proc->read_proc = &BuffaloAllLedReadProc;
	led_all_proc->write_proc= &BuffaloAllLedWriteProc;

	cpu_status_proc = create_proc_entry("buffalo/cpu_status", 0, 0);
	cpu_status_proc->read_proc =&BuffaloCpuStatusReadProc;
	cpu_status_proc->write_proc=&BuffaloCpuStatusWriteProc;

#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	BuffaloGpio_AlarmLedDisable();

	alarm_led_proc = create_proc_entry("buffalo/gpio/led/alarm", 0, 0);
	alarm_led_proc->read_proc = &BuffaloAlarmLedReadProc;
	alarm_led_proc->write_proc= &BuffaloAlarmLedWriteProc;

	alarm_led_blink_proc = create_proc_entry("buffalo/gpio/led/alarm_blink", 0, 0);
	alarm_led_blink_proc->read_proc = &BuffaloAlarmLedBlinkReadProc;
	alarm_led_blink_proc->write_proc= &BuffaloAlarmLedBlinkWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	BuffaloGpio_InfoLedDisable();

	info_led_proc = create_proc_entry("buffalo/gpio/led/info", 0, 0);
	info_led_proc->read_proc = &BuffaloInfoLedReadProc;
	info_led_proc->write_proc= &BuffaloInfoLedWriteProc;

	info_led_blink_proc = create_proc_entry("buffalo/gpio/led/info_blink", 0, 0);
	info_led_blink_proc->read_proc = &BuffaloInfoLedBlinkReadProc;
	info_led_blink_proc->write_proc= &BuffaloInfoLedBlinkWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	BuffaloGpio_PowerLedEnable();

	power_led_proc = create_proc_entry("buffalo/gpio/led/power", 0, 0);
	power_led_proc->read_proc = &BuffaloPowerLedReadProc;
	power_led_proc->write_proc= &BuffaloPowerLedWriteProc;

	power_led_blink_proc = create_proc_entry("buffalo/gpio/led/power_blink", 0, 0);
	power_led_blink_proc->read_proc = &BuffaloPowerLedBlinkReadProc;
	power_led_blink_proc->write_proc= &BuffaloPowerLedBlinkWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	BuffaloGpio_FuncLedDisable();

	func_led_proc = create_proc_entry("buffalo/gpio/led/func", 0, 0);
	func_led_proc->read_proc = &BuffaloFuncLedReadProc;
	func_led_proc->write_proc= &BuffaloFuncLedWriteProc;

	func_led_blink_proc = create_proc_entry("buffalo/gpio/led/func_blink", 0, 0);
	func_led_blink_proc->read_proc = &BuffaloFuncLedBlinkReadProc;
	func_led_blink_proc->write_proc= &BuffaloFuncLedBlinkWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_LED_ETH)
	eth_led_proc = create_proc_entry("buffalo/gpio/led/eth", 0, 0);
	eth_led_proc->read_proc = &BuffaloEthLedReadProc;
	eth_led_proc->write_proc= &BuffaloEthLedWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
	hdd_ctrl_proc = create_proc_entry("buffalo/gpio/power_control/hdd0", 0, 0);
	hdd_ctrl_proc->read_proc = &BuffaloHddReadProc;
	hdd_ctrl_proc->write_proc= &BuffaloHddWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	usb_ctrl_proc = create_proc_entry("buffalo/gpio/power_control/usb0", 0, 0);
	usb_ctrl_proc->read_proc =&BuffaloUsbReadProc;
	usb_ctrl_proc->write_proc=&BuffaloUsbWriteProc;
 #if defined(BIT_USB_PWR2)
	usb_ctrl_proc2 = create_proc_entry("buffalo/gpio/power_control/usb1", 0, 0);
	usb_ctrl_proc2->read_proc =&BuffaloUsbReadProc2;
	usb_ctrl_proc2->write_proc=&BuffaloUsbWriteProc2;
 #endif
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	power_proc = create_proc_entry("buffalo/gpio/switch/power", 0, 0);
	power_proc->read_proc =&BuffaloPowerReadProc;
	power_proc->write_proc=&BuffaloPowerReadProc;
#else

#endif
	slide_switch_proc = create_proc_info_entry("buffalo/gpio/switch/slide_switch", 0, 0, BuffaloSlideSwitchReadProc);

#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
	auto_power_proc = create_proc_entry("buffalo/gpio/switch/auto_power", 0, 0);
	auto_power_proc->read_proc =&BuffaloAutoPowerReadProc;
	auto_power_proc->write_proc=&BuffaloAutoPowerReadProc;

	sw_polling_control_proc = create_proc_entry("buffalo/gpio/switch/sw_control", 0, 0);
	sw_polling_control_proc->read_proc = &BuffaloSwPollingReadProc;
	sw_polling_control_proc->write_proc= &BuffaloSwPollingWriteProc;
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	func_proc = create_proc_entry("buffalo/gpio/switch/func", 0, 0);
	func_proc->read_proc = &BuffaloFuncReadProc;
	func_proc->write_proc= &BuffaloFuncReadProc;
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
	fan_control_proc = create_proc_entry("buffalo/gpio/fan/control", 0, 0);
	fan_control_proc->read_proc =&BuffaloFanControlReadProc;
	fan_control_proc->write_proc=&BuffaloFanControlWriteProc;
#endif


#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
	fan_lock_proc = create_proc_entry("buffalo/gpio/fan/lock", 0, 0);
	fan_lock_proc->read_proc =&BuffaloFanStatusReadProc;
	fan_lock_proc->write_proc=&BuffaloFanStatusWriteProc;
#endif

	printk("%s %s Ver.%s installed.\n", DRIVER_NAME, AUTHOR, BUFFALO_DRIVER_VER);
	return 0;
}

//----------------------------------------------------------------------
void buffaloLedDriver_exit(void)
{
	FUNCTRACE(printk(">%s\n",__FUNCTION__));
#if defined(CONFIG_BUFFALO_LED_INFO)
	BuffaloLed_InfoBlinkStop();
#endif

#if defined(CONFIG_BUFFALO_LED_POWER)
	BuffaloLed_PowerBlinkStop();
#endif

#if defined(CONFIG_BUFFALO_LED_ALARM)
	BuffaloLed_AlarmBlinkStop();
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
	remove_proc_entry("buffalo/gpio/fan/control", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
	remove_proc_entry("buffalo/gpio/fan/lock", 0);
#endif
	remove_proc_entry("buffalo/gpio/fan", 0);

#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	remove_proc_entry("buffalo/gpio/led/alarm", 0);
	remove_proc_entry("buffalo/gpio/led/alarm_blink", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	remove_proc_entry("buffalo/gpio/led/info", 0);
	remove_proc_entry("buffalo/gpio/led/info_blink", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	remove_proc_entry("buffalo/gpio/led/power", 0);
	remove_proc_entry("buffalo/gpio/led/power_blink", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	remove_proc_entry("buffalo/gpio/led/func", 0);
	remove_proc_entry("buffalo/gpio/led/func_blink", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_LED_ETH)
	remove_proc_entry("buffalo/gpio/led/eth", 0);
#endif
	remove_proc_entry("buffalo/gpio/led/all", 0);
	remove_proc_entry("buffalo/gpio/led", 0);

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	remove_proc_entry("buffalo/gpio/switch/func", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
	remove_proc_entry("buffalo/gpio/switch/auto_power", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	remove_proc_entry("buffalo/gpio/switch/power", 0);
	remove_proc_entry("buffalo/gpio/switch/sw_control", 0);
#else

#endif

	remove_proc_entry("buffalo/gpio/switch", 0);


#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
	remove_proc_entry("buffalo/gpio/power_control/hdd0", 0);
#endif

#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	remove_proc_entry("buffalo/gpio/power_control/usb0", 0);
#endif
	remove_proc_entry("buffalo/gpio/power_control", 0);
	remove_proc_entry("buffalo/gpio", 0);

	remove_proc_entry ("buffalo", 0);
}

module_init(buffaloLedDriver_init);
module_exit(buffaloLedDriver_exit);

