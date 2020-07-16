//*****************************************************************************************************
// FILE: 
//		Example8_priorityinversion.c
// PURPOSE:
//		Example of a priority inversion with 3 tasks.  A high priority task is prevented 
//		from running by a low priority task holding (pends) a semaphore and being 
//		preempted by a medium priority task before it can release (post) the semaphore.
//		The medium priority task does lots of processing, preventing the low priority 
//		task from running and releasing the semaphore.  Effectively the medium task 
//		prevents the high priority task from running. A solution is shown with a 
//		uCOS-II mutex that temporarily raises the priority of the low task to release
//		the semaphore early.
// VERSIONS:
//		1:  Priority inversion.
//				What is the sequence of tasks running?
//		2:  Uses a uCOS-II mutex to solve the priority inversion.
//				How has this sequence changed?
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

#if VERSION==2 
	#define USE_MUTEX
#endif

#define TASK_HIGH_PRIO		TASK_START_PRIO+1
#define TASK_MEDIUM_PRIO	TASK_START_PRIO+2
#define TASK_LOW_PRIO		TASK_START_PRIO+3

OS_STK        TaskHStk[TASK_STK_SIZE];
OS_STK        TaskMStk[TASK_STK_SIZE];
OS_STK        TaskLStk[TASK_STK_SIZE];

OS_EVENT	*sem_mutex;
OS_EVENT	*resource_mutex;


//*****************************************************************************************************
// TASK:	
//		TaskHigh
// DESCRIPTION:
//		Should always run when available.
//*****************************************************************************************************
void TaskHigh(void *pdata)
{   INT8U err;
	 pdata = pdata;                                         /* Prevent compiler warning                 */
	 OSTimeDly(1);  // long enough to allow other tasks to run first
	 OS_Printf("TaskHigh:\t Started ....\n");
#ifdef USE_MUTEX
	OS_Printf("TaskHigh:\t waiting on mutex resource_mutex \n");
	OSMutexPend(resource_mutex,0,&err);
	// when TaskHigh is blocked here any task holding the mutex is raised to priority 9
	// that was initialised with the mutex resource_mutex
#else
	 OS_Printf("TaskHigh:\t waiting on semaphore sem_mutex \n");
	 OSSemPend(sem_mutex, 0, &err);  // wants to get on with it but must wait for mutex
#endif
		OS_Printf("TaskHigh:\t I'm impatient and very important: Getting on with my processing ...\n");


#ifdef USE_MUTEX
	OSMutexPost(resource_mutex);
#else
	 OSSemPost(sem_mutex);
#endif

	OS_Printf("TaskHigh:\t finished\n");
	OSTimeDly(2000);
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} // TaskHigh

//*****************************************************************************************************
// TASK:	
//		TaskMedium
// DESCRIPTION:
//		
//*****************************************************************************************************
void TaskMedium(void *pdata)
{
	long int a=0, i, j;

	pdata = pdata;                                         /* Prevent compiler warning                 */

	 OSTimeDly(1);  // long enough to allow other tasks to run first
	 // consume lots of CPU time

	OS_Printf("TaskMedium:\t I love to use lots of CPU time:   .");
	for (i=0;i<100; i++)
	 {
		OS_Printf(".");
		for (j=0;j<50L; j++)
			a++;
	 }
	OS_Printf("\nTaskMedium:\t I've enjoyed the ride but alas finished!\n");

	OSTimeDly(2000);
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} // TaskMedium

//*****************************************************************************************************
// TASK:	
//		TaskLow
// DESCRIPTION:
//		This task can hold a semaphore preventing TaskHigh from running since TaskHigh is pending on
//		that semaphore.
//*****************************************************************************************************
void TaskLow(void *pdata)
{
	INT8U err;
	pdata = pdata;                                         /* Prevent compiler warning                 */

#ifdef USE_MUTEX
	OS_Printf("TaskLow:\t the early bird gets the worm (or resource_mutex in this case)\n");
	OSMutexPend(resource_mutex,0,&err);
#else
	OS_Printf("TaskLow:\t the early bird gets the worm (or sem_mutex in this case)\n");
	OSSemPend(sem_mutex,0,&err);
#endif

	OSTimeDly(1);  // let other tasks run - but this task has the mutex semaphore!

#ifdef USE_MUTEX
	OS_Printf("TaskLow:\t now ready to give up resource_mutex ... I'm low priority anyway!\n");
	OSMutexPost(resource_mutex);
#else
	OS_Printf("TaskLow:\t now ready to give up sem_mutex ... I'm low priority anyway!\n");
	OSSemPost(sem_mutex);
#endif
	OSTimeDly(2000);
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} // TaskLow


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

   	OS_Printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION);

#ifdef USE_MUTEX
	 OS_Printf("Priority Inversion solved:\nRESOURCE MUTEX version: **********************\n");
	 resource_mutex = OSMutexCreate(TASK_START_PRIO-1, &err);
		// TASK_START_PRIO-1 is the priority inheritance priority PIP reserved for the mutex
		//    - must be higher than all tasks that use mutex
		// priorities of tasks hold mutex are raised temporarily to this priority
	 if (resource_mutex == 0) Perr("Failed to create resource_mutex mutex in TaskStart()");
#else
	 OS_Printf("Priority Inversion a problem:\nSEMAPHORE version: **********************\n");
	 sem_mutex = OSSemCreate(1);
	 if (sem_mutex == 0) Perr("Failed to create sem_mutex semaphore in TaskStart()");
#endif

	OS_Printf("StartTask:\t Creating TaskHigh with priority %d\n", TASK_HIGH_PRIO);
	err = OSTaskCreateExt(TaskHigh,
                    (void *)0,
                    (OS_STK *)&TaskHStk[TASK_STK_SIZE-1],
                    TASK_HIGH_PRIO,
                    TASK_HIGH_PRIO,
                    (OS_STK *)&TaskHStk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(TaskHigh ..) failed"); // checks err and exits if an error printing message - see app.c

	OS_Printf("StartTask:\t Creating TaskMedium with priority %d\n", TASK_MEDIUM_PRIO);
 	err = OSTaskCreateExt(TaskMedium,
                    (void *)0,
                    (OS_STK *)&TaskMStk[TASK_STK_SIZE-1],
                    TASK_MEDIUM_PRIO,
                    TASK_MEDIUM_PRIO,
                    (OS_STK *)&TaskMStk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(TaskMedium ..) failed"); // checks err and exits if an error printing message - see app.c

	OS_Printf("StartTask:\t Creating TaskLow with priority %d\n", TASK_LOW_PRIO);
 	err = OSTaskCreateExt(TaskLow,
                    (void *)0,
                    (OS_STK *)&TaskLStk[TASK_STK_SIZE-1],
                    TASK_LOW_PRIO,
                    TASK_LOW_PRIO,
                    (OS_STK *)&TaskLStk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(TaskLow ..) failed"); // checks err and exits if an error printing message - see app.c
   
	
	while (1)                                 /* Task body, always written as an infinite loop.  */
	{       		
        OSTimeDlyHMSM(0, 0, 1, 0);       
    }
} //StartTask

