//*****************************************************************************************************
// FILE: 
//		ExampleWaitrose.c
// PURPOSE:
//		Example of multiple item buffer, multiple producers, multiple .
//		The example also illustrates deadlock in version 4.  This requires the use
//		of CS(printf()) rather than the OS_Printf().  The critical section macro CS is
//		define in example.h and is implemented with a mutual exclusion semaphore.
// VERSIONS:
//		1: Single producer, single consumer
//		2: One producer, checks if space available, adds multiple items to buffer
//		3: One producer, checks if at least m spaces available, adds multiple items to buffer
//		4: One producer, checks if at least m spaces available, adds multiple items to buffer. Bug in version 2 fixed
//		5: Same code as version 4, but multiple producers
//		6: Making problem in version 5 more clear by adding time delay. Immediate deadlock
//		7: Problem fixed of version 5/6
//		8: Multiple producers, producers do unrelated work after adding to buffer. Everything stops during  
//			tea break (loses concurrency)
//		9: Attempted solution, reaches deadlock
//		10: Good concurrency, even with tea break
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

#define VERSION 10

#define SIZE 20
#define num_spaces_in_shelf_needed 12

#if VERSION == 1
	#define NUM_PRODUCERS 1
	#define NUM_CONSUMERS 1
#elif ((VERSION >= 2) && (VERSION <= 4))  // 
	#define NUM_PRODUCERS 1
	#define NUM_CONSUMERS 3
#else
	#define NUM_PRODUCERS 3
	#define NUM_CONSUMERS 3
#endif

#define TASK_CONSUME_PRIO  TASK_START_PRIO+30
#define TASK_PRODUCE_PRIO  TASK_START_PRIO+20






// Functions ********

void clean_up_broken_eggs(void)
{
	long int a=0, i, j;
	for (i=0;i<1; i++)
	{
		for (j=0;j<5000L; j++)
			a++;
	}
	OSTimeDly(5);  
}

void tea_break(void)
{
	long int a=0, i, j;
	for (i=0;i<1; i++)
	{
		for (j=0;j<5000L; j++)
			a++;
	}
	OSTimeDlyHMSM(0,0,3,0);  // 0.5 secs
}

//*********


// Shared variables ************************************************************************************
OS_STK		Task1Stk[NUM_PRODUCERS][TASK_STK_SIZE];
OS_STK		Task2Stk[NUM_CONSUMERS][TASK_STK_SIZE];
OS_EVENT	*sem_full_places, *sem_empty_places;
OS_EVENT	*sem_buffer_mutex;

// Multi-item buffer object
// The object shared by TaskProduce and TaskConsume
// The buffer object consists of an integer variable
// and two functions, store_item and retrieve_item1 that operate on it.


// a simple ring buffer is used to hold the data

static int buffer[SIZE];
static int buffer_curr_size;
static int in_index, out_index, count;

OS_EVENT	*sem_buff_curr_size;

 //*************************************
 //StoreItem - store item in buffer object
 //*************************************

int StoreItem1(int item)
{
	INT8U err;

	OSSemPend(sem_empty_places,0,&err);
		OSSemPend(sem_buffer_mutex,0,&err);
			buffer[in_index] = item;
			in_index = (in_index + 1) % SIZE;
			count++;
		OSSemPost(sem_buffer_mutex);
	 OSSemPost(sem_full_places);
	 return 1;
} // StoreItem

int StoreItem2a(int item)
{
	INT8U err;
	int i, num_updates;
	
	num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
	OSSemPend(sem_empty_places,0,&err);
		OSSemPend(sem_buffer_mutex,0,&err);
			for (i=0;i<num_updates; i++) {
				buffer[in_index] = item;
				in_index = (in_index + 1) % SIZE;
				count++;
			}
		OSSemPost(sem_buffer_mutex);
	 OSSemPost(sem_full_places);
	 return num_updates;
} // StoreItem

int StoreItem2b(int item)
{
	INT8U err;
	int i, num_updates;
	
	for (i=0;i<num_spaces_in_shelf_needed; i++) 
		OSSemPend(sem_empty_places,0,&err);
	
	num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
		OSSemPend(sem_buffer_mutex,0,&err);
			for (i=0;i<num_updates; i++) {
				buffer[in_index] = item;
				in_index = (in_index + 1) % SIZE;
				count++;
			}
		OSSemPost(sem_buffer_mutex);

	for (i=0;i<num_updates; i++) 
		OSSemPost(sem_full_places);
	 
	return num_updates;
} 

int StoreItem2c(int item)
{
	INT8U err;
	int i, num_updates;
	
	for (i=0;i<num_spaces_in_shelf_needed; i++) 
		OSSemPend(sem_empty_places,0,&err);
	
	num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
		OSSemPend(sem_buffer_mutex,0,&err);
			for (i=0;i<num_updates; i++) {
				buffer[in_index] = item;
				in_index = (in_index + 1) % SIZE;
				count++;
			}
		OSSemPost(sem_buffer_mutex);

	for (i=0;i<num_updates; i++) 
		OSSemPost(sem_full_places);

	for (i=0;i<num_spaces_in_shelf_needed-num_updates; i++) 
		OSSemPost(sem_empty_places);
	 
	return num_updates;
} 

int StoreItem3a(int item)
{
	INT8U err;
	int i, num_updates;
	
	for (i=0;i<num_spaces_in_shelf_needed; i++) 
	{
		OSSemPend(sem_empty_places,0,&err);
		clean_up_broken_eggs();
	}

	num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
		OSSemPend(sem_buffer_mutex,0,&err);
			for (i=0;i<num_updates; i++) {
				buffer[in_index] = item;
				in_index = (in_index + 1) % SIZE;
				count++;
			}
		OSSemPost(sem_buffer_mutex);

	for (i=0;i<num_updates; i++) 
		OSSemPost(sem_full_places);

	for (i=0;i<num_spaces_in_shelf_needed-num_updates; i++) 
		OSSemPost(sem_empty_places);
	 
	return num_updates;
} 


int StoreItem3b(int item)
{
	INT8U err;
	int i, num_updates=0;
	
	clean_up_broken_eggs();
	OSSemPend(sem_buff_curr_size,0,&err);
		if (count<SIZE-num_spaces_in_shelf_needed)
		{
			OSSemPend(sem_buffer_mutex,0,&err);
				num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
				for (i=0;i<num_updates; i++)
					buffer[count] = item;
				count+=num_updates;
			OSSemPost(sem_buffer_mutex);
			for (i=0;i<num_updates; i++) 
				OSSemPost(sem_full_places);
		}
	OSSemPost(sem_buff_curr_size);

	return num_updates;
} 

int StoreItem4a(int item)
{
	INT8U err;
	int i, num_updates=0;
	
	clean_up_broken_eggs();
	OSSemPend(sem_buff_curr_size,0,&err);
		if (count<SIZE-num_spaces_in_shelf_needed)
		{
			OSSemPend(sem_buffer_mutex,0,&err);
				num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
				for (i=0;i<num_updates; i++)
					buffer[count] = item;
				count+=num_updates;
			OSSemPost(sem_buffer_mutex);
			for (i=0;i<num_updates; i++) 
				OSSemPost(sem_full_places);
			tea_break();
		}
	OSSemPost(sem_buff_curr_size);

	return num_updates;
} 


int StoreItem4b(int item)
// store item in buffer object
{
	INT8U err;
	int i, num_updates=0;

	srand (OSTimeGet());
	OSSemPend(sem_buff_curr_size,0,&err);
		if (count<SIZE-num_spaces_in_shelf_needed)
		{
			OSSemPend(sem_buffer_mutex,0,&err);
				num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
				for (i=0;i<num_updates; i++)
					buffer[count] = item;
				count+=num_updates;
			OSSemPost(sem_buffer_mutex);
			OSSemPost(sem_buff_curr_size);
			for (i=0;i<num_updates; i++) 
			{
				OSSemPost(sem_full_places);
			}
			tea_break();
		}
	return num_updates;
} 

int StoreItem4c(int item)
// store item in buffer object
{
	INT8U err;
	int i, num_updates=0;

	srand (OSTimeGet());
	OSSemPend(sem_buff_curr_size,0,&err);
		if (count<SIZE-num_spaces_in_shelf_needed)
		{
			OSSemPend(sem_buffer_mutex,0,&err);
				num_updates = (int)(rand() % num_spaces_in_shelf_needed)+1;
				for (i=0;i<num_updates; i++)
					buffer[count] = item;
				count+=num_updates;
			OSSemPost(sem_buffer_mutex);
			OSSemPost(sem_buff_curr_size);
			for (i=0;i<num_updates; i++) 
			{
				OSSemPost(sem_full_places);
			}
			tea_break();
		}
		else
		{
			OSSemPost(sem_buff_curr_size);
		}
	return num_updates;
} 

//**************************************


int RetrieveItem0(void)
// remove item from buffer object
{
	INT8U err;
	OS_SEM_DATA os_data; 
	short temp;

	int item;
	
	OSSemPend(sem_full_places,0,&err);;
		OSSemPend(sem_buffer_mutex,0,&err);
			item = buffer[out_index];
			out_index = (out_index + 1) % SIZE;
			count--;
		OSSemPost(sem_buffer_mutex);
	 OSSemPost(sem_empty_places);
	return item;
} //RetrieveItem

int RetrieveItem1(void)
// remove item from buffer object
{
	INT8U err;
	OS_SEM_DATA os_data; 
	short temp;

	int item;
	
	OSSemPend(sem_full_places,0,&err);
		OSSemPend(sem_buff_curr_size,0,&err);
		OSSemPend(sem_buffer_mutex,0,&err);
			item = buffer[out_index];
			out_index = (out_index + 1) % SIZE;
			count--;
		OSSemPost(sem_buffer_mutex);
		OSSemPost(sem_buff_curr_size);
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

	sem_buff_curr_size= OSSemCreate(1);

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
 void TaskProduce(int *p_arg)
 {
	 INT8U err;
	 int task_num = *(int *)p_arg;
	 int i, num_items;
	 
	 while (1)
	 {
		 #if VERSION == 1
			num_items=StoreItem1(task_num);
		 #elif VERSION == 2
			num_items=StoreItem2a(task_num);
		 #elif VERSION == 3
			num_items=StoreItem2b(task_num);
		 #elif VERSION == 4
			num_items=StoreItem2c(task_num);
		 #elif VERSION == 5
			num_items=StoreItem2c(task_num);
		 #elif VERSION == 6
			num_items=StoreItem3a(task_num);
		 #elif VERSION == 7
			num_items=StoreItem3b(task_num);
		 #elif VERSION == 8
			num_items=StoreItem4a(task_num);
		 #elif VERSION == 9
			num_items=StoreItem4b(task_num);
		 #else
			num_items=StoreItem4c(task_num);
		 #endif
		 
		 if (num_items>0)
		 {
			CS(printf("TaskProducer%d:\t ticks=%d Produced %d item, total items_in_buffer %d\n", task_num, OSTimeGet(), num_items, count));
		 }
		 OSTimeDlyHMSM(0,0,0,500);  // 0.5 secs
	 }
 } //  produce

//*************************************
// TASK:	
//		TaskConsume
// DESCRIPTION:
//		Task function for consumer of int item -
//		must first pend on sem_produced to be post by the producer task.
//		Posts sem_consumed to indicate that another item can now be produced.
 //*************************************
 void TaskConsume(int *p_arg)
 {
	 int j, item;
	 INT8U err;
	 int task_num = *(int *)p_arg;
	 while(1)
	 {
		 #if VERSION <= 6
			item = RetrieveItem0(task_num);
		 #else
			item = RetrieveItem1(task_num);
		 #endif
		 CS(printf("TaskConsumer%d:\t ticks=%d Consumed Producer%d's item, total items_in_buffer %d \n", task_num, OSTimeGet(), item, count ));
		 OSTimeDlyHMSM(0,0,0,500);  // 0.5 secs
	 }
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
	int i,j=0;
    p_arg = p_arg;  // removes compiler warning of unused p_arg

#if OS_TASK_STAT_EN > 0
    OSStatInit();                                /* Determine CPU capacity                                                     */
#endif
   	CS(printf("StartTask:\t %s VERSION %d\n", __FILENAME__, VERSION));
	InitBuffer();

	CS(printf("StartTask:\t ticks=%d Created sem_full_places, sem_empty_places\n", OSTimeGet()));
	
	for(i=0; i<NUM_CONSUMERS; i++)
	{
		err = OSTaskCreateExt(TaskConsume,
                    (void *)&i,
                    (OS_STK *)&Task2Stk[i][TASK_STK_SIZE-1],
                    TASK_CONSUME_PRIO-i,
                    TASK_CONSUME_PRIO-i,
                    (OS_STK *)&Task2Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		Perror(err,"ERROR - OSTaskCreate(TaskConsume ..) failed"); // checks err and exits if an error printing message - see app.c
		OSTimeDly(1);  
	}
	for(i=0; i<NUM_PRODUCERS; i++)
	{
		CS(printf("StartTask:\t ticks=%d Creating TaskProduce with priority %d\n", OSTimeGet(), TASK_PRODUCE_PRIO));
		err = OSTaskCreateExt(TaskProduce,
                    (void *)&i,
                    (OS_STK *)&Task1Stk[i][TASK_STK_SIZE-1],
                    TASK_PRODUCE_PRIO-i,
                    TASK_PRODUCE_PRIO-i,
                    (OS_STK *)&Task1Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
		CS(printf("StartTask:\t ticks=%d Creating TaskConsume with priority %d\n", OSTimeGet(), TASK_CONSUME_PRIO));
		OSTimeDly(1);  
	}
	
		
	OSTimeDlyHMSM(0,0,200,0);   // allow time for consumer and producer to terminate
	CS(printf("\nTerminating ....\n"));
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} //StartTask

