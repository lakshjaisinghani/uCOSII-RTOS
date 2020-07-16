#include <stdio.h>
#include "includes.h"
#include "example.h"
#include "IOWR.h"


// Task declarations
#define TASK_STACKSIZE 2048
OS_STK clk_task_stk[TASK_STACKSIZE];
OS_STK disp_task_stk[TASK_STACKSIZE];
OS_STK timeInc_task_stk[TASK_STACKSIZE];
OS_STK switch_alarm_task_stk[TASK_STACKSIZE];
OS_STK switch_mode_task_stk[TASK_STACKSIZE];
OS_STK change_time_task_stk[TASK_STACKSIZE];
OS_STK alarm_task_stk[TASK_STACKSIZE];

// Semaphore declarations
OS_EVENT *dispSem, *incTimeSem, *timeMutexSem, *alarmTimeMutexSem, *modeSem;
OS_EVENT *safeModesem, *alarmOnOffSem, *safeAlarmOnOffsem, *changeTimeSem;
OS_EVENT *AlarmSem;

// Task Priority
#define ALARM_TASK_PRIORITY 12
#define SWITCH_MODE_TASK_PRIORITY 13
#define SWITCH_ALARM_TASK_PRIORITY 14
#define CLK_TASK_PRIORITY 15
#define DISP_TASK_PRIORITY 16
#define TIME_INC_TASK_PRIORITY 17
#define CH_TIME_TASK_PRIORITY 18

// Hex display functions and variables
int alarmOn_hrs_min_hex_disp(int hour, int minute, int alarmOn);
int sec_hex_disp(int second, int minute);
int hex_num[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67 };
int ledgDisp[9] = { 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff };

// Variables modified by task
static int hour = 0;
static int minute = 0;
static int second = 0;
static int al_hour = 0;
static int al_minute = 0;
static int al_second = 0;
static int sec_hold_cnt = 1;
static int min_hold_cnt = 1;
static int mode = 0;          // 0 -> clock, 1 -> alarm
static int alarmOn = 0;       // 0 -> off, 1 -> on
static int beep = 0;          // for toggling alarm 'creative' sequence

// systemInit: 
// Creates all required semaphores
void systemInit(void)
{
	dispSem = OSSemCreate(0); 
	if (dispSem == 0) Perr("Failed to create dispSem semaphore in InitBuffer()");

	incTimeSem = OSSemCreate(0);
	if (incTimeSem == 0) Perr("Failed to create incTimeSem semaphore in InitBuffer()");

	modeSem = OSSemCreate(0);
	if (modeSem == 0) Perr("Failed to create modeSem semaphore in InitBuffer()");

	alarmOnOffSem = OSSemCreate(0);
	if (alarmOnOffSem == 0) Perr("Failed to create modeSem semaphore in InitBuffer()");

	changeTimeSem = OSSemCreate(0);
	if (changeTimeSem == 0) Perr("Failed to create changeTimeSem semaphore in InitBuffer()");

	AlarmSem = OSSemCreate(1);
	if (AlarmSem == 0) Perr("Failed to create noAlarmSem semaphore in InitBuffer()");

	timeMutexSem = OSSemCreate(1);
	if (timeMutexSem == 0) Perr("Failed to create timeMutexSem semaphore in InitBuffer()");

	safeModesem = OSSemCreate(1);
	if (safeModesem == 0) Perr("Failed to create safeModesem semaphore in InitBuffer()");

	safeAlarmOnOffsem = OSSemCreate(1);
	if (safeAlarmOnOffsem == 0) Perr("Failed to create safeAlarmOnOffsem semaphore in InitBuffer()");
}

// incTimeTask: 
// Increments time based on key press
void incTimeTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		OSSemPend(incTimeSem, 0, &err);

		if (sec_hold_cnt <= 60)
		{
			sec_hold_cnt++;
		}
		else
		{
			sec_hold_cnt = 60;
			min_hold_cnt++;

			if (min_hold_cnt >= 60)
			{
				min_hold_cnt = 60;
			}
		}
	}
}

// alarmTask: 
// Triggers alarm
void alarmTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		OSSemPend(AlarmSem, 0, &err);

		if (alarmOn)
		{
			if (al_hour == hour && al_minute == minute && al_second == second)
			{
				// triger alarm
				beep = 1;

				OSTimeDlyHMSM(0, 0, 7, 0);

				beep = 0;
			}
		}

		OSTimeDlyHMSM(0, 0, 0, 100); // to give other tasks time to run
		OSSemPost(AlarmSem);
	}
}

// clockTask: 
// Main time keeper
void clockTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		OSSemPend(timeMutexSem, 0, &err);

		if (mode == 1)
		{
			sec_hold_cnt = 1;
			min_hold_cnt = 1;
		}

		//increase second
		second = second + (1 * sec_hold_cnt);

		//update hour, minute and second
		if (second >= 60) {
			minute += min_hold_cnt;
			second = 0;
		}
		if (minute >= 60) {
			hour += 1;
			minute = 0;
		}
		if (hour >= 24) {
			hour = 0;
			minute = 0;
			second = 0;
		}

		//OS_Printf("Clock Task \n");

		OSSemPost(timeMutexSem);
		OSSemPost(dispSem);
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

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
		mode = !mode;
		OS_Printf("\n Mode: %d \n", mode);
		OSSemPost(safeModesem);
	}
}

// switchAlarmTask: 
// Switches between different alarm ON/OFF 
void switchAlarmTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		OSSemPend(alarmOnOffSem, 0, &err);
		OSSemPend(safeAlarmOnOffsem, 0, &err);
		alarmOn = !alarmOn;
		OS_Printf("\n alarmOn: %d \n", alarmOn);
		OSSemPost(safeAlarmOnOffsem);
	}
}

// timeChangeTask: 
// Task to manually change time 
void timeChangeTask(void* pdata)
{
	INT8U err;

	while (1)
	{
		int hrs, min, sec;
		int pre_hrs, pre_min, pre_sec;

		int once = 0;

		OSSemPend(changeTimeSem, 0, &err);
		OSSemPend(timeMutexSem, 0, &err);

		pre_hrs = hour;
		pre_min = minute;
		pre_sec = second;

		OS_Printf("time change task");

		//ask user input 
		while (once == 0 )
		{
			system("CLS");

			OS_Printf("---------------------------------------------------- \n");
			OS_Printf("------- USER ENTERED MANUAL TIME CHANGE MODE ------- \n");
			OS_Printf("---------------------------------------------------- \n");
			OS_Printf("\n Please remove all letters before adding numbers! \n");

			OS_Printf("\n Please add Hours: ");
			scanf("%d", &hrs);

			OS_Printf("\n Please add Minutes: ");
			scanf("%d", &min);

			OS_Printf("\n Please add Seconds: ");
			scanf("%d", &sec);

			once++;
		}
		
		if (once == 0)
		{
			hrs = pre_hrs;
			min = pre_min;
			sec = pre_sec;
		}

		if (mode == 0)
		{
			hour = hrs;
			minute = min;
			second = sec;
		}
		else
		{
			al_hour = hrs;
			al_minute = min;
			al_second = sec;
		}

		OSSemPost(timeMutexSem);
	}
}

// dispTask: 
// Main display task
void dispTask(void* pdata)
{
	INT8U err;
	int rate = 1;
	int alarmOn_hrs_min_hex, min_sec_hex;
	int col = 0, pulse = 0;

	while (1)
	{
		OSSemPend(dispSem, 0, &err);

		system("CLS");

		col++;

		// display increment speed
		rate = (sec_hold_cnt + min_hold_cnt) / 13;
		IOWR_em(LEDG_BASE_ADDR, 0, ledgDisp[rate]);

		if (mode == 0)
		{
			alarmOn_hrs_min_hex = alarmOn_hrs_min_hex_disp(hour, minute, alarmOn);
			min_sec_hex = sec_hex_disp(second, minute);
		}
		else
		{
			alarmOn_hrs_min_hex = alarmOn_hrs_min_hex_disp(al_hour, al_minute, alarmOn);;
			min_sec_hex = sec_hex_disp(al_second, al_minute);
		}

		// hex 1 - 0: Sec
		// hex 3 - 2: Min
		// hex 5 - 4: Hrs
		IOWR_em(HEX7_4_BASE_ADDR, 0, alarmOn_hrs_min_hex);
		IOWR_em(HEX3_0_BASE_ADDR, 0, min_sec_hex);

		// creative alarm buzz
		if (beep && pulse) IOWR_em(LEDR_BASE_ADDR, 0, 0xfffff);
		else IOWR_em(LEDR_BASE_ADDR, 0, 0x0);
		
		update_led_display(1, beep, col);
		OS_Printf("\n");
		update_hex_display(mode);
		OS_Printf("\n");
		update_led_display(0, beep, 0);
		OS_Printf("\n");

		OS_Printf("   .------------------------------------------------------. \n");
		OS_Printf("   | ----------------- User Interface ------------------- | \n");
		OS_Printf("   | ---------------------------------------------------- | \n");
		OS_Printf("   |  W - Incresase time at faster rate (hold)            | \n");
		OS_Printf("   |  A - Manually Change time                            | \n");
		OS_Printf("   |  S - Set alarm ON/OFF                                | \n");
		OS_Printf("   |  D - Toggle alarm/clock display                      | \n");
		OS_Printf("   | ---------------------------------------------------- | \n");
		OS_Printf("   |  While changing alarm time manually, please enter    | \n");
		OS_Printf("   |  Alrm Mode first, and then set alarm time (A key)    | \n");
		OS_Printf("   | ---------------------------------------------------- | \n");
		OS_Printf("   |  Hex 7/6 - Alarm On/OFF                              | \n");
		OS_Printf("   |  Hex 5/4 - Hours                                     | \n");
		OS_Printf("   |  Hex 3/2 - Minutes                                   | \n");
		OS_Printf("   |  Hex 1/0 - Seconds                                   | \n");
		OS_Printf("   !------------------------------------------------------! \n");

		if (col > 14)
		{
			col = 0;
		}

		pulse = !pulse;
	}
}

// createTasks: 
// Creates all tasks and is called
// in start task
void createTasks(void)
{
	INT8U err;

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

	err = OSTaskCreateExt(dispTask,
		NULL,
		(void*)&disp_task_stk[TASK_STACKSIZE - 1],
		DISP_TASK_PRIORITY,
		DISP_TASK_PRIORITY,
		disp_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(dispTask ..) failed"); 

	err = OSTaskCreateExt(incTimeTask,
		NULL,
		(void*)&timeInc_task_stk[TASK_STACKSIZE - 1],
		TIME_INC_TASK_PRIORITY,
		TIME_INC_TASK_PRIORITY,
		timeInc_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(incTimeTask ..) failed"); 

	err = OSTaskCreateExt(switchModeTask,
		NULL,
		(void*)&switch_mode_task_stk[TASK_STACKSIZE - 1],
		SWITCH_MODE_TASK_PRIORITY,
		SWITCH_MODE_TASK_PRIORITY,
		switch_mode_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(switchModeTask ..) failed"); 

	

	err = OSTaskCreateExt(switchAlarmTask,
		NULL,
		(void*)&switch_alarm_task_stk[TASK_STACKSIZE - 1],
		SWITCH_ALARM_TASK_PRIORITY,
		SWITCH_ALARM_TASK_PRIORITY,
		switch_alarm_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(switchAlarmTask ..) failed"); 


	err = OSTaskCreateExt(timeChangeTask,
		NULL,
		(void*)&change_time_task_stk[TASK_STACKSIZE - 1],
		CH_TIME_TASK_PRIORITY,
		CH_TIME_TASK_PRIORITY,
		change_time_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(timeChangeTask ..) failed"); 

	err = OSTaskCreateExt(alarmTask,
		NULL,
		(void*)&alarm_task_stk[TASK_STACKSIZE - 1],
		ALARM_TASK_PRIORITY,
		ALARM_TASK_PRIORITY,
		alarm_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(alarmTask ..) failed"); 
}

// StartTask:
// Highset priority task
void StartTask(void* p_arg)
{
	allocate_mem();    // Allocate memory for LEDR and LEDG
	OSStatInit();      // initialise OS statistics
	createTasks();     // create all tasks
	systemInit();      // initialise semaphores

	while (1)
	{

		if (GetKeyState('D') & 0x8000) OSSemPost(modeSem); // switch disp mode

		if (GetKeyState('W') & 0x8000) // time incrementation
		{
			OSSemPost(incTimeSem);
		}
		else
		{
			sec_hold_cnt = 1;
			min_hold_cnt = 1;
		}

		if (GetKeyState('A') & 0x8000) OSSemPost(changeTimeSem); // manually change time depending on which mode is selected
		
		if (GetKeyState('S') & 0x8000) OSSemPost(alarmOnOffSem);


		OSTimeDlyHMSM(0, 0, 0, 100) ;
	}
}


// Hex display funtions
int alarmOn_hrs_min_hex_disp(int hour, int minute, int alarmOn)
{
	int dispOn1, dispOn2;

	int hyphen = 0x40;
	int O = 0x5c;

	if (alarmOn)
	{
		dispOn1 = O;
		dispOn2 = 0x54; // n
	}
	else
	{
		dispOn1 = hyphen; 
		dispOn2 = hyphen;
	}


	// split hrs and min
	int hrs_tens = hour / 10;
	int hrs_ones = hour % 10;


	int hex_hrs_tens = hex_num[hrs_tens];
	int hex_hrs_ones = hex_num[hrs_ones];


	long int fin = dispOn1 << 8;
	fin += dispOn2;
	fin = (fin << 8) + hex_hrs_tens;
	fin = (fin << 8) + hex_hrs_ones;

	return fin;
}

int sec_hex_disp(int second, int minutes)
{
	// split hrs and min
	int sec_tens = second / 10;
	int sec_ones = second % 10;
	int min_tens = minutes / 10;
	int min_ones = minutes % 10;

	int hex_tens = hex_num[sec_tens];
	int hex_ones = hex_num[sec_ones];
	long int hex_min_tens = hex_num[min_tens];
	int hex_min_ones = hex_num[min_ones];

	long int fin = hex_min_tens << 8;
	fin += hex_min_ones;
	fin = (fin << 8) + hex_tens;
	fin = (fin << 8) + hex_ones;

	return fin;
}


// End of file