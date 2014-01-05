#include "BuffaloUart.h"
#include "mvUart.h"

#define CONSOLEPORT 0
#define MICONPORT 1

//#define MICONMSG

static volatile MV_UART_PORT* uartBase[MV_UART_MAX_CHAN]={mvUartBase(CONSOLEPORT),mvUartBase(MICONPORT)}; 


static MV_VOID mvUartInit2(MV_U32 port, MV_U32 baudDivisor)
{
	volatile MV_UART_PORT *pUartPort=uartBase[port];
	unsigned char ier;
	
	//uartBase[port] = pUartPort = (volatile MV_UART_PORT *)base;
	
	ier = pUartPort->ier;
	
	pUartPort->ier = 0x00;
	pUartPort->lcr = LCR_DIVL_EN;           /* Access baud rate */
	pUartPort->dll = baudDivisor & 0xff;
	pUartPort->dlm = (baudDivisor >> 8) & 0xff;
	if (port==0){
		pUartPort->lcr = LCR_8N1;
	}else{
		pUartPort->lcr = LCR_WLS_8 | LCR_2_STB | LCR_PEN | LCR_EPS;
	}
	/* Clear & enable FIFOs */
	pUartPort->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
	
	pUartPort->ier = ier;
		
	return;
}

static MV_VOID	mvUartPutc2(MV_U32 port, MV_U8 c)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_THRE) == 0) ;
	pUartPort->thr = c;
	return;
}

static MV_U8	mvUartGetc2(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	while ((pUartPort->lsr & LSR_DR) == 0) ;
	return (pUartPort->rbr);
}

MV_BOOL mvUartTstc2(MV_U32 port)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	return ((pUartPort->lsr & LSR_DR) != 0);
}

static void output(int port, const unsigned char *buff, int len)
{
	int i=0;
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	unsigned char ier;

	ier = pUartPort->ier;
	pUartPort->ier = 0;
	
	while (len--){
		mvUartPutc2(port,buff[i++]);
	}
	pUartPort->ier = ier;
}

static int input(int port, unsigned char *ch, unsigned tmout_ms)
{
	volatile MV_UART_PORT *pUartPort = uartBase[port];
	unsigned char status=0xff;
	unsigned char ier;
	
	ier = pUartPort->ier;
	pUartPort->ier = 0;
	
	tmout_ms /= 10;
	if (tmout_ms==0){
		tmout_ms=1;
	}
	
	do{
		if (--tmout_ms == 0){
#ifdef MICONMSG
			printk("DATA_RDY : Time out.\n");
#endif
			break;
		}
		mvOsDelay(1);  /* ms */
		status = mvUartTstc2(port);
#ifdef MICONMSG
		//printk("status=%x\n",status);
#endif
	} while (!status);
	
#ifdef MICONMSG
	printk(">%s:port=%d: status=%x tmout=%x\n",__FUNCTION__,port,status,tmout_ms);
#endif
	if (tmout_ms == 0){
		return -1;
	}
	*ch = mvUartGetc2(port);
	pUartPort->ier = ier;
	return 0;
}

//----------------------------------------------------------------------
// Initialize 
//----------------------------------------------------------------------
void BuffaloInitUart(void)
{
	//printk("%s (Debug): Entered.\n");
	unsigned baseclk=mvBoardTclkGet()/16;
	//printk("%s (Debug): baseclk=%u\n", __FUNCTION__, baseclk);
	//unsigned char tmp;
	
	//printk("%s (Debug): %x %x --\n",__FUNCTION__,baseclk/115200,baseclk/38400);
	
	// test : mvUartInit2(CONSOLEPORT, baseclk/115200);	/* 115200 */
	mvUartInit2(MICONPORT,   baseclk/38400);
	
	// test : BuffaloConsoleOutput("123456789123456789");

	//printk("%s (Debug): Leaving.\n");	
	return;
}

//----------------------------------------------------------------------
// for Console (port=0)
//----------------------------------------------------------------------
void BuffaloConsoleOutput(const unsigned char *buff)
{
	output(CONSOLEPORT,buff,strlen(buff));
}

int BuffaloConsoleInput(unsigned char *ch, unsigned tmout)
{
	return input(MICONPORT,ch,tmout);
}

//----------------------------------------------------------------------
// for Micon (port=1)
//----------------------------------------------------------------------
void BuffaloMiconOutput(const unsigned char *buff, int len)
{
	output(MICONPORT,buff,len);
}

int BuffaloMiconInput(unsigned char *ch, unsigned tmout)
{
	return input(MICONPORT,ch,tmout);
}

