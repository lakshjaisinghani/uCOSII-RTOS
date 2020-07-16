//*****************************************************************************************************
// FILE: 
//		Example3_IncDecCS.c
// PURPOSE:
//		Example of why critical sections are needed to protect a shared data structure.
//		An integer, shared_int, is shared between the two tasks, Task1 and StartTask.
//		Each task both increments and decrements once per loop iteration, so no net change should occur.
// VERSIONS:
//		1: No mutual exclusion critical section is implemented.
//				What results are obtained that are unexpected?  You may need to wait 10 secs!
//		2: Mutual exclusion is applied to each increment and decrement separately.
//				What is the maximum expected value of the shared variable?  How can this be reduced to 0?
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

#define TASK1_PRIO TASK_START_PRIO+1
OS_STK        Task1Stk[TASK_STK_SIZE];

#define VERSION 2  // 1 no critical section, 2: separately protects inc and dec

#if VERSION == 1
	#define  CS(x) x; // removes Critical Section code
#endif

static int shared_int=0;

//*****************************************************************************************************
// TASK:	
//		Task1
// DESCRIPTION:
//		Runs an infinite loop of an increment followed by a decrement.  CS is a critical section
//		that is implemented with a semaphore via the CS macro defined in example.h.  
//		CS is overridden when VERSION is 1.
//		Note that there is no blocking OS call in the infinite loop - this is deliberate and allows
//		a higher priority task (ie StartTask) to preempt this task at random times of the loop
//		that depend only on instruction execution times.
//*****************************************************************************************************
void  Task1 (void *p_arg){
	INT8U  err;
	int i;
	register int temp;
	    p_arg = p_arg;
    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{   

		CS(
			temp  = shared_int;
			++temp;
			shared_int = temp;
			)
		CS(
			temp = shared_int;
			--temp;
			shared_int = temp;
		)
    }


} //Task1

//*****************************************************************************************************
// TASK:	
//		StartTask
// DESCRIPTION:
//		First task created in app.c.
//		Creates anther Task1 with lower priority (important!).  
//		Task1 then increments and decrements the shared variable shared_int 100 times.  Note that
//		the OSTimeDlyHMSM calls allow Task1 to run.
//		The use of the register variable temp accentuates the possibility of unprotected updates of 
//		shared_int (when CS is disabled in VERSION 1) but is also representative of loading from
//		memory to a register, incrementing the register and writing back to memory.
//		Exercise:
//				Change two lines of StartTask and also Task1 to ensure shared_int is only every
//				0 when printed.
//*****************************************************************************************************

void  StartTask (void *p_arg)
{
	INT8U  err;
	int i;
	register int temp;
    p_arg = p_arg;  // removes compiler warning of unused p_arg

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                /* Determine CPU capacity                                                     */
#endif
    
   	OS_Printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);
   	OS_Printf("StartTask:\t Ticks=%d, Creating Task1 with priority %d\n", OSTimeGet(), TASK1_PRIO);
    err = OSTaskCreateExt(Task1,
                    (void *)0,
                    (OS_STK *)&Task1Stk[TASK_STK_SIZE-1],
                    TASK1_PRIO,
                    TASK1_PRIO,
                    (OS_STK *)&Task1Stk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task1");

    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{       		
		OS_Printf("StartTask:\t Ticks=%d,  shared_int = %d\n", OSTimeGet(), shared_int);  
		OSTimeDly(1);  // one timer interrupt delay, so Task1 runs
		for (i=0; i<100; i++){
			CS(
				temp  = shared_int; // load into register
				++temp;
				OSTimeDly(1);  // context switch to Task1
				shared_int = temp; //back to memory
				)
			CS(
				temp = shared_int;
				--temp;
				OSTimeDly(1); // context switch to Task1
				shared_int = temp;
				)
		}
    }
} //StartTask

