#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>

#include "BuffaloGpio.h"
#include "mvGpp.h"
#include "mvGppRegs.h"

#include "mvCpuIfRegs.h"

#include "mvDS1339.h"
#include "mvDS1339Reg.h"

#if defined CONFIG_BUFFALO_USE_GPIO_DRIVER
 #include "mvEthPhy.h"
#endif

#define MagicKeyAPC	0x45
#define MagicKeyOMR	0x3a
#define MagicKeyUSB	0x16

/*
	GPIO-setup
	LS-GL	BIT2 : micon-int
			BIT3 : RTC-int
	TS-HTGL
			BIT8 : micon-int
			BIT9 : RTC-int
			BIT13: UPS-SerailPort
			BIT14: OMRON_UPS Battery Low detection
*/

#define BIT(x) (1<<x)

#if defined CONFIG_BUFFALO_USE_MICON
//----------------------------------------------------------------------
// micon
//----------------------------------------------------------------------
void BuffaloGpio_MiconIntSetup(void)
{
	mvGppOutEnablle(0, BIT(BIT_MICON) , BIT(BIT_MICON));		// disable output
	mvGppPolaritySet(MICON_POL, BIT(BIT_MICON) , BIT(BIT_MICON));
	// set GPIO-2 : edge IRQ
	///set_irq_handler(32+BIT_MICON, do_edge_IRQ);
}

//----------------------------------------------------------------------
void BuffaloGpio_ClearMiconInt(void)
{
	unsigned cause;
	//printk(">%s\n",__FUNCTION__);
	/*
	printk("MPP_CONTROL_REG  %x %x %x %x\n"
		,MV_REG_READ(mvMppRegGet(0))
		,MV_REG_READ(mvMppRegGet(1))
		,MV_REG_READ(mvMppRegGet(2))
		,MV_REG_READ(mvMppRegGet(3))
		);
	*/
	
	cause = MV_REG_READ(GPP_INT_CAUSE_REG(0));
	//printk("cause:%x\n",cause);
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0), ~(cause & BIT(BIT_MICON)));
}
#endif // of CONFIG_BUFFALO_USE_MICON

#if defined CONFIG_BUFFALO_USE_MICON
//----------------------------------------------------------------------
// RTC
//----------------------------------------------------------------------
void BuffaloGpio_RtcIntSetup(void)
{
	mvGppOutEnablle(0, BIT(BIT_RTC) , BIT(BIT_RTC));		// disable output
	mvGppPolaritySet(0, BIT(BIT_RTC) , 0);
	MV_REG_WRITE(GPP_INT_LVL_REG(0), 0);
}

//----------------------------------------------------------------------
void BuffaloGpio_ClearRtcInt(void)
{
	unsigned cause;
	
	cause = MV_REG_READ(GPP_INT_CAUSE_REG(0));
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0), ~(cause & BIT(BIT_RTC)));
}
#endif // of CONFIG_BUFFALO_USE_MICON


#if defined CONFIG_BUFFALO_USE_INTERRUPT_DRIVER
 #if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
void
BuffaloGpio_CPUInterruptsSetup(void)
{
}

void
BuffaloGpio_CPUInterruptsClear(void)
{
}

 #else

void
BuffaloGpio_CPUInterruptsSetup(void)
{
	mvGppOutEnablle(0, BIT(BIT_POWER), BIT(BIT_POWER));
	mvGppPolaritySet(0, BIT(BIT_POWER), (MV_GPP_IN_INVERT & BIT(BIT_POWER)));
}

void
BuffaloGpio_CPUInterruptsClear(void)
{
	unsigned int cause;
	printk("MPP_CONTROL_REG 0x%04x 0x%04x 0x%04x 0x%04x\n",
		MV_REG_READ(mvMppRegGet(0)),
		MV_REG_READ(mvMppRegGet(1)),
		MV_REG_READ(mvMppRegGet(2)),
		MV_REG_READ(mvMppRegGet(3)));

	cause = MV_REG_READ(GPP_INT_CAUSE_REG(0));
	printk("cause:0x%04x\n", (unsigned int)cause);
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0), ~(cause & BIT(BIT_POWER)));
	//BuffaloGpio_EthLedOn();
}
 #endif //CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER

 #if defined(CONFIG_BUFFALO_USE_SWITCH_INIT)
void
BuffaloGpio_CPUInterruptsSetupInit(void)
{
	mvGppOutEnablle(0, BIT(BIT_INIT), BIT(BIT_INIT));
	mvGppPolaritySet(0, BIT(BIT_INIT), (MV_GPP_IN_INVERT & BIT(BIT_INIT)));
}

void
BuffaloGpio_CPUInterruptsClearInit(void)
{
	unsigned int cause;
	printk("MPP_CONTROL_REG 0x%04x 0x%04x 0x%04x 0x%04x\n",
		MV_REG_READ(mvMppRegGet(0)),
		MV_REG_READ(mvMppRegGet(1)),
		MV_REG_READ(mvMppRegGet(2)),
		MV_REG_READ(mvMppRegGet(3)));

	cause = MV_REG_READ(GPP_INT_CAUSE_REG(0));
	printk("cause:0x%04x\n", cause);
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0), ~(cause & BIT(BIT_INIT)));
}
 #endif //CONFIG_BUFFALO_USE_SWITCH_INIT

 #if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
void
BuffaloGpio_CPUInterruptsSetupFunc(void)
{
	mvGppValueSet(0, BIT(BIT_FUNC), BIT(BIT_FUNC));
	mvGppPolaritySet(0, BIT(BIT_FUNC), (MV_GPP_IN_INVERT & BIT(BIT_FUNC)));
}

void
BuffaloGpio_CPUInterruptsClearFunc(void)
{
	unsigned int cause;
	printk("MPP_CONTROL_REG 0x%04x 0x%04x 0x%04x 0x%04x\n",
		MV_REG_READ(mvMppRegGet(0)),
		MV_REG_READ(mvMppRegGet(1)),
		MV_REG_READ(mvMppRegGet(2)),
		MV_REG_READ(mvMppRegGet(3)));

	cause = MV_REG_READ(GPP_INT_CAUSE_REG(0));
	printk("cause:0x%04x\n", cause);
	MV_REG_WRITE(GPP_INT_CAUSE_REG(0), ~(cause & BIT(BIT_FUNC)));
}
 #endif // CONFIG_BUFFALO_USE_SWITCH_FUNC
#endif //CONFIG_BUFFALO_INTERRUPT_DRIVER

unsigned int
buffalo_gpio_read(void)
{
	unsigned int retval = 0;
	retval = mvGppValueGet(0, -1);

	// get usb power condition
#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	retval = (retval & ~BIT(BIT_USB_PWR)) | (MV_REG_READ(GPP_DATA_OUT_REG(0)) & BIT(BIT_USB_PWR));
#endif

#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
	// get hdd power condition
	retval = (retval & ~BIT(BIT_HDD_PWR)) | (MV_REG_READ(GPP_DATA_OUT_REG(0)) & BIT(BIT_HDD_PWR));
#endif

#if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	#if 0
	// in the case using push switch type, inverse result returnig.
	if((retval & BIT(BIT_FUNC))) // func bit is 1
		retval = (retval & ~BIT(BIT_FUNC)); // func bit set to 0
	else // func bit is 0
		retval = (retval | BIT(BIT_FUNC)); // func bit set to 1
	#endif
#endif

#if defined(CONFIG_BUFFALO_USE_LED_POWER_REVERSE)
	// reverse LED_PWR status
	if((retval & BIT(BIT_LED_PWR)))
	{
		// BIT_LED_PWR is 1 case.
		// set it donw to 0.
		retval = (retval & ~BIT(BIT_LED_PWR));
	}
	else
	{
		// BIT_LED_PWR is 0 case.
		// set it up to 1.
		retval = (retval | BIT(BIT_LED_PWR));
	}
	// reverse completed.
#endif
	return retval;
}

unsigned int
buffalo_gpio_blink_reg_read(void)
{
	return (unsigned int) MV_REG_READ(GPP_BLINK_EN_REG(0));
}

void
BuffaloGpio_Init(void)
{
	mvGppValueSet(0,
		(0                   |
#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
		 BIT(BIT_LED_ALARM)  |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_INFO)
		 BIT(BIT_LED_INFO)   |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_POWER)
		 BIT(BIT_LED_PWR)    |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
		 BIT(BIT_LED_FUNC)   |
#endif
#if defined(CONFIG_BUFFALO_USE_SWITCH_USE_SLIDE_POWER)
		 BIT(BIT_POWER)      |
#endif
#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
		 BIT(BIT_AUTO_POWER) |
#endif
#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
		 BIT(BIT_FAN_LOW)    |
		 BIT(BIT_FAN_HIGH)   |
#endif
#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
		 BIT(BIT_FAN_LCK)    |
#endif
#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
		 BIT(BIT_USB_PWR)    |
#endif
#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
		 BIT(BIT_HDD_PWR)    |
 #if defined(BIT_HDD_PWR1)
		 BIT(BIT_HDD_PWR1)   |
 #endif
#endif
		 0),
		(0                                 |
#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
		 (MV_GPP_OUT & BIT(BIT_LED_ALARM)) |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_INFO)
		 (MV_GPP_OUT & BIT(BIT_LED_INFO))  |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_POWER)
		 (MV_GPP_OUT & BIT(BIT_LED_PWR))   |
#endif
#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
		 (MV_GPP_OUT & BIT(BIT_LED_FUNC))  |
#endif
#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
		 (MV_GPP_OUT & BIT(BIT_POWER))     |
#endif
#if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
		 (MV_GPP_OUT & BIT(BIT_AUTO_POWER)) |
#endif
#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
		 (MV_GPP_OUT & BIT(BIT_FAN_LOW))   |
		 (MV_GPP_OUT & BIT(BIT_FAN_HIGH))  |
#endif
#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
		 (MV_GPP_IN & BIT(BIT_FAN_LCK))    |
#endif
#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
		 (MV_GPP_OUT & BIT(BIT_USB_PWR))   |
#endif
#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
		 (MV_GPP_OUT & BIT(BIT_HDD_PWR))   |
 #if defined(BIT_HDD_PWR1)
		 (MV_GPP_OUT & BIT(BIT_HDD_PWR1))  |
 #endif
#endif
		0)
		);
#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL) || defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
	unsigned int tmp_regval, reg_mask;
	tmp_regval = 0;
	reg_mask = 0;
#endif

#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	mvGppPolaritySet(0, BIT(BIT_USB_PWR), (MV_GPP_IN_INVERT & BIT(BIT_USB_PWR)));
#endif
#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	mvGppPolaritySet(0, BIT(BIT_LED_INFO), (MV_GPP_IN_ORIGIN & BIT(BIT_LED_INFO)));
#endif
#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	mvGppPolaritySet(0, BIT(BIT_LED_ALARM), (MV_GPP_IN_ORIGIN & BIT(BIT_LED_ALARM)));
#endif
#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	mvGppPolaritySet(0, BIT(BIT_LED_FUNC), (MV_GPP_IN_ORIGIN & BIT(BIT_LED_FUNC)));
#endif
#if defined(CONFIG_BUFFALO_USE_SWITCH_SLIDE_POWER)
	mvGppPolaritySet(0, BIT(BIT_POWER), (MV_GPP_IN_INVERT & BIT(BIT_POWER)));
	mvGppPolaritySet(0, BIT(BIT_AUTO_POWER), (MV_GPP_IN_INVERT & BIT(BIT_AUTO_POWER)));
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
	mvGppPolaritySet(0, BIT(BIT_FAN_LOW), (MV_GPP_IN_INVERT & BIT(BIT_FAN_LOW)));
	mvGppPolaritySet(0, BIT(BIT_FAN_HIGH), (MV_GPP_IN_INVERT & BIT(BIT_FAN_HIGH)));

	tmp_regval = MV_REG_READ(GPP_DATA_OUT_REG(0));
	tmp_regval &= ~(BIT(BIT_FAN_LOW) | BIT(BIT_FAN_HIGH));
	//tmp_regval |= (BIT(BIT_FAN_LOW) | BIT(BIT_FAN_HIGH)); //bootup time , setted to full
	MV_REG_WRITE(GPP_DATA_OUT_REG(0), tmp_regval);

	tmp_regval = MV_REG_READ(GPP_DATA_OUT_EN_REG(0));
	tmp_regval &= ~(BIT(BIT_FAN_LOW) | BIT(BIT_FAN_HIGH));
	tmp_regval |= ((MV_GPP_OUT_EN & BIT(BIT_FAN_LOW)) | (MV_GPP_OUT_EN & BIT(BIT_FAN_HIGH)));
	MV_REG_WRITE(GPP_DATA_OUT_EN_REG(0), tmp_regval);
#endif

#if defined(CONFIG_BUFFALO_USE_FAN_LOCK_DETECT)
	tmp_regval = (MV_REG_READ(GPP_DATA_OUT_REG(0)) & ~BIT(BIT_FAN_LCK)) | (MV_GPP_IN & BIT(BIT_FAN_LCK));
	MV_REG_WRITE(GPP_DATA_OUT_REG(0), tmp_regval);

	tmp_regval = (MV_REG_READ(GPP_DATA_OUT_EN_REG(0)) & ~BIT(BIT_FAN_LCK)) | (MV_GPP_OUT_DIS & BIT(BIT_FAN_LCK));
	MV_REG_WRITE(GPP_DATA_OUT_EN_REG(0), tmp_regval);
#endif
}

void
BuffaloGpio_LedEnable(unsigned int led_bit)
{
	//Outreg=0, OutEnReg=0, actlow=0, Din=0
#if defined(CONFIG_BUFFALO_USE_LED_POWER_REVERSE)
	if (led_bit == BIT_LED_PWR)
	{
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
		MV_REG_BIT_SET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
	}
	else
	{
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
	}
#else
	MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
	MV_REG_BIT_RESET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
#endif
}

void
BuffaloGpio_LedDisable(unsigned int led_bit)
{
	//OutReg=1, OutEnReg=0, actlow=0, Din=1
#if defined(CONFIG_BUFFALO_USE_LED_POWER_REVERSE)
	if (led_bit == BIT_LED_PWR)
	{
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
	}
	else
	{
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
		MV_REG_BIT_SET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
	}	
#else
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(led_bit)));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(led_bit)));
	MV_REG_BIT_SET(GPP_DATA_IN_REG(0), (BIT(led_bit)));
#endif
}

void
BuffaloGpio_LedBlinkEnable(unsigned int led_bit)
{
	MV_REG_BIT_SET(GPP_BLINK_EN_REG(0), (BIT(led_bit)));
}

void
BuffaloGpio_LedBlinkDisable(unsigned int led_bit)
{
	MV_REG_BIT_RESET(GPP_BLINK_EN_REG(0), (BIT(led_bit)));
}

#if defined(CONFIG_BUFFALO_USE_HDD_POWER_CONTROL)
void
BuffaloGpio_HddPowerOff(void)
{
	MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_HDD_PWR)));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(BIT_HDD_PWR)));
 #if defined(BIT_HDD_PWR1)
	MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_HDD_PWR1)));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (BIT(BIT_HDD_PWR1)));
 #endif
}

void
BuffaloGpio_HddPowerOn(void)
{
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_HDD_PWR)));
	MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0), (BIT(BIT_HDD_PWR)));
 #if defined(BIT_HDD_PWR1)
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_HDD_PWR1)));
	MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0), (BIT(BIT_HDD_PWR1)));
 #endif
}
#endif


#if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
void
BuffaloGpio_UsbPowerOff(void)
{
	MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_USB_PWR)));
}

void
BuffaloGpio_UsbPowerOn(void)
{
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_USB_PWR)));
}

#if defined(BIT_USB_PWR2)
void
BuffaloGpio_UsbPowerOff2(void)
{
	MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_USB_PWR2)));
}

void
BuffaloGpio_UsbPowerOn2(void)
{
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_USB_PWR2)));
}
#endif

#endif // CONFIG_BUFFALO_USE_USB_POWER_CONTROL


void
BuffaloGpio_CpuReset(void)
{
	MV_REG_BIT_SET(CPU_RSTOUTN_MASK_REG, BIT(2));
	MV_REG_BIT_SET(CPU_SYS_SOFT_RST_REG, BIT(0));
}


#if defined(CONFIG_BUFFALO_USE_LED_ETH)
void
BuffaloGpio_EthLedOn(void)
{
	buffalo_link_led_on(0);
}

void
BuffaloGpio_EthLedOff(void)
{
	buffalo_link_led_off(0);
}
#endif //CONFIG_BUFFALO_USE_LED_ETH


void
BuffaloGpio_AllLedOff(void)
{
#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	BuffaloGpio_AlarmLedDisable();
	BuffaloGpio_AlarmLedBlinkDisable();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	BuffaloGpio_InfoLedDisable();
	BuffaloGpio_InfoLedBlinkDisable();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	BuffaloGpio_PowerLedDisable();
	BuffaloGpio_PowerLedBlinkDisable();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_ETH)
	BuffaloGpio_EthLedOff();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	BuffaloGpio_FuncLedDisable();
	BuffaloGpio_FuncLedBlinkDisable();
#endif
}

void
BuffaloGpio_AllLedOn(void)
{
#if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	BuffaloGpio_AlarmLedEnable();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_INFO)
	BuffaloGpio_InfoLedEnable();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_POWER)
	BuffaloGpio_PowerLedEnable();
#endif
#if defined(CONIFG_BUFFALO_USE_LED_ETH)
	BuffaloGpio_EthLedOn();
#endif
#if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	BuffaloGpio_FuncLedEnable();
#endif
}

#if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
void
BuffaloGpio_FanControlWrite(unsigned int value)
{
	switch(value){
	case BUFFALO_GPIO_FAN_STOP:
		//printk("%s > changing to stop\n", __FUNCTION__);
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_LOW)));
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_HIGH)));
		break;
	case BUFFALO_GPIO_FAN_SLOW:
		//printk("%s > changing to slow\n", __FUNCTION__);
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_LOW)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_HIGH)));
		break;
	case BUFFALO_GPIO_FAN_FAST:
		//printk("%s > changing to fast\n", __FUNCTION__);
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_LOW)));
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_HIGH)));
		break;
	case BUFFALO_GPIO_FAN_FULL:
		//printk("%s > changing to full\n", __FUNCTION__);
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_LOW)));
		MV_REG_BIT_RESET(GPP_DATA_OUT_REG(0), (BIT(BIT_FAN_HIGH)));
		break;
	default:
		break;
	} // end of switch
}

unsigned int
BuffaloGpio_FanControlRead(void)
{
	unsigned long fan_low = MV_REG_READ(GPP_DATA_OUT_REG(0)) & BIT(BIT_FAN_LOW);
	unsigned long fan_high= MV_REG_READ(GPP_DATA_OUT_REG(0)) & BIT(BIT_FAN_HIGH);

	if(! fan_low && ! fan_high)
	{
		return BUFFALO_GPIO_FAN_FULL;
	}
	else if(!fan_low && fan_high)
	{
		return BUFFALO_GPIO_FAN_FAST;
	}
	else if(fan_low && !fan_high)
	{
		return BUFFALO_GPIO_FAN_SLOW;
	}
	else if(fan_low && fan_high)
	{
		return BUFFALO_GPIO_FAN_STOP;
	}
	return 0;
}
#endif

uint8_t
BuffaloGpio_ChangePowerStatus(uint8_t ChangeType){

        uint8_t MagicKey=0;

        switch(ChangeType){
        case POWER_STATUS_REBOOTING:
                MagicKey = MagicKeyReboot;
                break;
        case POWER_STATUS_REBOOTING_UBOOT_PASSED:
                MagicKey = MagicKeyRebootUbootPassed;
                break;
        case POWER_STATUS_NORMAL_STATE:
                MagicKey = MagicKeyNormalState;
                break;
        case POWER_STATUS_HW_POWER_OFF:
                MagicKey = MagicKeyHwPoff;
                break;
        case POWER_STATUS_SW_POWER_OFF:
                MagicKey = MagicKeySwPoff;
                break;
        case POWER_STATUS_SW_POFF_UBOOT_PASSED:
                MagicKey = MagicKeySWPoffUbootPassed;
                break;
        case POWER_STATUS_FWUPDATING:
                MagicKey = MagicKeyFWUpdating;
                break;
        case POWER_STATUS_REBOOT_REACHED_HALT:
                MagicKey = MagicKeyRebootReachedHalt;
                break;
        case POWER_STATUS_SW_POFF_REACHED_HALT:
                MagicKey = MagicKeySWPoffReachedHalt;
                break;
	case POWER_STATUS_UPS_SHUTDOWN:
		MagicKey = MagicKeyUpsShutdown;
		break;
	case POWER_STATUS_UPS_SHUTDOWN_REACHED_HALT:
		MagicKey = MagicKeyUpsShutdownReachedHalt;
		break;
        }

        if(MagicKey){
		//printk("%s > Writing 0x%02x\n", __FUNCTION__, MagicKey);
                BufRtcDS1339AlarmBSet(MagicKey);
        }else{
                BufRtcDS1339AlarmBGet(&MagicKey);
        }

	return MagicKey;
}



#ifdef CONFIG_BUFFALO_USE_UPS
// for ups codes
void BuffaloGpio_UPSPortEnable(void){

	mvGppOutEnablle(0, BIT(BIT_UPS), (MV_GPP_OUT_EN & BIT(BIT_UPS)));
	mvGppValueSet(0, BIT(BIT_UPS), BIT(BIT_UPS));

}

void BuffaloGpio_UPSPortDisable(void){

	mvGppOutEnablle(0, BIT(BIT_UPS), (MV_GPP_OUT_DIS & BIT(BIT_UPS)));
	mvGppValueSet(0, BIT(BIT_UPS), (0 & BIT(BIT_UPS)));

}

unsigned int BuffaloGpio_UPSPortScan(void){

	MV_U32 ret = mvGppValueGet(0, BIT(BIT_UPS));
	return (unsigned int)( ret & BIT(BIT_UPS));
	
}
// end of basically ups codes

////////////////////////////////////////////////
void BuffaloGpio_UPSOmronBLEnable(void){
	
	mvGppOutEnablle(0, BIT(BIT_OMR_BL), (MV_GPP_OUT_DIS & BIT(BIT_OMR_BL)));
	
}

void BuffaloGpio_UPSOmronBLDisable(void){
	
	mvGppOutEnablle(0, BIT(BIT_OMR_BL), (MV_GPP_OUT_EN & BIT(BIT_OMR_BL)));
	mvGppValueSet(0, BIT(BIT_OMR_BL), (0 & BIT(BIT_OMR_BL)));
	
}

unsigned int BuffaloGpio_UPSOmronBLGetStatus(void){
	
	MV_U32 ret = mvGppValueGet(0, BIT(BIT_OMR_BL));
	return (unsigned int)( ret & BIT(BIT_OMR_BL));
	
}

unsigned int BuffaloGpio_UPSOmronBLUseStatus(void){

	unsigned base=0xF1000000;								////caution base addr directry using!!!!///////
	volatile unsigned *gpioStat=(unsigned *)(base+GPP_DATA_OUT_EN_REG(0));
//	printk("0x%x\n", gpio[1]);
	return (gpioStat[0] & BIT(BIT_OMR_BL));
	
}
///////////////////////////////////////////////////////


// for ups recover function

void BuffaloRtc_UPSRecoverInit(void){
	mvRtcDS1339Init();
	BufRtcDS1339AlarmBSet(0);
}

void BuffaloRtc_UPSRecoverEnable(int8_t TargetType){

	int8_t MagicKey=0;

	switch(TargetType){
	case RECOVER_TARGET_UPS_APC:
		MagicKey = MagicKeyAPC;
		break;
	case RECOVER_TARGET_UPS_OMR:
		MagicKey = MagicKeyOMR;
		break;
	case RECOVER_TARGET_UPS_USB: 
		MagicKey = MagicKeyUSB;
		break;
	default:
		break;
	}
	
	if(MagicKey){
		BufRtcDS1339AlarmBSet(MagicKey);
	}
	
}

void BuffaloRtc_UPSRecoverDisable(void){
	
	BufRtcDS1339AlarmBSet(0);
}

int BuffaloRtc_UPSRecoverReadStatus(void){
	
	int8_t data;
	BufRtcDS1339AlarmBGet(&data);

	switch(data){
	case MagicKeyAPC:
		return RECOVER_TARGET_UPS_APC;
		break;
	case MagicKeyOMR:
		return RECOVER_TARGET_UPS_OMR;
		break;
	case MagicKeyUSB:
		return RECOVER_TARGET_UPS_USB;
		break;
	default:
		break;
	}
	return -1;
}
// end of recover function 


////
// for debug
////
/*
unsigned int BuffaloGpio_PortScan(void){
	
	MV_U32 ret = mvGppValueGet(0, 0xffff);
	
	return ret;
}*/

#endif  //CONFIG_BUFFALO_USE_UPS

//----------------------------------------------------------------------
// for Debug
//----------------------------------------------------------------------
/*
void BuffaloPrintGpio(void)
{
	printk("GPIO:Din=%x Int=%x mask=%x lmask=%x\n "
		,MV_REG_READ(GPP_DATA_IN_REG(0))
		,MV_REG_READ(GPP_INT_CAUSE_REG(0))
		,MV_REG_READ(GPP_INT_MASK_REG(0))
		,MV_REG_READ(GPP_INT_LVL_REG(0))
		);
}
*/

#if defined CONFIG_BUFFALO_USE_GPIO_DRIVER
 EXPORT_SYMBOL(BuffaloGpio_HddPowerOff);
 EXPORT_SYMBOL(BuffaloGpio_CpuReset);
#endif
