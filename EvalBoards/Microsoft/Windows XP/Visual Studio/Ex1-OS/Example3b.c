//*****************************************************************************************************
// FILE: 
//		Example3b.c
// PURPOSE:
//		Example of shared memory problem. Task1 always increments a shared variable, Task2 always 
//		increases the value of a shared variable. Task3 does unrelated work. Why does adding extra
//		versions of Task3 cause the value in the shared variable to sometimes decrease?
// VERSIONS:
//		Version number = number of instances of task3
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

#define TASK1_PRIO TASK_START_PRIO+10
#define TASK2_PRIO TASK_START_PRIO+1
OS_STK        Task1Stk[8][TASK_STK_SIZE];

#define VERSION 5  // 1 no critical section, 2: separately protects inc and dec

#if VERSION == 1
	#define  CS(x) x; // removes Critical Section code
#endif

static int shared_int=0;


// *********************** Procedures ******************
void proc2(int *temp)
{
	int *local;
	long int a=0, i;
	int extra_val;
	extra_val = (int)(rand() % 5);
	//extra_val = 5;

	local = temp;
	for (i=0;i<2000000L; i++)
		a++;
	
	*temp += extra_val;
}

void proc3(void)
{
	int local;
	long int a=0, i;
	for (i=0;i<5000000L; i++)
		a++;
}


//*****************************************************************************************************
// TASK:	
//		Task1,2,3
// DESCRIPTION:
//		Task 1 increments a variable
//		Task 2 takes a while before adding between 1 and 5 to a variable
// 		Task 3 performs unrelated work
//*****************************************************************************************************
void  Task1 (void *p_arg){
	INT8U  err;
	register int temp;
	p_arg = p_arg;
    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{   
		temp = shared_int;
		temp++;
		shared_int = temp;
		OSTimeDly(2);  
    }
} //Task1

void  Task2 (void *p_arg){
	INT8U  err;
	int i;
	long int a=0;
	int temp;
	    p_arg = p_arg;
    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{   
		temp  = shared_int;
		/* Do some work with temp. This procedure 
		returns a value in the range temp+1 to temp+5 */
		proc2(&temp);
		shared_int = temp;
		OSTimeDly(1);  
    }
} //Task2

void  Task3 (void *p_arg){
	INT8U  err;
	p_arg = p_arg;
    while (1)                                 /* Task body, always written as an infinite loop.                             */
	{   
		/* This procedure performs unrelated work */
		proc3();
		OSTimeDly(1);  
    }


} //Task3

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
                    (OS_STK *)&Task1Stk[0][TASK_STK_SIZE-1],
                    TASK1_PRIO,
                    TASK1_PRIO,
                    (OS_STK *)&Task1Stk[0][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task1");

	
	err = OSTaskCreateExt(Task2,
                    (void *)0,
                    (OS_STK *)&Task1Stk[1][TASK_STK_SIZE-1],
                    TASK1_PRIO+1,
                    TASK1_PRIO+1,
                    (OS_STK *)&Task1Stk[1][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err, "ERROR: Failed to create task Task2");
	
	for(i=0; i<VERSION; i++)
	{ 
		err = OSTaskCreateExt(Task3,
                    (void *)0,
                    (OS_STK *)&Task1Stk[i+2][TASK_STK_SIZE - 1],
                    TASK2_PRIO+i+1,
                    TASK2_PRIO+i+1,
                    (OS_STK *)&Task1Stk[i+2][0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
		Perror(err, "ERROR: Failed to create task Task3");
		OSTimeDly(1);
	}
	

	for (i = 0; i<200; i++)                               /* Task body, always written as an infinite loop.                             */
	{       		
		OS_Printf("StartTask:\t Ticks=%d,  shared_int = %d\n", OSTimeGet(), shared_int);  
		OSTimeDly(1);
    }
	OS_Printf("\nTerminating ....\n");
	OSTaskDel(OS_PRIO_SELF); /* terminate this task - tasks never return! */
} //StartTask

