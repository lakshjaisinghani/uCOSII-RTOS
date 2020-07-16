#include <stdio.h>
#include "includes.h"
#include "example.h"
#include "IOWR.h"
#include <string.h>
#include <math.h>


/* Definition of Task Stacks */
#define TASK_STACKSIZE 2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
OS_STK clk_task_stk[TASK_STACKSIZE];
OS_STK pwm_task_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK1_PRIORITY 6
#define TASK2_PRIORITY 7
#define CLK_TASK_PRIORITY 8
#define PWM_TASK_PRIORITY 9

#define MAX_BRIGHTNESS 32 // for pwm

int hrs_min_hex_disp(int hour, int minute);
int sec_hex_disp(int second, int minute);
void print_wave(int wave_num, int brightness_val, int on_off);
struct wave_val wave_acc(int flag, int accumulator, int brightness);


int hex_num[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67 };

struct wave_val {
	int acc;
	int flag;
};


/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata)
{
	while (1)
	{
		//printf("Hello from task 1 \n\n");
		//OS_Printf("Task1:\t\t Ticks=%d,\t delaying 3 sec\n", OSTimeGet());

		// uncomment for 3.5b
		int waste_time = 0;
		for (int i = 0; i < 50000; i++)
		{
			for (int j = 0; j < 50000; j++)
			{
				waste_time += i * j;
			}
		}

		OSTimeDlyHMSM(0, 0, 3, 0);
	}
}
/* Prints "Hello World" and sleeps for five seconds */
void task2(void* pdata)
{
	while (1) {
		//printf("Hello from task 2 \n\n");
		OS_Printf("Task2:\t\t Ticks=%d,\t delaying 10 sec\n", OSTimeGet());
		OSTimeDlyHMSM(0, 0, 5, 0);
	}
}

void clockTask(void* pdata)
{
	int rotate = 0xff;
	int rotate_ctr = 0;
	int hour, minute, second;

	hour = 15;
	minute = 31;
	second = 20;


	while (1)
	{
		//increase second
		second++;

		//update hour, minute and second
		if (second == 60) {
			minute += 1;
			second = 0;
		}
		if (minute == 60) {
			hour += 1;
			minute = 0;
		}
		if (hour == 24) {
			hour = 0;
			minute = 0;
			second = 0;
		}

		int hrs_min_hex = hrs_min_hex_disp(hour, minute);
		int min_sec_hex = sec_hex_disp(second, minute);

		// hex 1 - 0: Sec
		// hex 3 - 2: Min
		// hex 5 - 4: Hrs
		IOWR_em(HEX7_4_BASE_ADDR, 0, hrs_min_hex);
		IOWR_em(HEX3_0_BASE_ADDR, 0, min_sec_hex);

		update_hex_display();
		OSTimeDlyHMSM(0, 0, 1, 0);   //wait till 1 second

		/*value = IORD_em(LEDR_BASE_ADDR, 0);
		OS_Printf("Reading from LEDR %d\n", value);*/
		// Update LEDR display
		//update_led_display(1);
		// Update LEDG display
		//update_led_display(0);
	}

}

void pwmTask(void* pdata)
{
	INT8U brightness = 1;
	int accumulator1 = 0;
	int accumulator2 = 3;
	int accumulator3 = 6;
	int accumulator4 = 9;
	int accumulator5 = 12;
	int accumulator6 = 15;
	int accumulator7 = 18;
	int accumulator8 = 21;
	int flag1 = 1;
	int flag2 = 1;
	int flag3 = 1;
	int flag4 = 1;
	int flag5 = 1;
	int flag6 = 1;
	int flag7 = 1;
	int flag8 = 1;

	while (1)
	{
		
		struct wave_val wave1 = wave_acc(flag1, accumulator1, brightness);
		struct wave_val wave2 = wave_acc(flag2, accumulator2, brightness);
		struct wave_val wave3 = wave_acc(flag3, accumulator3, brightness);
		struct wave_val wave4 = wave_acc(flag4, accumulator4, brightness);
		struct wave_val wave5 = wave_acc(flag5, accumulator5, brightness);
		struct wave_val wave6 = wave_acc(flag6, accumulator6, brightness);
		struct wave_val wave7 = wave_acc(flag7, accumulator7, brightness);
		struct wave_val wave8 = wave_acc(flag8, accumulator8, brightness);
		
		flag1 = wave1.flag;
		accumulator1 = wave1.acc;
		flag2 = wave2.flag;
		accumulator2 = wave2.acc;
		flag3 = wave3.flag;
		accumulator3 = wave3.acc;
		flag4 = wave4.flag;
		accumulator4 = wave4.acc;
		flag5 = wave5.flag;
		accumulator5 = wave5.acc;
		flag6 = wave6.flag;
		accumulator6 = wave6.acc;
		flag7 = wave7.flag;
		accumulator7 = wave7.acc;
		flag8 = wave8.flag;
		accumulator8 = wave8.acc;

		if (accumulator1 < 0)
		{
			accumulator1 = 0;
			flag1 = 1;
		}
		if (accumulator2 < 0)
		{
			accumulator2 = 0;
			flag2 = 1;
		}
		if (accumulator3 < 0)
		{
			accumulator3 = 0;
			flag3 = 1;
		}
		if (accumulator4 < 0)
		{
			accumulator4 = 0;
			flag4 = 1;
		}
		if (accumulator5 < 0)
		{
			accumulator5 = 0;
			flag5 = 1;
		}
		if (accumulator6 < 0)
		{
			accumulator6 = 0;
			flag6 = 1;
		}
		if (accumulator7 < 0)
		{
			accumulator7 = 0;
			flag7 = 1;
		}
		if (accumulator8 < 0)
		{
			accumulator8 = 0;
			flag8 = 1;
		}

		system("CLS");
		print_wave(1, accumulator1, 1);
		print_wave(2, accumulator2, 1);
		print_wave(3, accumulator3, 1);
		print_wave(4, accumulator4, 1);
		print_wave(5, accumulator5, 1);
		print_wave(6, accumulator6, 1);
		print_wave(7, accumulator7, 1);
		print_wave(8, accumulator8, 1);

		OSTimeDlyHMSM(0, 0, 0, 150); // for testing
		OSTimeDly(1); // delay a timer tick and give other tasks a chance to run
	}
}

/* The main function creates two task and starts multi-tasking */
void StartTask(void* p_arg)
{
	allocate_mem(); // Allocate memory for LEDR and LEDG


	INT8U err;

	OSStatInit(); // initialise OS statistics

	//err = OSTaskCreateExt(task1,
	//	NULL,
	//	(void*)&task1_stk[TASK_STACKSIZE - 1],
	//	TASK1_PRIORITY,
	//	TASK1_PRIORITY,
	//	task1_stk,
	//	TASK_STACKSIZE,
	//	NULL,
	//	0);

	//Perror(err, "ERROR - OSTaskCreate(Task1 ..) failed"); // checks err and exits if an error occurs

	//err = OSTaskCreateExt(task2,
	//	NULL,
	//	(void*)&task2_stk[TASK_STACKSIZE - 1],
	//	TASK2_PRIORITY,
	//	TASK2_PRIORITY,
	//	task2_stk,
	//	TASK_STACKSIZE,
	//	NULL,
	//	0);

	//Perror(err, "ERROR - OSTaskCreate(Task2 ..) failed"); // checks err and exits if an error occurs

	err = OSTaskCreateExt(clockTask,
		NULL,
		(void*)&clk_task_stk[TASK_STACKSIZE - 1],
		CLK_TASK_PRIORITY,
		CLK_TASK_PRIORITY,
		clk_task_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(clockTask ..) failed"); // checks err and exits if an error printing message - see app.c

	//err = OSTaskCreateExt(pwmTask,
	//	NULL,
	//	(void*)&pwm_task_stk[TASK_STACKSIZE - 1],
	//	PWM_TASK_PRIORITY,
	//	PWM_TASK_PRIORITY,
	//	pwm_task_stk,
	//	TASK_STACKSIZE,
	//	NULL,
	//	0);
	//Perror(err, "ERROR - OSTaskCreate(pwmTask ..) failed"); // checks err and exits if an error printing message - see app.c


	while (1)
	{
		system("CLS");

		// uncomment to see task 1 and 2 playback
		//OS_Printf("Start:\t\t Ticks=%d,\t delaying 3 sec\n", OSTimeGet());
		//OS_Printf("CPU USAGE: %d \n", OSCPUUsage); // display cpu usage
		//OS_Printf("OSIdleCtrMax: %d \n", OSIdleCtrMax); // display OSIdleCtrMax
		//OSTimeDlyHMSM(0, 0, 3, 0);


		// uncomment to play clock or PWM
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

int hrs_min_hex_disp(int hour, int minute)
{
	// split hrs and min
	int hrs_tens = hour / 10;
	int hrs_ones = hour % 10;

	int hex_hrs_tens = hex_num[hrs_tens];
	int hex_hrs_ones = hex_num[hrs_ones];


	long int fin = hex_hrs_tens << 8;
	fin += hex_hrs_ones;

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

void print_wave(int wave_num, int brightness_val, int on_off)
{

	OS_Printf("\nWave %d\t", wave_num);
	OS_Printf("%.*s", MAX_BRIGHTNESS - brightness_val, "________________________________");
	OS_Printf("%.*s", brightness_val, "-----------------------------------");

	if ((MAX_BRIGHTNESS - brightness_val) / (brightness_val+1) >= 0.4)
	{
		OS_Printf("\t %c", 'H');
	}
	else
	{
		OS_Printf("\t %c", 'L');
	}
	
}

struct wave_val wave_acc(int flag, int accumulator, int brightness)
{
	struct wave_val  wave;

	if (flag) // upcount
	{
		accumulator += brightness;
	}
	else
	{
		accumulator -= brightness;
	}

	if (accumulator >= 32)
	{
		flag = 0;
		accumulator--;
	}


	if (flag == 0 && accumulator == 0)
	{
		flag = 1;
	}

	wave.acc = accumulator;
	wave.flag = flag;

	return wave;
}
