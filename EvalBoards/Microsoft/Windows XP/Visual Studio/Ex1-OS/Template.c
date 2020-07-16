#include <stdio.h>
#include "includes.h"
#include "example.h"
#include "IOWR.h"


// Task declarations
#define TASK_STACKSIZE 2048
OS_STK a_task[TASK_STACKSIZE];

// Semaphore declarations
OS_EVENT *aSem;

// Task Priority
#define A_TASK_PRIORITY 6

// Variables modified by task
static int an_int = 0;


// switchModeTask: 
// Switches between different modes (alarm/clock)
// for display
void switchModeTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		OSSemPend(modeSem, 0, &err);
		OSSemPend(safeModesem, 0, &err);
		
		OSSemPost(safeModesem);
	}
}

// systemInit: 
// Creates all required semaphores and tasks
void systemInit(void)
{
	INT8U err;

	aSem = OSSemCreate(0);
	

	err = OSTaskCreateExt(clockTask,
		NULL,
		(void*)&clk_task_stk[TASK_STACKSIZE - 1],
		CLK_TASK_PRIORITY,
		CLK_TASK_PRIORITY,
		clk_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(clockTask ..) failed");
}





