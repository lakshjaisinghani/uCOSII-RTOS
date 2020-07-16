//*****************************************************************************************************
// FILE: 
//		Example4_semaphore.c
// PURPOSE:
//		Example of creating a semaphore, illustrating simple pend/post operations and timeouts.
// VERSIONS:
//		1: No timeouts.
//				What is the order of events?
//		2,3,4: A timeout of 2 ticks applies to the pend in Task1.  A delay of VERSION-1 ticks occurs
//			before the post in StartTask.
//				What delays cause a timeout?
//		5:	Task1 priorit is changed to above StartTask.  A timeout of 2 ticks applies to the pend in Task1.  A delay of 2 ticks occurs
//			before the post in StartTask.
//				Why is this different to VERSION 3?
// CONTEXT:
//		Runs with the real time kernel uCOS-II v2.80 with the win32 port by Vladimir Antonenko.
// LICENSE:
//		This is free software; you can redistribute it and/or
//		modify it under the terms of the GNU General Public License.
// AUTHOR:
//		written by Lindsay Kleeman for ECE3073, Monash University.
//*****************************************************************************************************

#include <includes.h>
#include "example.h"

#define TASK1_PRIO  TASK_START_PRIO+1

#define VERSION 1

#define DELAY (VERSION-1)
#if VERSION==5
	#define TASK1_PRIO  TASK_START_PRIO-1
	#define DELAY 2
#endif

OS_STK        Task1Stk[TASK_STK_SIZE];
OS_EVENT		 *Sem1;

//*****************************************************************************************************
// TASK:	
//		Task1
// DESCRIPTION:
//		Pends on a semaphore Sem1 that is posted in StartTask.  A timeout is applied for VERSION>1.
//		Task1 terminates itself after another 5 ticks.
//*****************************************************************************************************
void  Task1 (void *p_arg){
	INT8U  err;
	p_arg = p_arg;
#if VERSION==1
	OS_Printf("Task1:\t\t ticks=%d Waiting (pending) on an event with Sem1\n", OSTimeGet());
	OSSemPend(Sem1,0,&err);
	OS_Printf("Task1:\t\t ticks=%d Received Sem1 event\n", OSTimeGet());
#endif
#if VERSION>1
	OS_Printf("Task1:\t\t ticks=%d Pending on Sem1 with timeout of 2 ticks\n", OSTimeGet());
	OSSemPend(Sem1,2,&err);
	if (err == OS_TIMEOUT)
			OS_Printf("Task1:\t\t ticks=%d Timeout occurred waiting for Sem1 event\n", OSTimeGet());
	else if (err == OS_NO_ERR)
			OS_Printf("Task1:\t\t ticks=%d Received Sem1 event\n", OSTimeGet());
#endif 
	OSTimeDly(10);
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} //Task1

//*****************************************************************************************************
// TASK:	
//		StartTask
// DESCRIPTION:
//		First task created in app.c.
//		Creates semaphore sem1 and another task with error checking. 
//		Posts to the semaphore after VERSION-1 ticks delay and then terminates after 5 ticks.
//*****************************************************************************************************

void  StartTask (void *p_arg)
{
	INT8U  err;
    p_arg = p_arg;  // removes compiler warning of unused p_arg

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                /* Determine CPU capacity                                                     */
#endif
   	OS_Printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);

	Sem1 = OSSemCreate(0);
	if (Sem1==0) Perr("StartTask failed to create Sem1");

	OS_Printf("StartTask:\t ticks=%d Created Sem1 and creating Task1 with priority %d\n", OSTimeGet(), TASK1_PRIO);
    err = OSTaskCreateExt(Task1,
                    (void *)0,
                    (OS_STK *)&Task1Stk[TASK_STK_SIZE-1],
                    TASK1_PRIO,
                    TASK1_PRIO,
                    (OS_STK *)&Task1Stk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(Task1 ..) failed"); // checks err and exits if an error printing message - see app.c

#if VERSION==1
	OS_Printf("StartTask:\t ticks=%d Signaling (posting) Sem1\n", OSTimeGet());
	OSSemPost(Sem1);
#endif
#if VERSION>1
	OS_Printf("StartTask:\t ticks=%d Sleeping %d tick(s)\n", OSTimeGet(), DELAY);
	OSTimeDly(DELAY);
	OS_Printf("StartTask:\t ticks=%d Signaling (posting) Sem1 \n", OSTimeGet());
	OSSemPost(Sem1);
#endif
	OSTimeDly(5);
	OS_Printf("\nterminating ....\n");
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
  
} //StartTask

