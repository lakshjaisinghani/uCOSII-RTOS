//*****************************************************************************************************
// FILE: 
//		Example7_printfnonreentrant.c
// PURPOSE:
//		Illustration of printf not being reentrant.
// VERSIONS:
//		1: printf is not protected against reentrancy and locks up after 10 ticks.
//				How is printf in one task made to preempt another task execution of printf in the code?
//				Are the priorities of the tasks important?
//		2: printf is replaced by OS_Printf which enforces a critical section - that is one task at a time can only execute printf.
//				Right click OS_Printf and look at the definition - can you find out how the critical section is implemented?
//				Can you see the two task outputs from printf now working? Be patient and look carefully for the *'s!
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


#define VERSION 1

#define TASK1_PRIO  TASK_START_PRIO+1

#if VERSION==2
	#define printf OS_Printf 
#endif

OS_STK        Task1Stk[TASK_STK_SIZE];


//*****************************************************************************************************
// TASK:	
//		Task1
// DESCRIPTION:
//		Prints a message in an infinite loop waiting for StartTask to preempt.
//*****************************************************************************************************
void  Task1 (void *p_arg){
		
	    p_arg = p_arg;

		while (1){
			printf(" Task1 here\t Ticks=%d,\n", OSTimeGet());
		}
} //Task1

//*****************************************************************************************************
// TASK:	
//		StartTask
// DESCRIPTION:
//		First task created in app.c.
//		Creates another task with error checking and then enters an infinite loop.  Each loop iteration has a blocking call to OS.
//		In this case it is OSTimeDlyHMSM.
//		Prints a message with elapsed timer ticks and goes to sleep for 1 sec
//*****************************************************************************************************

void  StartTask (void *p_arg)
{
	INT8U  err;
    p_arg = p_arg;  // removes compiler warning of unused p_arg

   	printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);
	printf("StartTask:\t Creating Task1 with priority %d\n", TASK1_PRIO);
	printf("StartTask: begins in 5 seconds!\n");
	OSTimeDlyHMSM(0,0,5,0); // give use time to read above....
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

    while (1)                                 /* Task body, always written as an infinite loop.  */
	{       		
		printf("StartTask:\t Ticks=%d,\t delaying 10 ticks *******************************************\n", OSTimeGet());  
		OSTimeDly(10);           /* Delay 100 timer interrupt ticks       */
   }
} //StartTask

