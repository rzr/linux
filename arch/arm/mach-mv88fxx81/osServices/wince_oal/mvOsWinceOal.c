/*******************************************************************************
*                Copyright 2004, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
*******************************************************************************/

#include "mvOsWinceOal.h"
#include "mvTypes.h"




unsigned long mvOsGetCurrentTime(void)
{
	/* maen later */
	return 0;
}

void mvOsSleep(unsigned long mils)
{
	/* maen later */
}

int	mvOsRand(void)
{
	/* maen later */
	return 0;
}

/********************/
/*   Semaphors      */
/********************/

MV_STATUS   mvOsSemCreate(char *name,unsigned long init,unsigned long count,
					 unsigned long *smid)
{
	/* maen later */
	return MV_OK;
}
MV_STATUS 	mvOsSemDelete(unsigned long smid)
{
	/* maen later */
	return MV_OK;
}

MV_STATUS	mvOsSemWait(unsigned long smid, unsigned long time_out)
{
	/* maen later */
	return MV_OK;
}
MV_STATUS	mvOsSemSignal(unsigned long smid)
{
	/* maen later */
	return MV_OK;
}



/*********************/
/*    Memory         */
/*********************/


void*   mvOsMalloc(MV_U32 size)
{
	/* maen later */
	return NULL;
}

void*   mvOsRealloc(void *pVirtAddr, MV_U32 size)
{
	/* maen later */
	return NULL;
}

void mvOsFree(void *pVirtAddr)
{
	/* maen later */
}


void* mvOsIoUncachedMalloc( void* pDev, MV_U32 size, MV_U32* pPhyAddr )
{
    *pPhyAddr = (MV_U32)OALVAtoPA(mvOsMalloc(size));
    return (void *)OALPAtoUA(*pPhyAddr);
}

void mvOsIoUncachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    mvOsFree(pVirtAddr);
}

void* mvOsIoCachedMalloc( void* pDev, MV_U32 size, MV_U32* pPhyAddr )
{
    *pPhyAddr = (MV_U32)OALVAtoPA(mvOsMalloc(size));
    return (void *)OALPAtoCA(*pPhyAddr);
}

void mvOsIoCachedFree( void* pDev, MV_U32 size, MV_U32 phyAddr, void* pVirtAddr )
{
    mvOsFree(pVirtAddr);
}

MV_U32 mvOsCacheFlush( void* pDev, void* p, int size )
{
	OALFlushDCacheLines(p,size);
    return (MV_U32)p;
}

MV_U32 mvOsCacheInvalidate( void* pDev, void* p, int size )
{
	OALFlushDCacheLines(p,size);
    return (MV_U32)p;
}




void mvOsPrintf(const char *sz, ...)
{
    unsigned char    c;
    va_list         vl;
    
    va_start(vl, sz);
    
    while (*sz) {
        c = *sz++;
        switch (c) {
        case '%':
            c = *sz++;
            switch (c) { 
            case 'x':
				OALLog(L"%x",va_arg(vl, unsigned long));
                break;
            case 'B':
				OALLog(L"%B",va_arg(vl, unsigned long));
                break;
            case 'H':
				OALLog(L"%H",va_arg(vl, unsigned long));
                break;
            case 'X':
				OALLog(L"%X",va_arg(vl, unsigned long));
                break;
            case 'd':
                {
                    long    l;
                
                    l = va_arg(vl, long);
                    if (l < 0) { 
						OALLog(L"-");
                        l = - l;
                    }
					OALLog(L"%d",(unsigned long)l);
                }
                break;
            case 'u':
				OALLog(L"%u",va_arg(vl, unsigned long));
                break;
            case 's':
				OALLog(L"%s",va_arg(vl, char *));
                break;
            case '%':
				OALLog(L"%");
                break;
            case 'c':
                c = va_arg(vl, unsigned char);
				OALLog(L"%c",c);
                break;
                
            default:
				OALLog(L" ");
                break;
            }
            break;
        case '\r':
            if (*sz == '\n')
                sz ++;
            c = '\n';
            // fall through
        case '\n':
			OALLog(L"\r");
            // fall through
        default:
			OALLog(L"%c",c);        }
    }
    
    va_end(vl);
}


//
// Functional Prototypes
//
static void pOutputByte(unsigned char c);
static void pOutputNumHex(unsigned long n,long depth);
static void pOutputNumDecimal(unsigned long n);
static void OutputString (const unsigned char *s);

char *strSprintf;

unsigned int mvOsSPrintf (unsigned char *pBuf, const unsigned char *sz, ...)
{
    unsigned char    c;
    va_list         vl;
    
    va_start(vl, sz);
    
    strSprintf = pBuf;
    while (*sz) {
		//mvOsPrintf("pBuf now = 0x%x\n",pBuf);
		//mvOsPrintf("strSprintf now = 0x%x\n",strSprintf);
        c = *sz++;
        switch (c) {
        case '%':
            c = *sz++;
            switch (c) { 
            case 'x':
                pOutputNumHex(va_arg(vl, unsigned long), 0);
                break;
            case 'B':
                pOutputNumHex(va_arg(vl, unsigned long), 2);
                break;
            case 'H':
                pOutputNumHex(va_arg(vl, unsigned long), 4);
                break;
            case 'X':
                pOutputNumHex(va_arg(vl, unsigned long), 8);
                break;
            case 'd':
                {
                    long    l;
                
                    l = va_arg(vl, long);
                    if (l < 0) { 
                        pOutputByte('-');
                        l = - l;
                    }
                    pOutputNumDecimal((unsigned long)l);
                }
                break;
            case 'u':
                pOutputNumDecimal(va_arg(vl, unsigned long));
                break;
            case 's':
                OutputString(va_arg(vl, char *));
                break;
            case '%':
                pOutputByte('%');
                break;
            case 'c':
                c = va_arg(vl, unsigned char);
                pOutputByte(c);
                break;
                
            default:
                pOutputByte(' ');
                break;
            }
            break;
        case '\r':
            if (*sz == '\n')
                sz ++;
            c = '\n';
            // fall through
        case '\n':
            pOutputByte('\r');
            // fall through
        default:
            pOutputByte(c);
        }
    }
    pOutputByte(0);
    c = strSprintf - pBuf;
    strSprintf = 0;
    va_end(vl);
    return (c);
}
/*****************************************************************************
*
*
*   @func   void    |   pOutputByte | Sends a byte out of the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned int |   c |
*               Byte to send.
*
*/
static void
pOutputByte(
    unsigned char c
)
{
   *strSprintf++ = c;
}


/*****************************************************************************
*
*
*   @func   void    |   pOutputNumHex | Print the hex representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*   @parm   long | depth |
*               Minimum number of digits to print.
*
*/
static void pOutputNumHex (unsigned long n, long depth)
{
    if (depth) {
        depth--;
    }
    
    if ((n & ~0xf) || depth) {
        pOutputNumHex(n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10) {
        pOutputByte((unsigned char)(n + '0'));
    } else { 
    pOutputByte((unsigned char)(n - 10 + 'A'));
}
}


/*****************************************************************************
*
*
*   @func   void    |   pOutputNumDecimal | Print the decimal representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*/
static void pOutputNumDecimal (unsigned long n)
{
    if (n >= 10) {
        pOutputNumDecimal(n / 10);
        n %= 10;
    }
    pOutputByte((unsigned char)(n + '0'));
}

/*****************************************************************************
*
*
*   @func   void    |   OutputString | Sends an unformatted string to the monitor port.
*
*   @rdesc  none
*
*   @parm   const unsigned char * |   s |
*               points to the string to be printed.
*
*   @comm
*           backslash n is converted to backslash r backslash n
*/
static void OutputString (const unsigned char *s)
{
    while (*s) {        
        if (*s == '\n') {
            pOutputByte('\r');
        }
        pOutputByte(*s++);
    }
}



