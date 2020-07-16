//*****************************************************************************************************
// FILE: 
//		Example5_SingleBuffer.c
// PURPOSE:
//		Example of semaphore use for protecting and managing a single item buffer.
// VERSIONS:
//		1: Consumer has higher priority.
//		2: Producer has higher priority.
//				What changes from VERSION 1?
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

#define VERSION 2

#if VERSION==1
	#define TASK_CONSUME_PRIO  TASK_START_PRIO+1
	#define TASK_PRODUCE_PRIO  TASK_START_PRIO+2
#endif

#if VERSION==2
	#define TASK_CONSUME_PRIO  TASK_START_PRIO+2
	#define TASK_PRODUCE_PRIO  TASK_START_PRIO+1
#endif

// Shared variables ************************************************************************************
OS_STK		Task1Stk[TASK_STK_SIZE];
OS_STK		Task2Stk[TASK_STK_SIZE];
OS_EVENT	*sem_produced, *sem_consumed;

// Single-item buffer object
// The object shared by TaskProduce and TaskConsume
// tasks consists of an integer variable
// and two functions, store_item and retrieve_item1 that operate on it.

static int buf;

 //*************************************
 //store_item - store item in buffer object
 //*************************************
 void store_item1(int item)
 {
	 buf = item;
 } //store_item

 //*************************************
 // retrieve item1 - return the value stored in buffer object
 //*************************************
 int retrieve_item1(void)
 {
	 return buf;
 } // retrieve_item
 // end single-item buffer object


//*************************************
// TASK:	
//		TaskProduce
// DESCRIPTION:
//		The task first waits for any previously produced items to be consumed
//		so there is room in the buffer by pending on sem_consumed.
//		An item is then stored and the sem_produced is posted to signal this event.
//*************************************
 void TaskProduce(void *pdata)
 {
	 INT8U err;
	 int i;
	 pdata = pdata;  /* avoid compiler warning */

	 for (i = 0; i<10; i++)
	 {
		 OSSemPend(sem_consumed,0,&err);
		 OS_Printf("TaskProducer:\t ticks=%d Produced %d\n", OSTimeGet(), i);
		 store_item1(i);
		 OSSemPost(sem_produced);
	 }
	OSTimeDly(20);  
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
 } //  produce

//*************************************
// TASK:	
//		TaskConsume
// DESCRIPTION:
//		Task function for consumer of int item -
//		must first pend on sem_produced to be post by the producer task.
//		Posts sem_consumed to indicate that another item can now be produced.
 //*************************************
 void TaskConsume(void *pdata)
 {
	 int j;
	 INT8U err;

	 pdata = pdata;  /* avoid compiler warning */
	 for (j = 0; j<10; j++)
	 {
		 OSSemPend(sem_produced,0,&err);
		 OS_Printf("TaskConsumer:\t ticks=%d Consumed %d \n", OSTimeGet(), retrieve_item1());
		 OSSemPost(sem_consumed);
	 }
	OSTimeDly(20);  
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
 } //  produce


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

	sem_produced = OSSemCreate(0);
	if (sem_produced==0) Perr("StartTask failed to create sem_produced");

	sem_consumed = OSSemCreate(1);
	if (sem_consumed==0) Perr("StartTask failed to create sem_consumed");

	OS_Printf("StartTask:\t ticks=%d Created sem_produced, sem_consumed\n", OSTimeGet());
	OS_Printf("StartTask:\t ticks=%d Creating TaskProduce with priority %d\n", OSTimeGet(), TASK_PRODUCE_PRIO);
    err = OSTaskCreateExt(TaskProduce,
                    (void *)0,
                    (OS_STK *)&Task1Stk[TASK_STK_SIZE-1],
                    TASK_PRODUCE_PRIO,
                    TASK_PRODUCE_PRIO,
                    (OS_STK *)&Task1Stk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
	OS_Printf("StartTask:\t ticks=%d Creating TaskConsume with priority %d\n", OSTimeGet(), TASK_CONSUME_PRIO);
    err = OSTaskCreateExt(TaskConsume,
                    (void *)0,
                    (OS_STK *)&Task2Stk[TASK_STK_SIZE-1],
                    TASK_CONSUME_PRIO,
                    TASK_CONSUME_PRIO,
                    (OS_STK *)&Task2Stk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR - OSTaskCreate(TaskConsume ..) failed"); // checks err and exits if an error printing message - see app.c

	OSTimeDly(20);  // allow time for consumer and producer to terminate
	OS_Printf("\nTerminating ....\n");
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
  
} //StartTask

