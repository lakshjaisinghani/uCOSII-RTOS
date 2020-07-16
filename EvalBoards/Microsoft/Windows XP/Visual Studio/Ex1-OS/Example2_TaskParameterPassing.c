//*****************************************************************************************************
// FILE: 
//		Example1_TaskParameterPassing.c
// PURPOSE:
//		Example of passing a parameter to a task on creation.  Two tasks are created from the same root function
//		and differentiated based on the passed parameter.  An array of stacks is used for the CreateTaskExt calls.
// VERSIONS:
//		1: Separate variable are used as parameters to the two create task OS calls.
//				Are the tasks created different?
//		2: The same variable is used as the parameter but changed between create OS calls.
//				Do the tasks get the different parameter values as intended?
//		3: TASK1_PRIO is changed to TASK_START_PRIO-1
//				What has changed over VERSION 2?  Why are the results different?
//		4: Create multiple tasks in a loop.
//				Why aren't all created				
//		5: Create multiple tasks in a loop.
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

#define VERSION 5  //1,2,3,4,5

#if VERSION == 3
	#define TASK1_PRIO TASK_START_PRIO-1
#else
	#define TASK1_PRIO  TASK_START_PRIO+1
#endif
#define TASK2_PRIO  TASK_START_PRIO+2


#define  N_TASKS  7       /* Number of identical tasks                          */
OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */

//*****************************************************************************************************
// TASK:	
//		Task
// DESCRIPTION:
//		Accepts a parameter that is assumed to be an integer that identifies the task -  task_num.
//		Prints an identifying message with elapsed timer ticks and goes to sleep for 1 sec.
//*****************************************************************************************************
void  Task (int *p_arg){
	int task_num = *(int *)p_arg;
	 while(1){
		OS_Printf("Task%d:\t\t Ticks=%d, still here\n", task_num, OSTimeGet());
		OSTimeDlyHMSM(0, 0, 1, 0);           /* Delay 1 second      */
	 }

} //Task

//*****************************************************************************************************
// TASK:	
//		StartTask
// DESCRIPTION:
//		First task created in app.c.
//		Creates two other tasks with parameters 0 and 1 and then enters an infinite loop that blocks for a second.
//		Note the use of an array of stacks in the OS calls to OSTaskCreateExt()
//*****************************************************************************************************

void  StartTask (void *p_arg)
{
	INT8U      err;
	int count0=0, count1=1, i;
    p_arg = p_arg;  // removes compiler warning of unused p_arg

   	OS_Printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);
    OS_Printf("StartTask:\t Ticks=%d, Creating Task with priority %d and parameter %d\n", OSTimeGet(), TASK1_PRIO, count0);
	err = OSTaskCreateExt(Task,
                    &count0,
                    (OS_STK *)&TaskStk[0][TASK_STK_SIZE - 1],
                    TASK1_PRIO,
                    TASK1_PRIO,
                    (OS_STK *)&TaskStk[0][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task1");

#if VERSION == 1
    OS_Printf("StartTask:\t Ticks=%d, Creating Task with priority %d and parameter %d\n", OSTimeGet(), TASK2_PRIO, count1);
	err = OSTaskCreateExt(Task,
                    &count1,
                    (OS_STK *)&TaskStk[1][TASK_STK_SIZE - 1],
                    TASK2_PRIO,
                    TASK2_PRIO,
                    (OS_STK *)&TaskStk[1][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task2");
#elif (VERSION == 2 || VERSION == 3)
	count0++;
    OS_Printf("StartTask:\t Ticks=%d, Creating Task with priority %d and parameter %d\n", OSTimeGet(), TASK2_PRIO, count0);
	err = OSTaskCreateExt(Task,
                    (void *)&count0,
                    (OS_STK *)&TaskStk[1][TASK_STK_SIZE - 1],
                    TASK2_PRIO,
                    TASK2_PRIO,
                    (OS_STK *)&TaskStk[1][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task2");
#elif (VERSION == 4)
	for(i=0; i<2; i++)
	{
		OSTimeDly(1);
		count0++;
		OS_Printf("StartTask:\t Ticks=%d, Creating Task with priority %d and parameter %d\n", OSTimeGet(), TASK2_PRIO+i, count0);
		err = OSTaskCreateExt(Task,
                    (void *)&count0,
                    (OS_STK *)&TaskStk[1][TASK_STK_SIZE - 1],
                    TASK2_PRIO+i,
                    TASK2_PRIO+i,
                    (OS_STK *)&TaskStk[1][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		Perror(err, "ERROR: Failed to create task Task2");
	}
#else	
	for(i=0; i<2; i++)
	{
		OSTimeDly(1);
		count0++;
		OS_Printf("StartTask:\t Ticks=%d, Creating Task with priority %d and parameter %d\n", OSTimeGet(), TASK2_PRIO+i, count0);
		err = OSTaskCreateExt(Task,
                    (void *)&count0,
                    (OS_STK *)&TaskStk[i+1][TASK_STK_SIZE - 1],
                    TASK2_PRIO+i,
                    TASK2_PRIO+i,
                    (OS_STK *)&TaskStk[i+1][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		Perror(err, "ERROR: Failed to create task Task2");
	}
#endif


	OS_Printf("TaskStart:\t Ticks=%d, still here!\n", OSTimeGet());

    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{       		
		OSTimeDlyHMSM(0, 0, 1, 0);  
    }
} //StartTask

