//*****************************************************************************************************
// FILE: 
//		ExamplePhilosophers.c
// PURPOSE:
//		Deadlock with philosophers
// VERSIONS:
//		1: Classic Deadlock
//		2: One philosopher gets time to grab chiostick (no deadlock, assuming no preempts by other tasks: 
//		may still reach deadlock if other higher priority tasks upset this timing based solution)
//		3: Philosophers randomly grab left and right chopstick. May or may not result in deadlock 
//		(probability increases with fewer philosophers)
//		4: Solution by giving chopstick priorities
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

#define VERSION 4

#define TASK_PHIL_PRIO  TASK_START_PRIO+1
#define num_philosophers 6

// Shared variables ************************************************************************************
OS_STK		Task1Stk[num_philosophers][TASK_STK_SIZE];
OS_EVENT	*chopsticks[num_philosophers+1];

// Functions with some time delays ******************************
void eat(int delay)
{
	long int a=0, i,j;
	for (i=0;i<3; i++)
	{
		OS_Printf("Philosopher %d: Eating...\n", delay);
		for (j=0;j<5000L; j++)
			a++;
		OSTimeDly(num_philosophers+1-delay);
	}
}

// **** time delay
void think(int delay)
{
	long int a=0, i,j;
	for (i=0;i<5; i++)
	{
		OS_Printf("Philosopher %d: Thinking...\n", delay);
		for (j=0;j<5000000L; j++)
			a++;
		OSTimeDly(num_philosophers+1-delay);
	}
}

//*************************************
// TASK: Philosopher
// DESCRIPTION:
//		Grab two chopsticks then eat
 //*************************************
void Philosopher(int *p_arg)
 {
	 int task_num = *(p_arg+0);
	 INT8U err;
	 OS_EVENT *left_chopstick, *right_chopstick;
	 left_chopstick = chopsticks[*(p_arg+1)];
	 right_chopstick = chopsticks[*(p_arg+2)];
	
	 while (1)                                 /* Task body, always written as an infinite loop.  */
	{  
		OS_Printf("Philosopher %d: Thinking until a chopstick is free\n", task_num);
	
		OSSemPend(left_chopstick,0,&err);
			OS_Printf("Philosopher %d: Got the left chopstick!\n", task_num);
			think(task_num);
			OSSemPend(right_chopstick,0,&err);
				OS_Printf("Philosopher %d: Got the right chopstick. Num nums!\n", task_num);
					eat(task_num);
				OS_Printf("Philosopher %d: Tasty food. Someone elses turn.!\n", task_num);
			OSSemPost(right_chopstick);
		OSSemPost(left_chopstick);
		OS_Printf("Philosopher %d: Too full to eat, better think!\n", task_num); 
		think(task_num);
		think(task_num);
		think(task_num);
	 }

 } 

 void Philosopher2(int *p_arg)
 {
	 int orig_args[2],chopstick_choice;
	 int task_num = *(p_arg+0);
	 INT8U err;
	 OS_EVENT *left_chopstick, *right_chopstick;

	 orig_args[0] = *(p_arg+1);
	 orig_args[1] = *(p_arg+2);
	
	 while (1)                                 /* Task body, always written as an infinite loop.  */
	{  
		OS_Printf("Philosopher %d: Thinking until a chopstick is free\n", task_num);
		srand (task_num);
		chopstick_choice = (int)(rand() % 10);
		if (chopstick_choice>4)
		{
			left_chopstick = chopsticks[orig_args[0]];
			right_chopstick = chopsticks[orig_args[1]];
		}
		else
		{
			OS_Printf("Choice 2: %d\n", chopstick_choice);
			left_chopstick = chopsticks[orig_args[1]];
			right_chopstick = chopsticks[orig_args[0]];
		}
		
		OSSemPend(left_chopstick,0,&err);
			OS_Printf("Philosopher %d: Got the left chopstick!\n", task_num);
			think(task_num);
			OSSemPend(right_chopstick,0,&err);
				OS_Printf("Philosopher %d: Got the right chopstick. Num nums!\n", task_num);
					eat(task_num);
				OS_Printf("Philosopher %d: Tasty food. Someone elses turn.!\n", task_num);
			OSSemPost(right_chopstick);
		OSSemPost(left_chopstick);
	 }

 } 


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
	int philosopher[num_philosophers],i;
	

    p_arg = p_arg;  // removes compiler warning of unused p_arg

	for(i=0; i<=num_philosophers; i++)
		chopsticks[i] = OSSemCreate(1);

	CS(printf("StartTask:\t ticks=%d Created Philosophers, sem_empty_places\n", OSTimeGet()));
    for(i=0; i<num_philosophers; i++)
	{ 
		philosopher[0] = i;
		#if VERSION < 4
			philosopher[1] = i;
			philosopher[2] = (i+1)%num_philosophers;
		#else 
			if (i==num_philosophers) {
				philosopher[1] = 0;
				philosopher[2] = i;
			}
			else
			{
				philosopher[1] = i;
				philosopher[2] = (i+1);
			}
		#endif

		CS(printf("StartTask:\t Creating Philosopher %d with priority %d\n", i, TASK_PHIL_PRIO+i));
		
		#if VERSION == 1
			err = OSTaskCreateExt(Philosopher,
                    &philosopher,
                    (OS_STK *)&Task1Stk[i][TASK_STK_SIZE-1],
                    TASK_PHIL_PRIO+i,
                    TASK_PHIL_PRIO+i,
                    (OS_STK *)&Task1Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
			Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
		
			OSTimeDly(10);  
		#elif VERSION == 2 
			err = OSTaskCreateExt(Philosopher,
                    &philosopher,
                    (OS_STK *)&Task1Stk[i][TASK_STK_SIZE-1],
                    TASK_PHIL_PRIO+i,
                    TASK_PHIL_PRIO+i,
                    (OS_STK *)&Task1Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
			Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
			if (i==0)
				OSTimeDly(100);
			else
				OSTimeDly(10); 
		#elif VERSION == 3 
			err = OSTaskCreateExt(Philosopher2,
                    &philosopher,
                    (OS_STK *)&Task1Stk[i][TASK_STK_SIZE-1],
                    TASK_PHIL_PRIO+i,
                    TASK_PHIL_PRIO+i,
                    (OS_STK *)&Task1Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
			Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
			OSTimeDly(10); 
		#else 
			err = OSTaskCreateExt(Philosopher,
                    &philosopher,
                    (OS_STK *)&Task1Stk[i][TASK_STK_SIZE-1],
                    TASK_PHIL_PRIO+i,
                    TASK_PHIL_PRIO+i,
                    (OS_STK *)&Task1Stk[i][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
			Perror(err,"ERROR - OSTaskCreate(TaskProduce ..) failed"); // checks err and exits if an error printing message - see app.c
			OSTimeDly(10); 
		#endif
	}

	while (1)                                 /* Task body, always written as an infinite loop.  */
	{       		
		OSTimeDlyHMSM(0, 0, 1, 0);       
	}
} //StartTask

