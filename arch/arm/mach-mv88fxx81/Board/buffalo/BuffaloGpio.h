#ifndef _BUFFALO_GPIO_H_
#define _BUFFALO_GPIO_H_

#define BIT(x)	(1<<x)

#if defined(CONFIG_BUFFALO_LINKSTATION_LSGL)
	#define BIT_MICON  2
	#define MICON_POL  0		/* high */
	#define BIT_RTC    3
#elif defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
	#define BIT_MICON  8
	#define MICON_POL  0		/* high */
	#define BIT_RTC    9
	#define BIT_UPS		13
	#define BIT_OMR_BL	14
#elif defined(CONFIG_BUFFALO_LINKSTATION_HSWDHTGL_R1)
	#define BIT_MICON 2
	#define MICON_POL 0
	#define BIT_RTC   3
#elif defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWSGL_R1) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWHGL_R1) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V3) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V2)
 #if defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1)
	#define BIT_HDD_PWR	1
	#define BIT_USB_PWR	9
	#define BIT_LED_INFO	3
	#define BIT_LED_PWR	0
	#define BIT_LED_ALARM	2
	#define BIT_FAN_LCK	6
	#define BIT_INIT	7
	#define BIT_POWER	8
	#define BIT_AUTO_POWER	10
	#define BIT_LED_ETH	11 //dummy
	#define BIT_FAN_LOW	17
	#define BIT_FAN_HIGH	14
 #elif defined(CONFIG_BUFFALO_LINKSTATION_LSWSGL_R1)
	#define CONFIG_BUFFALO_USE_LED_POWER_REVERSE
	#define BIT_HDD_PWR		1
	#define BIT_HDD_PWR1		19
	#define BIT_USB_PWR		16
	#define BIT_LED_INFO		3
	#define BIT_LED_PWR		14
	#define BIT_LED_ALARM		2
	#define BIT_LED_FUNC		9
	#define BIT_LED_RESERVE1	0
	#define BIT_LED_RESERVE2	14
	#define BIT_LED_RESERVE3	14
	#define BIT_LED_RESERVE4	14
	#define BIT_POWER		18
	#define BIT_FUNC		15
	#define BIT_AUTO_POWER		17
	#define BIT_LED_ETH		11 //dummy
 #elif defined(CONFIG_BUFFALO_LINKSTATION_LSWHGL_R1) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V3) || \
	defined(CONFIG_BUFFALO_LINKSTATION_LSWTGL_R1_V2)
//	#define CONFIG_BUFFALO_USE_LED_POWER_REVERSE
	#define BIT_HDD_PWR		1
//	#define BIT_HDD_PWR1		19
	#define BIT_USB_PWR		9
	#define BIT_USB_PWR2		19
	#define BIT_LED_INFO		3
	#define BIT_LED_PWR		0
	#define BIT_LED_ALARM		2
	#define BIT_LED_FUNC		18
//	#define BIT_LED_RESERVE1	0
//	#define BIT_LED_RESERVE2	14
//	#define BIT_LED_RESERVE3	14
//	#define BIT_LED_RESERVE4	14
	#define BIT_POWER		10
	#define BIT_FUNC		8
	#define BIT_AUTO_POWER		22
	#define BIT_LED_ETH		11 //dummy
	#define BIT_FAN_LCK		6
	#define BIT_FAN_LOW		17
	#define BIT_FAN_HIGH		14
 #endif

	#define BUFFALO_GPIO_FAN_STOP	0
	#define BUFFALO_GPIO_FAN_SLOW	1
	#define BUFFALO_GPIO_FAN_FAST	2
	#define BUFFALO_GPIO_FAN_FULL	3

	// prototypes.
	void BuffaloGpio_CPUInterruptsSetup(void);
	void BuffaloGpio_CPUInterruptsClear(void);
 #if defined(CONFIG_BUFFALO_USE_SWITCH_AUTOPOWER)
	void BuffaloGpio_CPUInterruptsSetupAutopower(void);
	void BuffaloGpio_CPUInterruptsClearAutopower(void);
 #endif
 #if defined(CONFIG_BUFFALO_USE_SWITCH_INIT)
	void BuffaloGpio_CPUInterruptsSetupInit(void);
	void BuffaloGpio_CPUInterruptsClearInit(void);
 #endif
 #if defined(CONFIG_BUFFALO_USE_SWITCH_FUNC)
	void BuffaloGpio_CPUInterruptsSetupFunc(void);
	void BuffaloGpio_CPUInterruptsClearFunc(void);
 #endif
	unsigned int buffalo_gpio_read(void);
	unsigned int buffalo_gpio_blink_reg_read(void);
	void BuffaloGpio_Init(void);
	void BuffaloGpio_LedEnable(unsigned int);
	void BuffaloGpio_LedDisable(unsigned int);
	void BuffaloGpio_LedBlinkEnable(unsigned int);
	void BuffaloGpio_LedBlinkDisable(unsigned int);
 #if defined(CONIFG_BUFFALO_USE_HDD_POWER_CONTROL)
	void BuffaloGpio_HddPowerOff(void);
	void BuffaloGpio_HddPowerOn(void);
 #endif
 #if defined(CONFIG_BUFFALO_USE_USB_POWER_CONTROL)
	void BuffaloGpio_UsbPowerOff(void);
	void BuffaloGpio_UsbPowerOn(void);
 #endif
	unsigned int BuffaloGpio_GetAutoPowerStatus(void);
	void BuffaloGpio_CpuReset(void);
	void BuffaloGpio_EthLedOn(void);
	void BuffaloGpio_EthLedOff(void);
	void BuffaloGpio_AllLedOff(void);
	void BuffaloGpio_AllLedOn(void);
	uint8_t BuffaloGpio_ChangePowerStatus(uint8_t);
 #if defined(CONFIG_BUFFALO_USE_FAN_CONTROL)
	#define BUFFALO_GPIO_FAN_STOP	0
	#define BUFFALO_GPIO_FAN_SLOW	1
	#define BUFFALO_GPIO_FAN_FAST	2
	#define BUFFALO_GPIO_FAN_FULL	3

	void BuffaloGpio_FanControlWrite(unsigned int);
	unsigned int BuffaloGpio_FanControlRead(void);
 #endif

	// convenient macros.
 #if defined(CONFIG_BUFFALO_USE_LED_ALARM)
	#define BuffaloGpio_AlarmLedEnable()	 	BuffaloGpio_LedEnable(BIT_LED_ALARM)
	#define BuffaloGpio_AlarmLedDisable()		BuffaloGpio_LedDisable(BIT_LED_ALARM)
	#define BuffaloGpio_AlarmLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_LED_ALARM)
	#define BuffaloGpio_AlarmLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_LED_ALARM)
 #endif

 #if defined(CONFIG_BUFFALO_USE_LED_INFO)
	#define BuffaloGpio_InfoLedEnable()		BuffaloGpio_LedEnable(BIT_LED_INFO)
	#define BuffaloGpio_InfoLedDisable()		BuffaloGpio_LedDisable(BIT_LED_INFO)
	#define BuffaloGpio_InfoLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_LED_INFO)
	#define BuffaloGpio_InfoLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_LED_INFO)
 #endif // CONFIG_BUFFALO_USE_LED_INFO

 #if defined(CONFIG_BUFFALO_USE_LED_POWER)
	#define BuffaloGpio_PowerLedEnable()		BuffaloGpio_LedEnable(BIT_LED_PWR)
	#define BuffaloGpio_PowerLedDisable()		BuffaloGpio_LedDisable(BIT_LED_PWR)
	#define BuffaloGpio_PowerLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_LED_PWR)
	#define BuffaloGpio_PowerLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_LED_PWR)
 #endif // CONFIG_BUFFALO_USE_LED_POWER

 #if defined(CONFIG_BUFFALO_USE_LED_FUNC)
	#define BuffaloGpio_FuncLedEnable()		BuffaloGpio_LedEnable(BIT_LED_FUNC)
	#define BuffaloGpio_FuncLedDisable()		BuffaloGpio_LedDisable(BIT_LED_FUNC)
	#define BuffaloGpio_FuncLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_LED_FUNC)
	#define BuffaloGpio_FuncLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_LED_FUNC)
 #endif // CONFIG_BUFFALO_USE_LED_FUNC

	#define BuffaloGpio_FanStop()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_STOP)
	#define BuffaloGpio_FanSlow()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_SLOW)
	#define BuffaloGpio_FanFast()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_FAST)
	#define BuffaloGpio_FanFull()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_FULL)

	// variable definition.
	#define POWER_STATUS_REBOOTING                  1
	#define POWER_STATUS_REBOOTING_UBOOT_PASSED     2
	#define POWER_STATUS_NORMAL_STATE               3
	#define POWER_STATUS_HW_POWER_OFF               4
	#define POWER_STATUS_SW_POWER_OFF               5
	#define POWER_STATUS_SW_POFF_UBOOT_PASSED       6
	#define POWER_STATUS_FWUPDATING                 7
	#define POWER_STATUS_REBOOT_REACHED_HALT        8
	#define	POWER_STATUS_SW_POFF_REACHED_HALT       9
	#define POWER_STATUS_UPS_SHUTDOWN		10
	#define POWER_STATUS_UPS_SHUTDOWN_REACHED_HALT	11

	#define MagicKeyReboot                  0x18
	#define MagicKeyRebootUbootPassed       0x3a
	#define MagicKeyNormalState             0x71
	#define MagicKeyHwPoff                  0x43
	#define MagicKeySwPoff                  0x02
	#define MagicKeySWPoffUbootPassed       0x5c
	#define MagicKeyFWUpdating              0x6f
	#define MagicKeyRebootReachedHalt       0x2b
	#define MagicKeySWPoffReachedHalt       0x7a
	#define MagicKeyUpsShutdown		0x21
	#define MagicKeyUpsShutdownReachedHalt	0x32

#else
 #error
#endif


void BuffaloGpio_MiconIntSetup(void);
void BuffaloGpio_ClearMiconInt(void);

void BuffaloGpio_RtcIntSetup(void);
void BuffaloGpio_ClearRtcInt(void);

void BuffaloPrintGpio(void);

#ifdef CONFIG_BUFFALO_USE_UPS
void BuffaloGpio_UPSPortEnable(void);
void BuffaloGpio_UPSPortDisable(void);
unsigned int BuffaloGpio_UPSPortScan(void); // for debug

void BuffaloGpio_UPSOmronBLEnable(void);
void BuffaloGpio_UPSOmronBLDisable(void);
unsigned int BuffaloGpio_UPSOmronBLGetStatus(void);
unsigned int BuffaloGpio_UPSOmronBLUseStatus(void);

void BuffaloRtc_UPSRecoverInit(void);
void BuffaloRtc_UPSRecoverEnable(int8_t);
void BuffaloRtc_UPSRecoverDisable(void);
int BuffaloRtc_UPSRecoverReadStatus(void);

#define RECOVER_TARGET_UPS_APC	1
#define RECOVER_TARGET_UPS_OMR	2
#define RECOVER_TARGET_UPS_USB	3
#endif

#endif

