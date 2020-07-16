//*****************************************************************************************************
// FILE: 
//		Example1_Tasks.c
// PURPOSE:
//		Example of creating tasks with different priorities.  A different time delay is used in two tasks.
//		The number of operating system timer interrupt ticks since starting is illustrated.
// VERSIONS:
//		1: TASK1_PRIO the same as TASK_START_PRIO
//				What goes wrong?
//		2: TASK1_PRIO is TASK_START_PRIO+1
//				Which task takes priority?
//		3: TASK1_PRIO is TASK_START_PRIO-1
//				What has changed over VERSION 2?
//		4: TASK1 is a function instead of a task
//				What has changed?
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


#define VERSION 3
#if VERSION==1
	#define TASK1_PRIO  TASK_START_PRIO
#endif
#if VERSION==2
	#define TASK1_PRIO  TASK_START_PRIO+1
#endif
#if VERSION>2
	#define TASK1_PRIO  TASK_START_PRIO-1
#endif



OS_STK        Task1Stk[TASK_STK_SIZE];


//*****************************************************************************************************
// TASK:	
//		Task1
// DESCRIPTION:
//		Prints a message with elapsed timer ticks and goes to sleep for 500 msec
//*****************************************************************************************************
void  Task1 (void *p_arg){
	    p_arg = p_arg;

		while (1){
			OS_Printf("Task1:\t\t Ticks=%d,\t delaying 500 msec\n", OSTimeGet());  /* your code here. Create more tasks, etc.                                    */
			OSTimeDlyHMSM(0, 0, 0, 500); 

		}
} //Task1

//***********************************************************

void notatask()
{
	while (1){
		OS_Printf("Notatask:\t\t Ticks=%d,\t delaying 1 msec\n", OSTimeGet());  /* your code here. Create more tasks, etc.                                    */
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

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

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                /* Determine CPU capacity                                                     */
#endif
   	OS_Printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);
	OS_Printf("StartTask:\t Creating Task1 with priority %d\n", TASK1_PRIO);
    #if VERSION<4
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
	#endif

	#if VERSION==4
		notatask();
	#endif
	

    while (1)                                 /* Task body, always written as an infinite loop.  */
	{       		
		OS_Printf("StartTask:\t Ticks=%d,\t delaying 1 second\n", OSTimeGet());  
	    OSTimeDlyHMSM(0, 0, 1, 0);       
    }
} //StartTask

