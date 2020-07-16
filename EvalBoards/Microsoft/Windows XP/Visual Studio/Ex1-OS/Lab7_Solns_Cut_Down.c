/****************************************************************************
* Made for ECE3073, Lab 7 (Second RTOS Lab) for emulator				    *
* This is to understand semaphores used for signallnig and mutual exclusion *
* Developed by Aaron Choong & Horace Josh (2020)						    *
****************************************************************************/

/*

Documentation:

USED KEY PRESSES:
"D" is to increment the CLOCK/ALARM time 
"A" is to toggle CLOCK/ALARM displays

*/

#include <stdio.h>
#include "includes.h"
#include "IOWR.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    increment_task_stk[TASK_STACKSIZE];
OS_STK    clock_task_stk[TASK_STACKSIZE]; 
OS_STK    alarm_task_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define CLOCK_TASK_PRIORITY		6	// Increments clock every second
#define ALARM_TASK_PRIORITY		7   // For incrementing alarm time 
#define INCREMENT_TASK_PRIORITY 8   // For deciding the increment value



/* Definition of predefined / control variables */

#define REDLEDS 1
#define GREENLEDS 0
#define KEY_SLEEP_TIME 100 // The key will sleep for 100 milliseconds
#define SCREEN_REFRESH_RATE 100 // In terms of milliseconds
#define LEDG_NUM 9 // Number of LEDG bits
#define LEDR_NUM 18 // Number of LEDR bits
#define INCREMENT_TIME 5 // 5 seconds per second


/* Definition of what to print*/
#define DEBUG FALSE // printfs


/* Global variables*/
//IOWR_em(hex_base_address, 0, arr[0]);
int arr[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f }; // For HEX display
int secs = 58, a_secs = 0;
int mins = 59, a_mins = 0;
int hours = 23, a_hours = 0;
char key_pressed;

UINT32 hex3to0_value = 0;
UINT32 hex7to4_value = 0;
UINT8 ones, tens, mins_ones, mins_tens, hours_ones, hours_tens; // For printing to HEX
UINT8 clock_or_alarm = 1; // 1 for clock, 0 for alarm
UINT8 increment_time = 1; // +1 second per second

/* Semaphore instantiation */
OS_EVENT * sem_keypress; // Signalling
OS_EVENT * sem_alarm_increment; // Signalling
OS_EVENT * sem_mutex; // Mutual exclusion


void update_hex_values()
{

	if (clock_or_alarm)
	{
		// To separate the digits
		ones = secs % 10; 
		tens = secs / 10;
		mins_ones = mins % 10;
		mins_tens = mins / 10;
		hours_ones = hours % 10;
		hours_tens = hours / 10;
		hex3to0_value = ((arr[mins_ones] << 24) + (0x00 << 16) + (arr[tens] << 8) + arr[ones]);
		hex7to4_value = ((arr[hours_tens] << 24) + (arr[hours_ones] << 16) + (0x00 << 8) + arr[mins_tens]);
	}
	else
	{
		// To separate the digits
		ones = a_secs % 10;
		tens = a_secs / 10;
		mins_ones = a_mins % 10;
		mins_tens = a_mins / 10;
		hours_ones = a_hours % 10;
		hours_tens = a_hours / 10;
		hex3to0_value = ((arr[mins_ones] << 24) + (0x00 << 16) + (arr[tens] << 8) + arr[ones]);
		hex7to4_value = ((arr[hours_tens] << 24) + (arr[hours_ones] << 16) + (0x00 << 8) + arr[mins_tens]);
	}

	IOWR_em(HEX3_0_BASE_ADDR, 0, hex3to0_value);
	IOWR_em(HEX7_4_BASE_ADDR, 0, hex7to4_value);

	system("cls");
	update_hex_display();
}


void increment_task(void *pdata)
{
	UINT8 err;
	while (1)
	{
		OSSemPend(sem_keypress, 0, &err);

		OSSemPend(sem_mutex, 0, &err);
		increment_time = INCREMENT_TIME;
		if (!clock_or_alarm) OSSemPost(sem_alarm_increment); 
		OSSemPost(sem_mutex);

	}
}


void clock_task(void* pdata)
{

	int old_ctr = 0;
	UINT8 err;

	while (1)
	{
		OSSemPend(sem_mutex, 0, &err); // Protect the global variables accessed by more than one task (secs, mins, hours, increment_time)
		if (secs <= 59)
		{
			clock_or_alarm ? secs += increment_time : secs++;
		}
		
		// To determine secs, mins, hours
		if (secs >= 60)
		{
			secs -= 60;
			if (mins < 59)
				mins++;
			else {
				mins = 0;
				if (hours < 23)
					hours++;
				else {
					hours = 0;
				}
			}
		}
		
		// Updates the display
		update_hex_values();

		OSSemPost(sem_mutex); // Releases the mutually exclusive semaphore
		if (DEBUG) OS_Printf("Clock: %d:%d:%d\n", hours, mins, secs);

		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void alarm_task(void* pdata)
{
	UINT8 err;
	while (1)
	{
		OSSemPend(sem_alarm_increment, 0, &err); // Blocked until signalled to increment

		OSSemPend(sem_mutex, 0, &err); // IMPORTANT!!! Protection semaphores needs to be used after a signalling semaphore, otherwise it could result in a deadlock!!
		if (a_secs <= 59)
			a_secs += increment_time;
		if (a_secs >= 60) {
			a_secs -= 60;
			if (a_mins < 59)
				a_mins++;
			else {
				a_mins = 0;
				if (a_hours < 23)
					a_hours++;
				else {
					a_hours = 0;
				}
			}
		}

		OSSemPost(sem_mutex);
		//OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

/* The main function creates two task and starts multi-tasking */
void StartTask(void *p_arg)
{
	allocate_mem(); // Allocate memory for LEDR, LEDG, and HEX

	INT8U  err;
	err = OSTaskCreateExt(clock_task,
		NULL,
		(void *)&clock_task_stk[TASK_STACKSIZE - 1],
		CLOCK_TASK_PRIORITY,
		CLOCK_TASK_PRIORITY,
		clock_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(clock_task ..) failed");


	err = OSTaskCreateExt(increment_task,
		NULL,
		(void *)&increment_task_stk[TASK_STACKSIZE - 1],
		INCREMENT_TASK_PRIORITY,
		INCREMENT_TASK_PRIORITY,
		increment_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(clock_task ..) failed");

	err = OSTaskCreateExt(alarm_task,
		NULL,
		(void *)&alarm_task_stk[TASK_STACKSIZE - 1],
		ALARM_TASK_PRIORITY,
		ALARM_TASK_PRIORITY,
		alarm_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(alarm_task ..) failed"); 


	sem_keypress = OSSemCreate(0); // Signalling semaphore
	sem_alarm_increment = OSSemCreate(0); // Signalling semaphore
	sem_mutex = OSSemCreate(1); // Protection semaphore

	while (1)
	{

		if(GetKeyState('D') & 0x8000)
		{
			OSSemPost(sem_keypress);
		}
		else
		{
			OSSemPend(sem_mutex, 0, &err);
			increment_time = 1;
			OSSemPost(sem_mutex);
		}

		if(GetKeyState('A') & 0x8000)
		{
			OSSemPend(sem_mutex, 0, &err);
			clock_or_alarm = !clock_or_alarm; // Toggle clock or alarm
			OSSemPost(sem_mutex);
		}

		if (DEBUG) OS_Printf("The key that has been pressed is: %c\n", key_pressed);

		OSTimeDlyHMSM(0, 0, 0, KEY_SLEEP_TIME);
	}


}
