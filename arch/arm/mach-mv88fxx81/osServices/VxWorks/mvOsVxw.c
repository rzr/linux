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


#include <vxWorks.h>
#include <intLib.h>
#include <msgQLib.h>
#include <objLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <taskLib.h>
#include <time.h>
#include <ftpLib.h>
#include <fioLib.h>
#include "config.h"
#include "ioLib.h"


#include "mvOs.h"

#define MV_OS_DEF_STACK_SIZE  	0x2000
#define MV_OS_STAND_MSG_LENGTH	4

/* externs */
IMPORT void  sysPrintfPoll   (char *pString, int nchars, int  outarg);
IMPORT int	consoleFd;		/* fd of initial console device */

/* locals */
LOCAL STATUS printbuf(char *buf, int nbytes, int fd);


/*******************
 * Public Functions
 *******************
 */


/*******************************************************************************
* mvOsVxwPrintf - Printf function.
*
* DESCRIPTION:
*       This routine introduce single API for formatted message printing 
*       for both before and after console initilalization.
*       Before initialization the characters will be output to CONSOLE_TTY
*       using poll mode function implemented in the BSP. 
*       After console initialization the characters will be output to file
*       descriptors using IO layer.
*
* INPUT:
*       fmt - Formatted string.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Number of bytes been printed.
*
*******************************************************************************/
void mvOsVxwPrintf(const char *  fmt, ...)
{
    va_list vaList;	/* traverses argument list */
    int nChars;
    FUNCPTR outRoutine;
    int     outArg;

    if (consoleFd == NONE)
    {
        outRoutine = (FUNCPTR)sysPrintfPoll;
        outArg     = CONSOLE_TTY;
    }
    else
    {
        outRoutine = (FUNCPTR)printbuf;
        outArg     = STD_OUT;
    }

    va_start (vaList, fmt);
    nChars = fioFormatV (fmt, vaList, outRoutine, outArg);
    va_end (vaList);
}


/*******************************************************************************
* printbuf - Print a buffer using IO.
*
* DESCRIPTION:
*       This routine supports printf() routine: print characters in a buffer
*       into given file descriptor fd.
*
* INPUT:
*       buf    - The printed buffer.
*       nbytes - Number of bytes to print.
*       fd     - File descriptor.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Number of bytes been printed.
*
*******************************************************************************/
LOCAL STATUS printbuf(char *buf, int nbytes, int fd)
{
    return (write (fd, buf, nbytes) == nbytes ? OK : ERROR);
}


/* mvOsTaskCreate -- Create a task */
MV_STATUS   mvOsTaskCreate(char  *name,
                          unsigned long prio,
                          unsigned long stack,
                          unsigned (__TASKCONV *start_addr)(void*),
                          void   *arglist,
                          unsigned long *tid)
{
    unsigned long new_size;


    if (stack == 0)
      stack = MV_OS_DEF_STACK_SIZE;

    new_size = stack & ~0x01UL;

    *tid = taskSpawn (name, prio, 0, new_size, (FUNCPTR) start_addr,
                      (int) arglist, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    return MV_OK;
}


/* mvOsTaskIdent -- Get task ID using task name */
MV_STATUS   mvOsTaskIdent(char *name, unsigned long *tid)
{
    if (name == NULL)
    {
        *tid = taskIdSelf();
    }
    else
        *tid = taskNameToId(name);

    if (*tid == ERROR)
        return MV_FAIL;

    return MV_OK;
}


/* mvOsTaskDelete -- Delete a task */
MV_STATUS   mvOsTaskDelete(unsigned long tid)
{
    STATUS rc;

    if (tid == 0)
        tid = taskIdSelf();

    rc = taskDelete(tid);

    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}


/* mvOsTaskSuspend -- Suspend a task */
MV_STATUS   mvOsTaskSuspend(unsigned long tid)
{
    STATUS rc;

    if (tid == 0)
        tid = taskIdSelf();

    rc = taskSuspend(tid);

    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}

/* mvOsTaskResume -- Resume a suspended task */
MV_STATUS   mvOsTaskResume(unsigned long tid)
{
    STATUS rc;

    rc = taskResume(tid);

    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}

/* mvOsSleep -- Send the current task to sleep */
void	mvOsSleep(unsigned long mils)
{
    int num, delay;

	if(mils == 0)
	{
		taskDelay (0);	

	}
	else
	{
 	 	num = sysClkRateGet();
    	delay = (num * mils) / 1000;
		if(delay == 0)
			delay = 1;	
    	taskDelay (delay);
	}
}

/*******************/
/*   Queues        */
/*******************/

/* mvQueueCreate -- Create a queue */
MV_STATUS   mvOsQueueCreate(char *name, unsigned long size, 
                                        unsigned long *qid)
{
    MSG_Q_ID q_id;

    q_id = msgQCreate (size, MV_OS_STAND_MSG_LENGTH, MSG_Q_FIFO);
    if (q_id == NULL)
        return MV_FAIL;

    *qid = (unsigned long) q_id;

    return MV_OK;
}


/* mvOsQueueDelete -- Delete a queue */
MV_STATUS   mvOsQueueDelete(unsigned long qid)
{
    STATUS rc;

    rc = msgQDelete ((MSG_Q_ID) qid);
    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}


/* mvOsQueueWait -- Wait for a message in queue */
MV_STATUS   mvOsQueueWait(unsigned long qid, void* msg, unsigned long time_out)
{
    int num;

    if (time_out == MV_OS_WAIT_FOREVER)
	{
        num = msgQReceive ((MSG_Q_ID) qid, (char*)msg, 
							MV_OS_STAND_MSG_LENGTH, WAIT_FOREVER);
	}
    else
    {
        int tick, delay;

        tick = sysClkRateGet();
        delay = (tick * time_out) / 1000;
        if (delay < 1)
            num = msgQReceive((MSG_Q_ID)qid, (char*)msg, 
							  MV_OS_STAND_MSG_LENGTH, 1);
        else
            num = msgQReceive((MSG_Q_ID)qid, (char*)msg, 
							  MV_OS_STAND_MSG_LENGTH, delay);
    }

	if (num == MV_OS_STAND_MSG_LENGTH)
	{
		return MV_OK;
	}
	else
	{
		if (errno == S_objLib_OBJ_TIMEOUT)
			return MV_TIMEOUT;
		else
			return MV_FAIL;
	}
}


/* mvOsQueueSend -- Put a message inside queue */
MV_STATUS   mvOsQueueSend(unsigned long qid, void* msg)
{
	STATUS rc;

	rc = msgQSend( (MSG_Q_ID)qid, (char*)msg, MV_OS_STAND_MSG_LENGTH,
					NO_WAIT, MSG_PRI_NORMAL);
    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}


/*******************/
/*  Semaphores     */
/*******************/

/* mvOsSemCreate -- Create a semaphore */
MV_STATUS   mvOsSemCreate(char *name, unsigned long init, 
					      unsigned long count, unsigned long *smid)
{
    SEM_ID s_id;
    unsigned long i;
    
    if( (init > count) || (count == 0) )
        return MV_FAIL;
    
    if (count == 1)
    {
        /* create semaphore binary */
        if (init == 0)
            s_id = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
        else
            s_id = semBCreate (SEM_Q_FIFO, SEM_FULL);
    
        if (s_id == NULL)
        {
            *smid = 0;
            return MV_FAIL;
        }
    }
    else
    {
        /* create a counting semaphore */
        s_id = semCCreate (SEM_Q_FIFO, count);
        if (s_id == NULL)
        {
            *smid = 0;
            return MV_FAIL;
        }
        for (i = 0; i < (count-init); i++)
            semTake (s_id, WAIT_FOREVER);
    }
    
    *smid = (unsigned long) s_id;
    return MV_OK;
}


/* mvOsSemDelete -- Delete a semaphore */
MV_STATUS   mvOsSemDelete(unsigned long smid)
{
    STATUS rc;

    rc = semDelete((SEM_ID) smid);
    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}



/* mvOsSemWait -- Wait on semaphore */
MV_STATUS   mvOsSemWait(unsigned long smid, unsigned long time_out)
{
    STATUS rc;

    if (time_out == MV_OS_WAIT_FOREVER)
        rc = semTake ((SEM_ID) smid, WAIT_FOREVER);
    else
    {
        int num, delay;

        num = sysClkRateGet();
        delay = (num * time_out) / 1000;
        if (delay < 1)
            rc = semTake ((SEM_ID) smid, 1);
        else
            rc = semTake ((SEM_ID) smid, delay);
    }

    if (rc != OK)
    {
        if (errno == S_objLib_OBJ_TIMEOUT)
            return MV_TIMEOUT;
        else
            return MV_FAIL;
    }
    return MV_OK;
}


/* mvOsSemSignal -- Signal a semaphore */
MV_STATUS   mvOsSemSignal(unsigned long smid)
{
    STATUS rc;

    rc = semGive ((SEM_ID) smid);
    if (rc != OK)
        return MV_FAIL;

    return MV_OK;
}


/* mvOsGetCurrentTime -- Return current system's up-time */
unsigned long mvOsGetCurrentTime()
{
    clock_t current_clock;
    
    current_clock = clock();
    current_clock = current_clock * 1000 / CLOCKS_PER_SEC;
    return (current_clock);
}    


/* mvOsGetErrNo -- Return last error number  */
unsigned long	mvOsGetErrNo()
{
	return errno;
}


/* mvOsRand -- Return random 32-bit number */
int	mvOsRand(void)
{
	return rand();
}


void*   mvOsIoCachedMalloc(void* pDev, MV_U32 size, MV_ULONG* pPhyAddr)
{
    void*   pVirtAddr;

    pVirtAddr = mvOsMalloc(size);
    if(pPhyAddr != NULL)
        *pPhyAddr = mvOsIoVirtToPhy(pDev, pVirtAddr);

    return pVirtAddr;

}

void    mvOsIoCachedFree(void* pDev, MV_U32 size, MV_ULONG phyAddr, void* pVirtAddr)
{
    mvOsFree(pVirtAddr);
}

void*   mvOsIoUncachedMalloc(void* pDev, MV_U32 size, MV_ULONG* pPhyAddr)
{
    void*   pVirtAddr;

    pVirtAddr = cacheDmaMalloc(size);
    if(pPhyAddr != NULL)
        *pPhyAddr = mvOsIoVirtToPhy(pDev, pVirtAddr);

    return pVirtAddr;
}

void    mvOsIoUncachedFree(void* pDev, MV_U32 size, MV_ULONG phyAddr, void* pVirtAddr)
{
    cacheDmaFree(pVirtAddr);
}
