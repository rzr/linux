
#include "proslic.h"
#include <linux/kernel.h>
#include <linux/delay.h>
extern int mvTdmSpiRead(unsigned char addr, unsigned char *data);
extern int mvTdmSpiWrite(unsigned char addr, unsigned char data);

#define printf			printk
#define diag_printf		printk
#define Slic_os_delay		mdelay
#define TIMEOUTCODE (int) -1
char * exceptionStrings[] = 
{	"ProSLIC not communicating", 
	"Time out durring Power Up",
	"Time out durring Power Down",
	"Power is Leaking; might be a short",
	"Tip or Ring Ground Short",
	"Too Many Q1 Power Alarms",
	"Too Many Q2 Power Alarms",
	"Too Many Q3 Power Alarms", 
	"Too Many Q4 Power Alarms",
	"Too Many Q5 Power Alarms",
	"Too Many Q6 Power Alarms"
};
void exception (enum exceptions e)
{
	diag_printf( "\n\tE X C E P T I O N: %s\n",exceptionStrings[e] );
	diag_printf( " SLIC Interface Error\n");
}


int Si3215_Flag = 0;
int TestForSi3215(void)
{
	return ((readDirectReg(1) & 0x80)==0x80);
}

unsigned char readDirectReg(unsigned char address)
{
	unsigned char data;
	mvTdmSpiRead(address/*taken care by mvTdmSpiRead |0x80*/, &data);
	return data;
}

void writeDirectReg(unsigned char address, unsigned char data)
{
	mvTdmSpiWrite(address/*taken care by mvTdmSpiRead &0x7f*/, data);
}

void waitForIndirectReg(void)
{
	while (readDirectReg(I_STATUS));
}

unsigned short readIndirectReg(unsigned char address)
{ 
	if (Si3215_Flag) 
	{
		/* 
		** re-map the indirect registers for Si3215.
		*/
		if (address < 13 ) return 0;
		if ((address >= 13) && (address <= 40))
			address -= 13;
		else if ((address == 41) || (address == 43))
			address += 23;
		else if ((address >= 99) && (address <= 104))
			address -= 30;
	}

	waitForIndirectReg();
	writeDirectReg(IAA,address); 
	waitForIndirectReg();
	return ( readDirectReg(IDA_LO) | (readDirectReg (IDA_HI))<<8);
}

void writeIndirectReg(unsigned char address, unsigned short data)
{
	if (Si3215_Flag)
	{
		/* 
		** re-map the indirect registers for Si3215.
		*/
		if (address < 13 ) return;
		if ((address >= 13) && (address <= 40))
			address -= 13;
		else if ((address == 41) || (address == 43))
			address += 23;
		else if ((address >= 99) && (address <= 104))
			address -= 30;
	}

	waitForIndirectReg();
	writeDirectReg(IDA_LO,(unsigned char)(data & 0xFF));
	writeDirectReg(IDA_HI,(unsigned char)((data & 0xFF00)>>8));
	writeDirectReg(IAA,address);
}


/*
The following Array contains:
*/
indirectRegister  indirectRegisters[] =
{
/* Reg#			Label		Initial Value  */

{	0,	"DTMF_ROW_0_PEAK",	0x55C2	},
{	1,	"DTMF_ROW_1_PEAK",	0x51E6	},
{	2,	"DTMF_ROW2_PEAK",	0x4B85	},
{	3,	"DTMF_ROW3_PEAK",	0x4937	},
{	4,	"DTMF_COL1_PEAK",	0x3333	},
{	5,	"DTMF_FWD_TWIST",	0x0202	},
{	6,	"DTMF_RVS_TWIST",	0x0202	},
{	7,	"DTMF_ROW_RATIO",	0x0198	},
{	8,	"DTMF_COL_RATIO",	0x0198	},
{	9,	"DTMF_ROW_2ND_ARM",	0x0611	},
{	10,	"DTMF_COL_2ND_ARM",	0x0202	},
{	11,	"DTMF_PWR_MIN_",	0x00E5	},
{	12,	"DTMF_OT_LIM_TRES",	0x0A1C	},
{	13,	"OSC1_COEF",		0x7b30	},
{	14,	"OSC1X",			0x0063	},
{	15,	"OSC1Y",			0x0000	},
{	16,	"OSC2_COEF",		0x7870	},
{	17,	"OSC2X",			0x007d	},
{	18,	"OSC2Y",			0x0000	},
{	19,	"RING_V_OFF",		0x0000	},
{	20,	"RING_OSC",			0x7EF0	},
{	21,	"RING_X",			0x0160	},
{	22,	"RING_Y",			0x0000	},
{	23,	"PULSE_ENVEL",		0x2000	},
{	24,	"PULSE_X",			0x2000	},
{	25,	"PULSE_Y",			0x0000	},
{	26,	"RECV_DIGITAL_GAIN",0x4000	},
{	27,	"XMIT_DIGITAL_GAIN",0x4000	},
{	28,	"LOOP_CLOSE_TRES",	0x1000	},
{	29,	"RING_TRIP_TRES",	0x3600	},
{	30,	"COMMON_MIN_TRES",	0x1000	},
{	31,	"COMMON_MAX_TRES",	0x0200	},
{	32,	"PWR_ALARM_Q1Q2",	0x7c0   },
{	33,	"PWR_ALARM_Q3Q4",	0x2600	},
{	34,	"PWR_ALARM_Q5Q6",	0x1B80	},
{	35,	"LOOP_CLSRE_FlTER",	0x8000	},
{	36,	"RING_TRIP_FILTER",	0x0320	},
{	37,	"TERM_LP_POLE_Q1Q2",0x08c	},
{	38,	"TERM_LP_POLE_Q3Q4",0x0100	},
{	39,	"TERM_LP_POLE_Q5Q6",0x0010	},
{	40,	"CM_BIAS_RINGING",	0x0C00	},
{	41,	"DCDC_MIN_V",		0x0C00	},
{	43,	"LOOP_CLOSE_TRES Low",	0x1000	},
{	99,	"FSK 0 FREQ PARAM",	0x00DA	},
{	100,"FSK 0 AMPL PARAM",	0x6B60	},
{	101,"FSK 1 FREQ PARAM",	0x0074	},
{	102,"FSK 1 AMPl PARAM",	0x79C0	},
{	103,"FSK 0to1 SCALER",	0x1120	},
{	104,"FSK 1to0 SCALER",	0x3BE0	},
{	97,	"RCV_FLTR",			0},
{	0,"",0},
};


void stopTone(void) 
{
  writeDirectReg(32, INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
  writeDirectReg(33, INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation

  writeIndirectReg(13,INIT_IR13);
  writeIndirectReg(14,INIT_IR14);
  writeIndirectReg(16,INIT_IR16);
  writeIndirectReg(17,INIT_IR17);
  writeDirectReg(36,  INIT_DR36);
  writeDirectReg(37,  INIT_DR37);
  writeDirectReg(38,  INIT_DR38);
  writeDirectReg(39,  INIT_DR39);
  writeDirectReg(40,  INIT_DR40);
  writeDirectReg(41,  INIT_DR41);
  writeDirectReg(42,  INIT_DR42);
  writeDirectReg(43,  INIT_DR43);

//  writeDirectReg(32, INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
//  writeDirectReg(33, INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation
}

void genTone(tone_struct tone) 
{ 
	// Uber function to extract values for oscillators from tone_struct
	// place them in the registers, and enable the oscillators.
	unsigned char osc1_ontimer_enable=0, osc1_offtimer_enable=0,osc2_ontimer_enable=0,osc2_offtimer_enable=0;
	int enable_osc2=0;

	//loopBackOff();
	disableOscillators(); // Make sure the oscillators are not already on.

	if (tone.osc1.coeff == 0 || tone.osc1.x == 0) {
		// Error!
		diag_printf("You passed me a invalid struct!\n");
		return;
	}
	// Setup osc 1
	writeIndirectReg( OSC1_COEF, tone.osc1.coeff);
	writeIndirectReg( OSC1X, tone.osc1.x);
	writeIndirectReg( OSC1Y, tone.osc1.y);
	//diag_printf("OUt-> 0x%04x\n",tone.osc1.coeff);
	// Active Timer


	if (tone.osc1.on_hi_byte != 0) {
		writeDirectReg( OSC1_ON__LO, tone.osc1.on_low_byte);
		writeDirectReg( OSC1_ON_HI, tone.osc1.on_hi_byte);
		osc1_ontimer_enable = 0x10;
	}
	// Inactive Timer
	if (tone.osc1.off_hi_byte != 0) {
		writeDirectReg( OSC1_OFF_LO, tone.osc1.off_low_byte);
		writeDirectReg( OSC1_OFF_HI, tone.osc1.off_hi_byte);
		osc1_offtimer_enable = 0x08;
	}
	
	if (tone.osc2.coeff != 0) {
		// Setup OSC 2
		writeIndirectReg( OSC2_COEF, tone.osc2.coeff);
		writeIndirectReg( OSC2X, tone.osc2.x);
		writeIndirectReg( OSC2Y, tone.osc2.y);
		
		// Active Timer
		if (tone.osc1.on_hi_byte != 0) {
			writeDirectReg( OSC2_ON__LO, tone.osc2.on_low_byte);
			writeDirectReg( OSC2_ON_HI, tone.osc2.on_hi_byte);
			osc2_ontimer_enable = 0x10;
		}
		// Inactive Timer
		if (tone.osc1.off_hi_byte != 0) {
			writeDirectReg( OSC2_OFF_LO, tone.osc2.off_low_byte);
			writeDirectReg( OSC2_OFF_HI, tone.osc2.off_hi_byte);
			osc2_offtimer_enable = 0x08;
		}
		enable_osc2 = 1;
	}

	writeDirectReg( OSC1, (unsigned char)(0x06 | osc1_ontimer_enable | osc1_offtimer_enable));
	if (enable_osc2)
		writeDirectReg( OSC2, (unsigned char)(0x06 | osc2_ontimer_enable | osc2_offtimer_enable));
	return;
}


void dialTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_DIALTONE_IR13);
	  writeIndirectReg(14,SI3215_DIALTONE_IR14);
	  writeIndirectReg(16,SI3215_DIALTONE_IR16);
	  writeIndirectReg(17,SI3215_DIALTONE_IR17);
  }
  else
  {
	  writeIndirectReg(13,SI3210_DIALTONE_IR13);
	  writeIndirectReg(14,SI3210_DIALTONE_IR14);
	  writeIndirectReg(16,SI3210_DIALTONE_IR16);
	  writeIndirectReg(17,SI3210_DIALTONE_IR17);
  }

  writeDirectReg(36,  DIALTONE_DR36);
  writeDirectReg(37,  DIALTONE_DR37);
  writeDirectReg(38,  DIALTONE_DR38);
  writeDirectReg(39,  DIALTONE_DR39);
  writeDirectReg(40,  DIALTONE_DR40);
  writeDirectReg(41,  DIALTONE_DR41);
  writeDirectReg(42,  DIALTONE_DR42);
  writeDirectReg(43,  DIALTONE_DR43);

  writeDirectReg(32,  DIALTONE_DR32);
  writeDirectReg(33,  DIALTONE_DR33);
}


void busyTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_BUSYTONE_IR13);
	  writeIndirectReg(14,SI3215_BUSYTONE_IR14);
	  writeIndirectReg(16,SI3215_BUSYTONE_IR16);
	  writeIndirectReg(17,SI3215_BUSYTONE_IR17);
  }
  else
  {
	  writeIndirectReg(13,SI3210_BUSYTONE_IR13);
	  writeIndirectReg(14,SI3210_BUSYTONE_IR14);
	  writeIndirectReg(16,SI3210_BUSYTONE_IR16);
	  writeIndirectReg(17,SI3210_BUSYTONE_IR17);
  }
  writeDirectReg(36,  BUSYTONE_DR36);
  writeDirectReg(37,  BUSYTONE_DR37);
  writeDirectReg(38,  BUSYTONE_DR38);
  writeDirectReg(39,  BUSYTONE_DR39);
  writeDirectReg(40,  BUSYTONE_DR40);
  writeDirectReg(41,  BUSYTONE_DR41);
  writeDirectReg(42,  BUSYTONE_DR42);
  writeDirectReg(43,  BUSYTONE_DR43);
  
  writeDirectReg(32,  BUSYTONE_DR32);
  writeDirectReg(33,  BUSYTONE_DR33);
}

void reorderTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_REORDERTONE_IR13);
	  writeIndirectReg(14,SI3215_REORDERTONE_IR14);
	  writeIndirectReg(16,SI3215_REORDERTONE_IR16);
	  writeIndirectReg(17,SI3215_REORDERTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_REORDERTONE_IR13);
	  writeIndirectReg(14,SI3210_REORDERTONE_IR14);
	  writeIndirectReg(16,SI3210_REORDERTONE_IR16);
	  writeIndirectReg(17,SI3210_REORDERTONE_IR17);
  }

  writeDirectReg(36,  REORDERTONE_DR36);
  writeDirectReg(37,  REORDERTONE_DR37);
  writeDirectReg(38,  REORDERTONE_DR38);
  writeDirectReg(39,  REORDERTONE_DR39);
  writeDirectReg(40,  REORDERTONE_DR40);
  writeDirectReg(41,  REORDERTONE_DR41);
  writeDirectReg(42,  REORDERTONE_DR42);
  writeDirectReg(43,  REORDERTONE_DR43);
  
  writeDirectReg(32,  REORDERTONE_DR32);
  writeDirectReg(33,  REORDERTONE_DR33);
 
}

void congestionTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_CONGESTIONTONE_IR13);
	  writeIndirectReg(14,SI3215_CONGESTIONTONE_IR14);
	  writeIndirectReg(16,SI3215_CONGESTIONTONE_IR16);
	  writeIndirectReg(17,SI3215_CONGESTIONTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_CONGESTIONTONE_IR13);
	  writeIndirectReg(14,SI3210_CONGESTIONTONE_IR14);
	  writeIndirectReg(16,SI3210_CONGESTIONTONE_IR16);
	  writeIndirectReg(17,SI3210_CONGESTIONTONE_IR17);
  }

  writeDirectReg(36,  CONGESTIONTONE_DR36);
  writeDirectReg(37,  CONGESTIONTONE_DR37);
  writeDirectReg(38,  CONGESTIONTONE_DR38);
  writeDirectReg(39,  CONGESTIONTONE_DR39);
  writeDirectReg(40,  CONGESTIONTONE_DR40);
  writeDirectReg(41,  CONGESTIONTONE_DR41);
  writeDirectReg(42,  CONGESTIONTONE_DR42);
  writeDirectReg(43,  CONGESTIONTONE_DR43);

  writeDirectReg(32,  CONGESTIONTONE_DR32);
  writeDirectReg(33,  CONGESTIONTONE_DR33);

}

void ringbackPbxTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_RINGBACKPBXTONE_IR13);
	  writeIndirectReg(14,SI3215_RINGBACKPBXTONE_IR14);
	  writeIndirectReg(16,SI3215_RINGBACKPBXTONE_IR16);
	  writeIndirectReg(17,SI3215_RINGBACKPBXTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_RINGBACKPBXTONE_IR13);
	  writeIndirectReg(14,SI3210_RINGBACKPBXTONE_IR14);
	  writeIndirectReg(16,SI3210_RINGBACKPBXTONE_IR16);
	  writeIndirectReg(17,SI3210_RINGBACKPBXTONE_IR17);
  }

  writeDirectReg(36,  RINGBACKPBXTONE_DR36);
  writeDirectReg(37,  RINGBACKPBXTONE_DR37);
  writeDirectReg(38,  RINGBACKPBXTONE_DR38);
  writeDirectReg(39,  RINGBACKPBXTONE_DR39);
  writeDirectReg(40,  RINGBACKPBXTONE_DR40);
  writeDirectReg(41,  RINGBACKPBXTONE_DR41);
  writeDirectReg(42,  RINGBACKPBXTONE_DR42);
  writeDirectReg(43,  RINGBACKPBXTONE_DR43);

  writeDirectReg(32,  RINGBACKPBXTONE_DR32);
  writeDirectReg(33,  RINGBACKPBXTONE_DR33);

}


void ringBackTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_RINGBACKTONE_IR13);
	  writeIndirectReg(14,SI3215_RINGBACKTONE_IR14);
	  writeIndirectReg(16,SI3215_RINGBACKTONE_IR16);
	  writeIndirectReg(17,SI3215_RINGBACKTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_RINGBACKTONE_IR13);
	  writeIndirectReg(14,SI3210_RINGBACKTONE_IR14);
	  writeIndirectReg(16,SI3210_RINGBACKTONE_IR16);
	  writeIndirectReg(17,SI3210_RINGBACKTONE_IR17);
  }

  writeDirectReg(36,  RINGBACKTONE_DR36);
  writeDirectReg(37,  RINGBACKTONE_DR37);
  writeDirectReg(38,  RINGBACKTONE_DR38);
  writeDirectReg(39,  RINGBACKTONE_DR39);
  writeDirectReg(40,  RINGBACKTONE_DR40);
  writeDirectReg(41,  RINGBACKTONE_DR41);
  writeDirectReg(42,  RINGBACKTONE_DR42);
  writeDirectReg(43,  RINGBACKTONE_DR43);
  
  writeDirectReg(32,  RINGBACKTONE_DR32);
  writeDirectReg(33,  RINGBACKTONE_DR33);
 
}

void ringBackToneSi3216(void)
{
  writeIndirectReg(13,RINGBACKTONE_SI3216_IR13);
  writeIndirectReg(14,RINGBACKTONE_SI3216_IR14);
  writeIndirectReg(16,RINGBACKTONE_SI3216_IR16);
  writeIndirectReg(17,RINGBACKTONE_SI3216_IR17);
  writeDirectReg(36,  RINGBACKTONE_SI3216_DR36);
  writeDirectReg(37,  RINGBACKTONE_SI3216_DR37);
  writeDirectReg(38,  RINGBACKTONE_SI3216_DR38);
  writeDirectReg(39,  RINGBACKTONE_SI3216_DR39);
  writeDirectReg(40,  RINGBACKTONE_SI3216_DR40);
  writeDirectReg(41,  RINGBACKTONE_SI3216_DR41);
  writeDirectReg(42,  RINGBACKTONE_SI3216_DR42);
  writeDirectReg(43,  RINGBACKTONE_SI3216_DR43);
  
  writeDirectReg(32,  RINGBACKTONE_SI3216_DR32);
  writeDirectReg(33,  RINGBACKTONE_SI3216_DR33);
 
}


void ringBackJapanTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_RINGBACKJAPANTONE_IR13);
	  writeIndirectReg(14,SI3215_RINGBACKJAPANTONE_IR14);
	  writeIndirectReg(16,SI3215_RINGBACKJAPANTONE_IR16);
	  writeIndirectReg(17,SI3215_RINGBACKJAPANTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_RINGBACKJAPANTONE_IR13);
	  writeIndirectReg(14,SI3210_RINGBACKJAPANTONE_IR14);
	  writeIndirectReg(16,SI3210_RINGBACKJAPANTONE_IR16);
	  writeIndirectReg(17,SI3210_RINGBACKJAPANTONE_IR17);
  }

  writeDirectReg(36,  RINGBACKJAPANTONE_DR36);
  writeDirectReg(37,  RINGBACKJAPANTONE_DR37);
  writeDirectReg(38,  RINGBACKJAPANTONE_DR38);
  writeDirectReg(39,  RINGBACKJAPANTONE_DR39);
  writeDirectReg(40,  RINGBACKJAPANTONE_DR40);
  writeDirectReg(41,  RINGBACKJAPANTONE_DR41);
  writeDirectReg(42,  RINGBACKJAPANTONE_DR42);
  writeDirectReg(43,  RINGBACKJAPANTONE_DR43);
  
  writeDirectReg(32,  RINGBACKJAPANTONE_DR32);
  writeDirectReg(33,  RINGBACKJAPANTONE_DR33);
 
}


void busyJapanTone(void)
{
  if (Si3215_Flag)
  {
	  writeIndirectReg(13,SI3215_BUSYJAPANTONE_IR13);
	  writeIndirectReg(14,SI3215_BUSYJAPANTONE_IR14);
	  writeIndirectReg(16,SI3215_BUSYJAPANTONE_IR16);
	  writeIndirectReg(17,SI3215_BUSYJAPANTONE_IR17);
  }
  else {
	  writeIndirectReg(13,SI3210_BUSYJAPANTONE_IR13);
	  writeIndirectReg(14,SI3210_BUSYJAPANTONE_IR14);
	  writeIndirectReg(16,SI3210_BUSYJAPANTONE_IR16);
	  writeIndirectReg(17,SI3210_BUSYJAPANTONE_IR17);
  }

  writeDirectReg(36,  BUSYJAPANTONE_DR36);
  writeDirectReg(37,  BUSYJAPANTONE_DR37);
  writeDirectReg(38,  BUSYJAPANTONE_DR38);
  writeDirectReg(39,  BUSYJAPANTONE_DR39);
  writeDirectReg(40,  BUSYJAPANTONE_DR40);
  writeDirectReg(41,  BUSYJAPANTONE_DR41);
  writeDirectReg(42,  BUSYJAPANTONE_DR42);
  writeDirectReg(43,  BUSYJAPANTONE_DR43);
  
  writeDirectReg(32,  BUSYJAPANTONE_DR32);
  writeDirectReg(33,  BUSYJAPANTONE_DR33);
 
}

void standardRinging(void) { 	
	// Enables ringing mode on ProSlic for standard North American ring
	//	RING_ON__LO	48
	//	RING_ON_HI	49
	//	RING_OFF_LO	50
	//	RING_OFF_HI	51
	// Active Timer

	writeDirectReg( RING_ON__LO, 0x80); // low reg 48
	writeDirectReg( RING_ON_HI, 0x3E); // hi reg 49
	// Inactive Timer
	writeDirectReg( RING_OFF_LO, 0x00); // low reg 50
	writeDirectReg( RING_OFF_HI, 0x7D); // hi reg 51
	// Enable timers for ringing oscillator
	writeDirectReg( 34, 0x18);

}

unsigned char powerUp(void)
{ 
	unsigned char vBat ; 
	int i=0;


	if (chipType() == 3)  // M version correction
	{
		writeDirectReg(92,INIT_SI3210M_DR92);// M version
		writeDirectReg(93,INIT_SI3210M_DR93);// M version
	}
	else	
	{
		/* MA */
		writeDirectReg(93, 12); 
		writeDirectReg(92, 0xff); /* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW*/

	}

	writeDirectReg(14, 0); /* Engage the DC-DC converter */
  
	while ((vBat=readDirectReg(82)) < 0xc0)
	{ 
		Slic_os_delay (1000);
		++i;
		if (i > 200) return 0;
	}
  	
	if (chipType() == 3)  // M version correction
	{
		writeDirectReg(92,0x80 +INIT_SI3210M_DR92);// M version
	}
	else
		writeDirectReg(93, 0x8c);  /* DC-DC Calibration  */ /* MA */

	while(0x80 & readDirectReg(93));  // Wait for DC-DC Calibration to complete

	return vBat;
}
 
unsigned char powerLeakTest(void)
{ 
	unsigned char vBat ;
	
	writeDirectReg(64,0);

	writeDirectReg(14, 0x10); 
	Slic_os_delay (1000);

	if (vBat=readDirectReg(82) < 0x4)  // 6 volts
	 return 0;

	return vBat;
}


void xcalibrate(void)
{ unsigned char i ;

	writeDirectReg(97, 0x1e); /* Do all of the Calibarations */
	writeDirectReg(96, 0x47); /* Start the calibration No Speed Up */
	
	manualCalibrate();

	writeDirectReg(23,4);  // enable interrupt for the balance Cal
	writeDirectReg(97,0x1);
	writeDirectReg(96,0x40);


	while (readDirectReg(96) != 0 );
	diag_printf("\nCalibration Vector Registers 98 - 107: ");
	
	for (i=98; i < 107; i++)  diag_printf( "%X.", readDirectReg(i));
	diag_printf("%X \n\n",readDirectReg(107));

}

void goActive(void)

{
	writeDirectReg(64,1);	/* LOOP STATE REGISTER SET TO ACTIVE */
				/* Active works for on-hook and off-hook see spec. */
				/* The phone hook-switch sets the off-hook and on-hook substate*/
}


unsigned char version(void)
{
	return 0xf & readDirectReg(0); 
}

unsigned char chipType (void)
{
	return (0x30 & readDirectReg(0)) >> 4; 
}

unsigned char family (void)
{
        return (readDirectReg (1) & 0x80);
}

int selfTest(void)
{
	/*  Begin Sanity check  Optional */
	if (readDirectReg(8) !=2) {exception(PROSLICiNSANE); return 0;}
	if (readDirectReg(64) !=0) {exception(PROSLICiNSANE); return 0;}
	if (readDirectReg(11) !=0x33) {exception(PROSLICiNSANE); return 0;}
	/* End Sanity check */
	return 1;
}

int slicStart(void)
{
	volatile unsigned char t,v;
	volatile unsigned short i;

	// Test if Si3210 or Si3215 is used.
	Si3215_Flag = TestForSi3215();

	if (Si3215_Flag)
		diag_printf ("Proslic Si3215 Detected.\n");
	else
		diag_printf ("Proslic Si3210 detected.\n");

	/*  Another Quarter of a Second */
	if (!selfTest())
		return 0;

	v = version();
	diag_printf("Si321x Vesion = %x\n", v);

	t = chipType();
	diag_printf("chip Type t=%d\n", t);

	initializeIndirectRegisters();

	i=verifyIndirectRegisters ();
	if (i != 0) {
		diag_printf ("verifyIndirect failed\n");
		return 0;
	}

	writeDirectReg (8, 0);
	if (v == 5)
		writeDirectReg (108, 0xeb); /* turn on Rev E features. */

	
	if (   t == 0 ) // Si3210 not the Si3211 or Si3212	
	{
		writeDirectReg(67,0x17); // Make VBat switch not automatic 
		// The above is a saftey measure to prevent Q7 from accidentaly turning on and burning out.
		//  It works in combination with the statement below.  Pin 34 DCDRV which is used for the battery switch on the
		//  Si3211 & Si3212 
	
		writeDirectReg(66,1);  //    Q7 should be set to OFF for si3210
	}

	if (v <=2 ) {  //  REVISION B   
		writeDirectReg(73,4);  // set common mode voltage to 6 volts
	}

	// PCm Start Count. 
	writeDirectReg (2, 0);
	writeDirectReg (4, 0);


	/* Do Flush durring powerUp and calibrate */
	if (t == 0 || t==3) //  Si3210
	{
		diag_printf("Powerup the Slic\n");

		// Turn on the DC-DC converter and verify voltage.
		if (!powerUp())
			return 0;
	}

	initializeDirectRegisters();

	if (!calibrate()) {
		diag_printf("calibrate failed\n");
		return 0;
	}

	diag_printf ("Register 98 = %x\n", readDirectReg(98));
	diag_printf ("Register 99 = %x\n", readDirectReg(99));

	clearInterrupts();

	diag_printf("Slic is Active\n");
	goActive();

//#define TORTURE_INTERFACE_TEST
#ifdef TORTURE_INTERFACE_TEST
#define SPI_ITERATIONS 10000
{
	int j, i=0;
	volatile unsigned char WriteIn, ReadBack;
	diag_printf("SPI test Write/Read %d times...\n", SPI_ITERATIONS);
	for (j=0,i=0;j<SPI_ITERATIONS;j++,i++) {
		i &= 0xff;
//		diag_printf("Writing %d\n",i);
		writeDirectReg (2, i);
		Slic_os_delay( 1 );
		ReadBack = readDirectReg(2);
		if (ReadBack != i) {
			diag_printf("Wrote %x, ReadBack = %x \n", i, ReadBack);
			break;
		}
/*		
		i++;
		i = i & 0xff;
*/
	}
	if(j == SPI_ITERATIONS)
		diag_printf("SPI test ok\n");
	else
		diag_printf("SPI test failed\n");
}	
#endif


#ifdef SLIC_TEST_LOOPSTATUS_AND_DIGIT_DETECTION
{
	volatile unsigned char WriteIn, ReadBack;
	while (1) {
		volatile int HookStatus;
		volatile short ReadBack, CurrentReadBack;
		if (loopStatus () != HookStatus) {
			HookStatus = loopStatus();
			diag_printf ("HOOK Status = %x\n", HookStatus);
		}
		ReadBack = readDirectReg(64);
		if (ReadBack != CurrentReadBack) {
			diag_printf ("ReadBack = %x\n", ReadBack);
			CurrentReadBack = ReadBack;
		}

		/*
		** Check digit detection.
		*/
		digit_decode_status = digit() ;
		if (digit_decode_status & DTMF_DETECT) {

			/* compare with the last detect status. */
			if ((digit_decode_status & DTMF_DIGIT_MASK) != FXS_Signaling_current_digit) {

				diag_printf("Digit %x detected.\n", digit_decode_status & 0xF);

				// New digit detected.
				FXS_Signaling_current_digit =  (digit_decode_status & DTMF_DIGIT_MASK);
			}
		}
		else
			FXS_Signaling_current_digit = DTMF_NONE;
	}
}
#endif

	return 1;

}


void stopRinging(void)
{
	
	if ((0xf & readDirectReg(0))<=2 )  // if REVISION B  
        writeDirectReg(69,10);   // Loop Debounce Register  = initial value
    
	goActive();
	
}

unsigned short manualCalibrate(void)
{ 
unsigned char x,y,i,progress=0; // progress contains individual bits for the Tip and Ring Calibrations

	//Initialized DR 98 and 99 to get consistant results.
	// 98 and 99 are the results registers and the search should have same intial conditions.
	writeDirectReg(98,0x10); // This is necessary if the calibration occurs other than at reset time
	writeDirectReg(99,0x10);

	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(98,i);
		Slic_os_delay(200);
		if((readDirectReg(88)) == 0)
		{	progress|=1;
		x=i;
		break;
		}
	} // for



	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(99,i);
		Slic_os_delay(200);
		if((readDirectReg(89)) == 0){
			progress|=2;
			y=i;
		break;
		}
	
	}//for

	return progress;
}


void cadenceRingPhone(ringStruct ringRegs) { 
	
	// Enables ringing mode on ProSlic for standard North American ring
	//	RING_ON__LO	48
	//	RING_ON_HI	49
	//	RING_OFF_HI	51
	//	RING_OFF_LO	50


	diag_printf ("\n on= %d ms off =%d ms", ringRegs.u1.onTime/8, ringRegs.u2.offTime/8);
	writeDirectReg( RING_ON__LO,  ringRegs.u1.s1.onLowByte); // lo reg 48
	writeDirectReg( RING_ON_HI,   ringRegs.u1.s1.onHiByte); // hi reg 49
	// Inactive Timer
	writeDirectReg( RING_OFF_LO, ringRegs.u2.s2.offLowByte); // low reg 50
	writeDirectReg( RING_OFF_HI, ringRegs.u2.s2.offHiByte); // hi reg 51
	// Enable timers for ringing oscillator
		writeDirectReg( 34, 0x18);
	}


int groundShort(void)
{ 
	int rc;

	rc= ( (readDirectReg(80) < 2) || readDirectReg(81) < 2);
		
		if (rc) {
			exception(TIPoRrINGgROUNDsHORT);
			return -1;
		}
		return rc;
}

void clearAlarmBits(void)
{
	writeDirectReg(19,0xFC); //Clear Alarm bits
}


unsigned long interruptBits (void) {
	// Determines which interrupt bit is set
//	int  count = 1;
//	unsigned char j = 0x01 ;
	union {
		unsigned char reg_data[3];
		long interrupt_bits;
	} u ;
	u.interrupt_bits=0;
	
// ONLY CLEAR the ACTIVE INTERRUPT or YOU WILL CREATE CRITICAL SECTION ERROR of LEAVING
// THE TIME OPEN BETWEEN THE Nth BIT and the N+1thbit within the same BYTE.
// eg. if the inactive oscillators are firing at nearly the same time
// you would only see one.


	u.reg_data[0] = readDirectReg( 18);
	writeDirectReg( 18,u.reg_data[0]);

	u.reg_data[1] = readDirectReg( 19);
	if (u.reg_data[1] & 0xfc != 0) {
		diag_printf ("Power alarm = %x\n",(u.reg_data[1] & 0xfc));
		clearAlarmBits ();
	} 
	writeDirectReg( 19,u.reg_data[1] );

	u.reg_data[2] = readDirectReg( 20);
	writeDirectReg( 20,u.reg_data[2]);


	return u.interrupt_bits ;
}


void activateRinging(void)
{
	writeDirectReg( LINE_STATE, RING_LINE); // REG 64,4
}


void converterOn(void){
	writeDirectReg(14,	0);
}


void disableOscillators(void) 
{ 
	// Turns of OSC1 and OSC2
	unsigned char i;

	//diag_printf("Disabling Oscillators!!!\n");
	for ( i=32; i<=45; i++) 
		if (i !=34)  // Don't write to the ringing oscillator control
		writeDirectReg(i,0);
}

void initializeLoopDebounceReg(void)
{
	writeDirectReg(69,10);   // Loop Debounce Register  = initial value
}

void printLoopState(void)
{

diag_printf(readDirectReg(68)&4?"On hook":"Off hook");
}


unsigned char loopStatus(void)
{
static int ReadBack;

	// check for power alarms
	if ((readDirectReg(19) & 0xfc) != 0) {
		diag_printf ("Power alarm = %x\n",(readDirectReg(19) & 0xfc));
		clearAlarmBits ();
	} 
	ReadBack = readDirectReg (68);
	return (ReadBack & 0x3);

}

unsigned char digit()
{
	return readDirectReg(24) & 0x1f;
}


void printFreq_Revision(void)
{

	int freq;
	char* freqs[ ] = {"8192","4028","2048","1024","512","256","1536","768","32768"};

	// Turn on all interrupts
	freq=readDirectReg(13)>>4;  /* Read the frequency */
	diag_printf("PCM clock =  %s KHz   Rev %c \n",  freqs[freq], 'A'-1 + version()); 
}

int calibrate()
{ 
	unsigned short i=0;
	volatile unsigned char   DRvalue;
	int timeOut,nCalComplete;

	/* Do Flush durring powerUp and calibrate */
	writeDirectReg(21,DISABLE_ALL_DR21);//(0)  Disable all interupts in DR21
	writeDirectReg(22,DISABLE_ALL_DR22);//(0)	Disable all interupts in DR21
	writeDirectReg(23,DISABLE_ALL_DR23);//(0)	Disabel all interupts in DR21
	writeDirectReg(64,OPEN_DR64);//(0)
  
	writeDirectReg(97,0x1e); //(0x18)Calibrations without the ADC and DAC offset and without common mode calibration.
	writeDirectReg(96,0x47); //(0x47)	Calibrate common mode and differential DAC mode DAC + ILIM
	
	diag_printf("Calibration vBat = %x\n ", readDirectReg(82));
 
	i=0;
	do 
	{
		DRvalue = readDirectReg(96);
		nCalComplete = DRvalue==CAL_COMPLETE_DR96;// (0)  When Calibration completes DR 96 will be zero
		timeOut= i++>MAX_CAL_PERIOD;
		Slic_os_delay(1);
	}
	while (!nCalComplete&&!timeOut);

	if (timeOut) {
	    diag_printf ("Error in Caliberatation: timeOut\n");
	    return 0;
	}        

	if (!Si3215_Flag)
	{
		manualCalibrate ();
	}

    
/*Initialized DR 98 and 99 to get consistant results.*/
/* 98 and 99 are the results registers and the search should have same intial conditions.*/
/*******The following is the manual gain mismatch calibration******/
/*******This is also available as a function **********************/
	// Wait for any power alarms to settle. 
	Slic_os_delay(1000);

	writeIndirectReg(88,0);
	writeIndirectReg(89,0);
	writeIndirectReg(90,0);
	writeIndirectReg(91,0);
	writeIndirectReg(92,0);
	writeIndirectReg(93,0);


	goActive();

	if  (loopStatus() & 4) {
		diag_printf ("Error in Caliberate:  ERRORCODE_LONGBALCAL\n");
		return 0 ;
	}

	writeDirectReg(64,OPEN_DR64);

	writeDirectReg(23,ENB2_DR23);  // enable interrupt for the balance Cal
	writeDirectReg(97,BIT_CALCM_DR97); // this is a singular calibration bit for longitudinal calibration
	writeDirectReg(96,0x40);

	DRvalue = readDirectReg(96);
	i=0;
	do 
	{
       	DRvalue = readDirectReg(96);
        nCalComplete = DRvalue==CAL_COMPLETE_DR96;// (0)  When Calibration completes DR 96 will be zero
		timeOut= i++>MAX_CAL_PERIOD;// (800) MS
		Slic_os_delay(1);
	}
	while (!nCalComplete&&!timeOut);
	  
	if (timeOut) {
	    diag_printf ("Error in Caliberate:  timeOut\n");
	    return 0;
	}

	Slic_os_delay(1000);

	for (i=88; i<=95; i++) {
		writeIndirectReg (i, 0);
	}
	writeIndirectReg (97, 0);

	for (i=193; i<=211; i++) {
		writeIndirectReg (i, 0);
	}
    		
   	writeDirectReg(21,INIT_DR21);
	writeDirectReg(22,INIT_DR22);
	writeDirectReg(23,INIT_DR23);

/**********************************The preceding is the longitudinal Balance Cal***********************************/


	diag_printf ("Caliberation done\n");
	return(1);

}// End of calibration

void initializeIndirectRegisters()										
{										
if (!Si3215_Flag)
{
	writeIndirectReg(	0	,	INIT_IR0		);	//	0x55C2	DTMF_ROW_0_PEAK
	writeIndirectReg(	1	,	INIT_IR1		);	//	0x51E6	DTMF_ROW_1_PEAK
	writeIndirectReg(	2	,	INIT_IR2		);	//	0x4B85	DTMF_ROW2_PEAK
	writeIndirectReg(	3	,	INIT_IR3		);	//	0x4937	DTMF_ROW3_PEAK
	writeIndirectReg(	4	,	INIT_IR4		);	//	0x3333	DTMF_COL1_PEAK
	writeIndirectReg(	5	,	INIT_IR5		);	//	0x0202	DTMF_FWD_TWIST
	writeIndirectReg(	6	,	INIT_IR6		);	//	0x0202	DTMF_RVS_TWIST
	writeIndirectReg(	7	,	INIT_IR7		);	//	0x0198	DTMF_ROW_RATIO
	writeIndirectReg(	8	,	INIT_IR8		);	//	0x0198	DTMF_COL_RATIO
	writeIndirectReg(	9	,	INIT_IR9		);	//	0x0611	DTMF_ROW_2ND_ARM
	writeIndirectReg(	10	,	INIT_IR10		);	//	0x0202	DTMF_COL_2ND_ARM
	writeIndirectReg(	11	,	INIT_IR11		);	//	0x00E5	DTMF_PWR_MIN_
	writeIndirectReg(	12	,	INIT_IR12		);	//	0x0A1C	DTMF_OT_LIM_TRES
}

	writeIndirectReg(	13	,	INIT_IR13		);	//	0x7b30	OSC1_COEF
	writeIndirectReg(	14	,	INIT_IR14		);	//	0x0063	OSC1X
	writeIndirectReg(	15	,	INIT_IR15		);	//	0x0000	OSC1Y
	writeIndirectReg(	16	,	INIT_IR16		);	//	0x7870	OSC2_COEF
	writeIndirectReg(	17	,	INIT_IR17		);	//	0x007d	OSC2X
	writeIndirectReg(	18	,	INIT_IR18		);	//	0x0000	OSC2Y
	writeIndirectReg(	19	,	INIT_IR19		);	//	0x0000	RING_V_OFF
	writeIndirectReg(	20	,	INIT_IR20		);	//	0x7EF0	RING_OSC
	writeIndirectReg(	21	,	INIT_IR21		);	//	0x0160	RING_X
	writeIndirectReg(	22	,	INIT_IR22		);	//	0x0000	RING_Y
	writeIndirectReg(	23	,	INIT_IR23		);	//	0x2000	PULSE_ENVEL
	writeIndirectReg(	24	,	INIT_IR24		);	//	0x2000	PULSE_X
	writeIndirectReg(	25	,	INIT_IR25		);	//	0x0000	PULSE_Y
	writeIndirectReg(	26	,	INIT_IR26		);	//	0x4000	RECV_DIGITAL_GAIN
	writeIndirectReg(	27	,	INIT_IR27		);	//	0x4000	XMIT_DIGITAL_GAIN
	writeIndirectReg(	28	,	INIT_IR28		);	//	0x1000	LOOP_CLOSE_TRES
	writeIndirectReg(	29	,	INIT_IR29		);	//	0x3600	RING_TRIP_TRES
	writeIndirectReg(	30	,	INIT_IR30		);	//	0x1000	COMMON_MIN_TRES
	writeIndirectReg(	31	,	INIT_IR31		);	//	0x0200	COMMON_MAX_TRES
	writeIndirectReg(	32	,	INIT_IR32		);	//	0x7c0  	PWR_ALARM_Q1Q2
	writeIndirectReg(	33	,	INIT_IR33		);	//	0x2600	PWR_ALARM_Q3Q4
	writeIndirectReg(	34	,	INIT_IR34		);	//	0x1B80	PWR_ALARM_Q5Q6
	writeIndirectReg(	35	,	INIT_IR35		);	//	0x8000	LOOP_CLSRE_FlTER
	writeIndirectReg(	36	,	INIT_IR36		);	//	0x0320	RING_TRIP_FILTER
	writeIndirectReg(	37	,	INIT_IR37		);	//	0x08c	TERM_LP_POLE_Q1Q2
	writeIndirectReg(	38	,	INIT_IR38		);	//	0x0100	TERM_LP_POLE_Q3Q4
	writeIndirectReg(	39	,	INIT_IR39		);	//	0x0010	TERM_LP_POLE_Q5Q6
	writeIndirectReg(	40	,	INIT_IR40		);	//	0x0C00	CM_BIAS_RINGING
	writeIndirectReg(	41	,	INIT_IR41		);	//	0x0C00	DCDC_MIN_V
	writeIndirectReg(	43	,	INIT_IR43		);	//	0x1000	LOOP_CLOSE_TRES Low
	writeIndirectReg(	99	,	INIT_IR99		);	//	0x00DA	FSK 0 FREQ PARAM
	writeIndirectReg(	100	,	INIT_IR100		);	//	0x6B60	FSK 0 AMPL PARAM
	writeIndirectReg(	101	,	INIT_IR101		);	//	0x0074	FSK 1 FREQ PARAM
	writeIndirectReg(	102	,	INIT_IR102		);	//	0x79C0	FSK 1 AMPl PARAM
	writeIndirectReg(	103	,	INIT_IR103		);	//	0x1120	FSK 0to1 SCALER
	writeIndirectReg(	104	,	INIT_IR104		);	//	0x3BE0	FSK 1to0 SCALER
	writeIndirectReg(	97	,	INIT_IR97		);	//	0x0000	TRASMIT_FILTER
}										

	

void initializeDirectRegisters(void)
{

writeDirectReg(0,	INIT_DR0	);//0X00	Serial Interface
writeDirectReg(1,	INIT_DR1	);//0X08	PCM Mode - INIT TO disable
writeDirectReg(2,	INIT_DR2	);//0X00	PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB)
writeDirectReg(3,	INIT_DR3	);//0x00	PCM TX Clock Slot High Byte
writeDirectReg(4,	INIT_DR4	);//0x00	PCM RX Clock Slot Low Byte (1 PCLK cycle/LSB)
writeDirectReg(5,	INIT_DR5	);//0x00	PCM RX Clock Slot High Byte
writeDirectReg(8,	INIT_DR8	);//0X00	Loopbacks (digital loopback default)
writeDirectReg(9,	INIT_DR9	);//0x00	Transmit and receive path gain and control
writeDirectReg(10,	INIT_DR10	);//0X28	Initialization Two-wire impedance (600  and enabled)
writeDirectReg(11,	INIT_DR11	);//0x33	Transhybrid Balance/Four-wire Return Loss
writeDirectReg(18,	INIT_DR18	);//0xff	Normal Oper. Interrupt Register 1 (clear with 0xFF)
writeDirectReg(19,	INIT_DR19	);//0xff	Normal Oper. Interrupt Register 2 (clear with 0xFF)
writeDirectReg(20,	INIT_DR20	);//0xff	Normal Oper. Interrupt Register 3 (clear with 0xFF)
writeDirectReg(21,	INIT_DR21	);//0xff	Interrupt Mask 1
writeDirectReg(22,	INIT_DR22	);//0xff	Initialization Interrupt Mask 2
writeDirectReg(23,	INIT_DR23	);//0xff	 Initialization Interrupt Mask 3
writeDirectReg(32,	INIT_DR32	);//0x00	Oper. Oscillator 1 Controltone generation
writeDirectReg(33,	INIT_DR33	);//0x00	Oper. Oscillator 2 Controltone generation
writeDirectReg(34,	INIT_DR34	);//0X18	34 0x22 0x00 Initialization Ringing Oscillator Control
writeDirectReg(35,	INIT_DR35	);//0x00	Oper. Pulse Metering Oscillator Control
writeDirectReg(36,	INIT_DR36	);//0x00	36 0x24 0x00 Initialization OSC1 Active Low Byte (125 탎/LSB)
writeDirectReg(37,	INIT_DR37	);//0x00	37 0x25 0x00 Initialization OSC1 Active High Byte (125 탎/LSB)
writeDirectReg(38,	INIT_DR38	);//0x00	38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 탎/LSB)
writeDirectReg(39,	INIT_DR39	);//0x00	39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 탎/LSB)
writeDirectReg(40,	INIT_DR40	);//0x00	40 0x28 0x00 Initialization OSC2 Active Low Byte (125 탎/LSB)
writeDirectReg(41,	INIT_DR41	);//0x00	41 0x29 0x00 Initialization OSC2 Active High Byte (125 탎/LSB)
writeDirectReg(42,	INIT_DR42	);//0x00	42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 탎/LSB)
writeDirectReg(43,	INIT_DR43	);//0x00	43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 탎/LSB)
writeDirectReg(44,	INIT_DR44	);//0x00	44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 탎/LSB)
writeDirectReg(45,	INIT_DR45	);//0x00	45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 탎/LSB)
writeDirectReg(46,	INIT_DR46	);//0x00	46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 탎/LSB)
writeDirectReg(47,	INIT_DR47	);//0x00	47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 탎/LSB)
writeDirectReg(48,	INIT_DR48	);//0X80	48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 탎/LSB)
writeDirectReg(49,	INIT_DR49	);//0X3E	49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 탎/LSB)
writeDirectReg(50,	INIT_DR50	);//0X00	50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 탎/LSB)
writeDirectReg(51,	INIT_DR51	);//0X7D	51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 탎/LSB)
writeDirectReg(52,	INIT_DR52	);//0X00	52 0x34 0x00 Normal Oper. FSK Data Bit
writeDirectReg(63,	INIT_DR63	);//0X54	63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval
writeDirectReg(64,	INIT_DR64	);//0x00	64 0x40 0x00 Normal Oper. Mode Byte뾭rimary control
writeDirectReg(65,	INIT_DR65	);//0X61	65 0x41 0x61 Initialization External Bipolar Transistor Settings
writeDirectReg(66,	INIT_DR66	);//0X03	66 0x42 0x03 Initialization Battery Control
writeDirectReg(67,	INIT_DR67	);//0X1F	67 0x43 0x1F Initialization Automatic/Manual Control
writeDirectReg(69,	INIT_DR69	);//0X0C	69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB)
writeDirectReg(70,	INIT_DR70	);//0X0A	70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB)
writeDirectReg(71,	INIT_DR71	);//0X01	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB)
writeDirectReg(72,	INIT_DR72	);//0X20	72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB)
writeDirectReg(73,	INIT_DR73	);//0X02	73 0x49 0x02 Initialization Common Mode VoltageVCM = 3 V(1.5 V/LSB)
writeDirectReg(74,	INIT_DR74	);//0X32	74 0x4A 0x32 Initialization VBATH (ringing) = 75 V (1.5 V/LSB)
writeDirectReg(75,	INIT_DR75	);//0X10	75 0x4B 0x10 Initialization VBATL (off-hook) = 24 V (TRACK = 0)(1.5 V/LSB)
if (chipType() != 3)
writeDirectReg(92,	INIT_DR92	);//0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB)
else
writeDirectReg(92,	INIT_SI3210M_DR92	);//0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB)


writeDirectReg(93,	INIT_DR93	);//0x14	93 0x5D 0x14 0x19 Initialization DCDC Converter Min. Off Time (61.035 ns/LSB)
writeDirectReg(96,	INIT_DR96	);//0x00	96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration)
writeDirectReg(97,	INIT_DR97	);//0X1F	97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96)
writeDirectReg(98,	INIT_DR98	);//0X10	98 0x62 0x10 Informative Calibration result (see data sheet)
writeDirectReg(99,	INIT_DR99	);//0X10	99 0x63 0x10 Informative Calibration result (see data sheet)
writeDirectReg(100,	INIT_DR100	);//0X11	100 0x64 0x11 Informative Calibration result (see data sheet)
writeDirectReg(101,	INIT_DR101	);//0X11	101 0x65 0x11 Informative Calibration result (see data sheet)
writeDirectReg(102,	INIT_DR102	);//0x08	102 0x66 0x08 Informative Calibration result (see data sheet)
writeDirectReg(103,	INIT_DR103	);//0x88	103 0x67 0x88 Informative Calibration result (see data sheet)
writeDirectReg(104,	INIT_DR104	);//0x00	104 0x68 0x00 Informative Calibration result (see data sheet)
writeDirectReg(105,	INIT_DR105	);//0x00	105 0x69 0x00 Informative Calibration result (see data sheet)
writeDirectReg(106,	INIT_DR106	);//0x20	106 0x6A 0x20 Informative Calibration result (see data sheet)
writeDirectReg(107,	INIT_DR107	);//0x08	107 0x6B 0x08 Informative Calibration result (see data sheet)
writeDirectReg(108,	INIT_DR108	);//0xEB	108 0x63 0x00 0xEB Initialization Feature enhancement register
}
   

void clearInterrupts(void)
{
	writeDirectReg(	18	,	INIT_DR18	);//0xff	Normal Oper. Interrupt Register 1 (clear with 0xFF)
	writeDirectReg(	19	,	INIT_DR19	);//0xff	Normal Oper. Interrupt Register 2 (clear with 0xFF)
	writeDirectReg(	20	,	INIT_DR20	);//0xff	Normal Oper. Interrupt Register 3 (clear with 0xFF)
}

int verifyIndirectReg(unsigned char address, unsigned short should_be_value)
{ 
	int error_flag ;
	unsigned short value;
		value = readIndirectReg(address);
		error_flag = (should_be_value != value);
		
		if ( error_flag )
		{
			diag_printf("\n   iREG %d = %X  should be %X ",address,value,should_be_value );			
		}	
		return error_flag;
}


int verifyIndirectRegisters()										
{		
	int error=0;

	if (!Si3215_Flag)
	{

	error |= verifyIndirectReg(	0	,	INIT_IR0		);	//	0x55C2	DTMF_ROW_0_PEAK
	error |= verifyIndirectReg(	1	,	INIT_IR1		);	//	0x51E6	DTMF_ROW_1_PEAK
	error |= verifyIndirectReg(	2	,	INIT_IR2		);	//	0x4B85	DTMF_ROW2_PEAK
	error |= verifyIndirectReg(	3	,	INIT_IR3		);	//	0x4937	DTMF_ROW3_PEAK
	error |= verifyIndirectReg(	4	,	INIT_IR4		);	//	0x3333	DTMF_COL1_PEAK
	error |= verifyIndirectReg(	5	,	INIT_IR5		);	//	0x0202	DTMF_FWD_TWIST
	error |= verifyIndirectReg(	6	,	INIT_IR6		);	//	0x0202	DTMF_RVS_TWIST
	error |= verifyIndirectReg(	7	,	INIT_IR7		);	//	0x0198	DTMF_ROW_RATIO
	error |= verifyIndirectReg(	8	,	INIT_IR8		);	//	0x0198	DTMF_COL_RATIO
	error |= verifyIndirectReg(	9	,	INIT_IR9		);	//	0x0611	DTMF_ROW_2ND_ARM
	error |= verifyIndirectReg(	10	,	INIT_IR10		);	//	0x0202	DTMF_COL_2ND_ARM
	error |= verifyIndirectReg(	11	,	INIT_IR11		);	//	0x00E5	DTMF_PWR_MIN_
	error |= verifyIndirectReg(	12	,	INIT_IR12		);	//	0x0A1C	DTMF_OT_LIM_TRES
	}
	error |= verifyIndirectReg(	13	,	INIT_IR13		);	//	0x7b30	OSC1_COEF
	error |= verifyIndirectReg(	14	,	INIT_IR14		);	//	0x0063	OSC1X
	error |= verifyIndirectReg(	15	,	INIT_IR15		);	//	0x0000	OSC1Y
	error |= verifyIndirectReg(	16	,	INIT_IR16		);	//	0x7870	OSC2_COEF
	error |= verifyIndirectReg(	17	,	INIT_IR17		);	//	0x007d	OSC2X
	error |= verifyIndirectReg(	18	,	INIT_IR18		);	//	0x0000	OSC2Y
	error |= verifyIndirectReg(	19	,	INIT_IR19		);	//	0x0000	RING_V_OFF
	error |= verifyIndirectReg(	20	,	INIT_IR20		);	//	0x7EF0	RING_OSC
	error |= verifyIndirectReg(	21	,	INIT_IR21		);	//	0x0160	RING_X
	error |= verifyIndirectReg(	22	,	INIT_IR22		);	//	0x0000	RING_Y
	error |= verifyIndirectReg(	23	,	INIT_IR23		);	//	0x2000	PULSE_ENVEL
	error |= verifyIndirectReg(	24	,	INIT_IR24		);	//	0x2000	PULSE_X
	error |= verifyIndirectReg(	25	,	INIT_IR25		);	//	0x0000	PULSE_Y
	error |= verifyIndirectReg(	26	,	INIT_IR26		);	//	0x4000	RECV_DIGITAL_GAIN
	error |= verifyIndirectReg(	27	,	INIT_IR27		);	//	0x4000	XMIT_DIGITAL_GAIN
	error |= verifyIndirectReg(	28	,	INIT_IR28		);	//	0x1000	LOOP_CLOSE_TRES
	error |= verifyIndirectReg(	29	,	INIT_IR29		);	//	0x3600	RING_TRIP_TRES
	error |= verifyIndirectReg(	30	,	INIT_IR30		);	//	0x1000	COMMON_MIN_TRES
	error |= verifyIndirectReg(	31	,	INIT_IR31		);	//	0x0200	COMMON_MAX_TRES
	error |= verifyIndirectReg(	32	,	INIT_IR32		);	//	0x7c0  	PWR_ALARM_Q1Q2
	error |= verifyIndirectReg(	33	,	INIT_IR33		);	//	0x2600	PWR_ALARM_Q3Q4
	error |= verifyIndirectReg(	34	,	INIT_IR34		);	//	0x1B80	PWR_ALARM_Q5Q6
	error |= verifyIndirectReg(	35	,	INIT_IR35		);	//	0x8000	LOOP_CLSRE_FlTER
	error |= verifyIndirectReg(	36	,	INIT_IR36		);	//	0x0320	RING_TRIP_FILTER
	error |= verifyIndirectReg(	37	,	INIT_IR37		);	//	0x08c	TERM_LP_POLE_Q1Q2
	error |= verifyIndirectReg(	38	,	INIT_IR38		);	//	0x0100	TERM_LP_POLE_Q3Q4
	error |= verifyIndirectReg(	39	,	INIT_IR39		);	//	0x0010	TERM_LP_POLE_Q5Q6
	error |= verifyIndirectReg(	40	,	INIT_IR40		);	//	0x0C00	CM_BIAS_RINGING
	error |= verifyIndirectReg(	41	,	INIT_IR41		);	//	0x0C00	DCDC_MIN_V
	error |= verifyIndirectReg(	43	,	INIT_IR43		);	//	0x1000	LOOP_CLOSE_TRES Low
	error |= verifyIndirectReg(	99	,	INIT_IR99		);	//	0x00DA	FSK 0 FREQ PARAM
	error |= verifyIndirectReg(	100	,	INIT_IR100		);	//	0x6B60	FSK 0 AMPL PARAM
	error |= verifyIndirectReg(	101	,	INIT_IR101		);	//	0x0074	FSK 1 FREQ PARAM
	error |= verifyIndirectReg(	102	,	INIT_IR102		);	//	0x79C0	FSK 1 AMPl PARAM
	error |= verifyIndirectReg(	103	,	INIT_IR103		);	//	0x1120	FSK 0to1 SCALER
	error |= verifyIndirectReg(	104	,	INIT_IR104		);	//	0x3BE0	FSK 1to0 SCALER
	
	return error;
}

void enablePCMhighway(void)
{
	/* pcm Mode Select */
	/*
	** Bit 0:   tri-State on Positive edge of Pclk
	** Bit 1:   GCI Clock Format, 1 PCLK per data bit
	** Bit 2:   PCM Transfer size (8 bt transfer)
	** Bit 3-4:	PCM Format mu-law
	** Bit 5:   PCM Enable
	*/
	writeDirectReg(1,0x28); 
}

void disablePCMhighway(void)
{
	/* Disable the PCM */
	writeDirectReg(1,0x08);
}

