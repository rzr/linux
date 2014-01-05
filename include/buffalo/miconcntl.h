#ifndef _MICONCNTL_H_
#define _MICONCNTL_H_

#ifdef CONFIG_BUFFALO_LINKSTATION_LSGL
 #ifdef CONFIG_CPU_ARM926T
    // see. 88F5182 User Manual : A.4.3. Main Interrupt Controller Registers 
    // BIT0 : irq 32
    // BIT2 : Micon
    // BIT3 : RTC
    //
    #define MICON_IRQ (32+2)
 #endif
#elif defined (CONFIG_BUFFALO_TERASTATION_TSTGL)
  #define MICON_IRQ 18
#endif

void miconCntl_Reboot(void);
void miconCntl_PowerOff(void);


#endif
