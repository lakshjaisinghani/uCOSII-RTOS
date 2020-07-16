//*****************************************************************************************************
// FILE: 
//		Example6_multibuffer.c
// PURPOSE:
//		Example of semaphore use for protecting and managing a multiple item buffer.
//		The example also illustrates deadlock in version 4.  This requires the use
//		of CS(printf()) rather than the OS_Printf().  The critical section macro CS is
//		define in example.h and is implemented with a mutual exclusion semaphore.
// VERSIONS:
//		1: Consumer works slower than producer.  Consumer has higher priority than producer.
//		2: Producer works slower than consumer.  Consumer has higher priority than producer.
//				What changes from VERSION 1?
//		3: Consumer works slower than producer.  Producer has higher priority.
//				What has changed?
//		4: Deadlock is demonstrated.
//				Where in each task is the code deadlocked and what resources are held when the deadlock occurs?
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

#define TASK_CONSUME_PRIO  TASK_START_PRIO+1
#define TASK_PRODUCE_PRIO  TASK_START_PRIO+2

#if VERSION==3
	#define TASK_CONSUME_PRIO  TASK_START_PRIO+2
	#define TASK_PRODUCE_PRIO  TASK_START_PRIO+1
#endif

// Shared variables ************************************************************************************
OS_STK		Task1Stk[TASK_STK_SIZE];
OS_STK		Task2Stk[TASK_STK_SIZE];
OS_EVENT	*sem_full_places, *sem_empty_places;
OS_EVENT	*sem_buffer_mutex;

// Multi-item buffer object
// The object shared by TaskProduce and TaskConsume
// The buffer object consists of an integer variable
// and two functions, store_item and retrieve_item1 that operate on it.


// a simple ring buffer is used to hold the data
#define SIZE 5

static int buffer[SIZE];
static int in_index, out_index, count;

 //*************************************
 //StoreItem - store item in buffer object
 //*************************************
void StoreItem(int item)
// store item in buffer object
{
	INT8U err;

	OSSemPend(sem_empty_places,0,&err);
		OSSemPend(sem_buffer_mutex,0,&err);
			buffer[in_index] = item;
			in_index = (in_index + 1) % SIZE;
			count++;
		OSSemPost(sem_buffer_mutex);
	 OSSemPost(sem_full_places);
} // StoreItem

int RetrieveItem(void)
// remove item from buffer object
{
	INT8U err;
	int item;

	OSSemPend(sem_full_places,0,&err);
		OSSemPend(sem_buffer_mutex,0,&err);
			item = buffer[out_index];
			out_index = (out_index + 1) % SIZE;
			count--;
		OSSemPost(sem_buffer_mutex);
	 OSSemPost(sem_empty_places);
	return item;
} //RetrieveItem

void InitBuffer(void) // initialize buffer object
{
	// set up the indices into the buffer
	in_index = 0;
	out_index = 0;
	count = 0; // initially, the buffer is empty
	
	sem_buffer_mutex = OSSemCreate(1);
	if (sem_buffer_mutex == 0) Perr("Failed to create sem_buffer_mutex semaphore in InitBuffer()");
   	
	sem_full_places = OSSemCreate(0);
	if (sem_full_places==0) Perr("StartTask failed to create sem_produced");

	sem_empty_places = OSSemCreate(SIZE);
	if (sem_empty_places==0) Perr("StartTask failed to create sem_consumed");

} // InitBuffer
// end multi-item buffer object


//*************************************
// TASK:	
//		TaskProduce
// DESCRIPTION:
//		The task first waits for any previously produced items to be consumed
//		so there is room in the buffer by pending on sem_empty_places.
//		An item is then stored and the sem_full_places is posted to signal this event.
//*************************************
 void TaskProduce(void *pdata)
 {
	 INT8U err;
	 int i;
	 pdata = pdata;  /* avoid compiler warning */

	 for (i = 0; i<10; i++)
	 {
		 CS(printf("TaskProducer:\t ticks=%d Produced %d\n", OSTimeGet(), i));
		 StoreItem(i);
#if VERSION==2
		 OSTimeDlyHMSM(0,0,0, 500);  // 0.5 secs
#endif
	 }
	OSTimeDlyHMSM(0,0,5,0);  
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
	 int j, item;
	 INT8U err;

	 pdata = pdata;  /* avoid compiler warning */
	 for (j = 0; j<10; j++)
	 {
#if VERSION==4
		 CS(printf("TaskConsumer:\t ticks=%d Consumed %d \n", OSTimeGet(), RetrieveItem() ));
#else
		 item = RetrieveItem();
		 CS(printf("TaskConsumer:\t ticks=%d Consumed %d \n", OSTimeGet(), item ));
#endif
#if VERSION==1 || VERSION==3
		 OSTimeDlyHMSM(0,0,0, 500);  // 0.5 secs
#endif
	 }
	OSTimeDlyHMSM(0,0,5,0);  
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
   	CS(printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION));
	InitBuffer();

	CS(printf("StartTask:\t ticks=%d Created sem_full_places, sem_empty_places\n", OSTimeGet()));
	CS(printf("StartTask:\t ticks=%d Creating TaskProduce with priority %d\n", OSTimeGet(), TASK_PRODUCE_PRIO));
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
	CS(printf("StartTask:\t ticks=%d Creating TaskConsume with priority %d\n", OSTimeGet(), TASK_CONSUME_PRIO));
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

	OSTimeDlyHMSM(0,0,5,0);   // allow time for consumer and producer to terminate
	CS(printf("\nTerminating ....\n"));
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} //StartTask

