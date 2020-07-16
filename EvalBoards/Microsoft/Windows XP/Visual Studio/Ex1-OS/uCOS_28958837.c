//***************************************//
// Friday 3-6PM Lab Test for ECE3073	 
// Student's Name: Laksh Jaisinghani				  
// Student's ID: 28958837				 
//***************************************//

#include <stdio.h>
#include "includes.h"
#include "IOWR.h"
#include "example.h"

#define   TASK_STACKSIZE       2048
OS_STK    display_hex_task[TASK_STACKSIZE];
OS_STK    update_ledg_task[TASK_STACKSIZE];
OS_STK    update_ledr_task[TASK_STACKSIZE];
OS_STK    display_initial_task[TASK_STACKSIZE];

/* Definition of the Task Priorities */
#define DISPLAY_INITIAL_PRIORITY  6
#define DISPLAY_HEX_PRIORITY      7
#define LEDG_PRIORITY             8
#define LEDR_PRIORITY             9

//define semaphores
OS_EVENT *mutexSem, *incrementLedrSem;
INT8U err;

// shared variables
static int hello[4] = { 0x76, 0x79, 0x38, 0x3f}; // H E L O
static int state = 0;
static int mode  = 0;
static int flag  = 0;
static int ledr  = 0;
static int ledg = 0x1ffff;
static int hex_7_4[4] = { 0x76793838, 0x00767938, 0x00007679, 0x00000076 };
static int hex_3_0[4] = { 0x3f000000, 0x383f0000, 0x38383f00, 0x7938383f };

// function changes display mode
// 0 -> initial state
// 1 -> start state 
void changeMode()
{
	OSSemPend(mutexSem, 0, &err);
	mode = !mode;
	state = 0;
	flag = 0;
	ledr = 0;
	ledg = 0x1ffff;

	OS_Printf("Mode = %d", mode);
	OSSemPost(mutexSem);
}

// initial display task
void DISPLAY_INITIAL_TASK(void* pdata)
{
	while (1)
	{
		if (mode == 0)
		{
			system("CLS");

			OSSemPend(mutexSem, 0, &err);

				// set ledg/r
				IOWR_em(LEDG_BASE_ADDR, 0, 0x1ffff);
				IOWR_em(LEDR_BASE_ADDR, 0, 0x0);

				//set hex in original state
				IOWR_em(HEX7_4_BASE_ADDR, 0, hex_7_4[state]);
				IOWR_em(HEX3_0_BASE_ADDR, 0, hex_3_0[state]);

				update_led_display(1);
				OS_Printf("\n");
				update_hex_display();
				OS_Printf("\n");
				update_led_display(0);
				OS_Printf("\n");

			OSSemPost(mutexSem);
		}

		OSTimeDlyHMSM(0, 0, 0, 200);
	}
}

// main display task
void DISPLAY_HEX_TASK(void* pdata)
{
	int count = 0;

	while (1)
	{
		if (mode == 1)
		{
			system("CLS");

			OSSemPend(mutexSem, 0, &err);

			// increment state
			if (state == 0 && flag == 1) flag = 0;

			if (state < 3 && flag == 0) state++;
			else if (state == 3 && flag == 0) flag = 1;
			else
			{
				if (state > 0 && flag == 1) state--;
				else flag = 0;
			}

			if (state == 0 && count > 0) OSSemPost(incrementLedrSem);
			
			// set ledg/r
			IOWR_em(LEDG_BASE_ADDR, 0, ledg);
			IOWR_em(LEDR_BASE_ADDR, 0, ledr);

			//set hex in original state
			IOWR_em(HEX7_4_BASE_ADDR, 0, hex_7_4[state]);
			IOWR_em(HEX3_0_BASE_ADDR, 0, hex_3_0[state]);

			update_led_display(1);
			OS_Printf("\n");
			update_hex_display();
			OS_Printf("\n");
			update_led_display(0);
			OS_Printf("\n");

			OSSemPost(mutexSem);
		}

		count++;

		OSTimeDlyHMSM(0, 0, 0, 200);
	}
}

// ledg decrement counter
void UP_LEDG_TASK(void* pdata)
{
	while(1)
	{
		if (mode == 1)
		{
			OSSemPend(mutexSem, 0, &err);

				if (ledg == 0)
				{
					ledg = 0x1ffff;
				}

				ledg -= 1;
			OSSemPost(mutexSem);
		}
		
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}

// ledr increment counter
void UP_LEDR_TASK(void* pdata)
{
	while (1)
	{
		OSSemPend(incrementLedrSem, 0, &err);
		OSSemPend(mutexSem, 0, &err);
		if (mode == 1)
		{
			ledr += 4;
		}
		OSSemPost(mutexSem);
	}
}

// systemInit: 
// Creates all required semaphores and tasks
void systemInit(void)
{
	
	// semaphores
	mutexSem = OSSemCreate(1);
	if (mutexSem == 0) Perr("Failed to create mutexSem semaphore");

	incrementLedrSem = OSSemCreate(0);
	if (incrementLedrSem == 0) Perr("Failed to create incrementLedrSem semaphore");

	// tasks
	err = OSTaskCreateExt(DISPLAY_INITIAL_TASK,
		NULL,
		(void*)&display_initial_task[TASK_STACKSIZE - 1],
		DISPLAY_INITIAL_PRIORITY,
		DISPLAY_INITIAL_PRIORITY,
		display_initial_task,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(display_initial_task ..) failed");

	err = OSTaskCreateExt(DISPLAY_HEX_TASK,
		NULL,
		(void*)&display_hex_task[TASK_STACKSIZE - 1],
		DISPLAY_HEX_PRIORITY,
		DISPLAY_HEX_PRIORITY,
		display_hex_task,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(display_hex_task ..) failed");

	err = OSTaskCreateExt(UP_LEDG_TASK,
		NULL,
		(void*)&update_ledg_task[TASK_STACKSIZE - 1],
		LEDG_PRIORITY,
		LEDG_PRIORITY,
		update_ledg_task,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(update_ledg_task ..) failed");

	err = OSTaskCreateExt(UP_LEDR_TASK,
		NULL,
		(void*)&update_ledr_task[TASK_STACKSIZE - 1],
		LEDR_PRIORITY,
		LEDR_PRIORITY,
		update_ledr_task,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(update_ledr_task ..) failed");

}

// main task (highest priority)
void StartTask(void *p_arg)
{
	allocate_mem(); // allocates memory to get base addreses

	systemInit(); // initialises tasks and semaphores

	while (1)
	{
		// key polling
		if (GetKeyState('S') & 0x8000) changeMode(); // switch disp mode
		if (GetKeyState('E') & 0x8000) changeMode(); // switch disp mode
		
		OSTimeDlyHMSM(0, 0, 0, 100);
	}
}