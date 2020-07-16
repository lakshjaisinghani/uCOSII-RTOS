
#include <stdio.h>
#include "includes.h"
#include "IOWR.h"
#include "example.h"
#include <string.h>
#include <math.h>

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      6

int hrs_min_hex_disp(int hour, int minute);
int sec_hex_disp(int second, int minute);


int hex_num[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};


void clockTask(void* pdata)
{
	int rotate = 0xff;
	int rotate_ctr = 0;
    int hour, minute, second;

	hour   = 23;
	minute = 59;
	second = 40;


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
		
		IOWR_em(HEX7_4_BASE_ADDR, 0, hrs_min_hex);
		IOWR_em(HEX3_0_BASE_ADDR, 0, min_sec_hex);

		update_hex_display();
        OSTimeDlyHMSM(0, 0, 1, 0);   //wait till 1 second
    }

}

/* The main function creates two task and starts multi-tasking */
void StartTask(void* p_arg)
{
	allocate_mem(); // Allocate memory for LEDR and LEDG

	INT8U  err;
	err = OSTaskCreateExt(clockTask,
		NULL,
		(void*)&task1_stk[TASK_STACKSIZE - 1],
		TASK1_PRIORITY,
		TASK1_PRIORITY,
		task1_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(clockTask ..) failed"); // checks err and exits if an error printing message - see app.c


	int ctr = 506; // To watch it overflow for the LEDG
	int value = 0;

	while (1)
	{
		system("CLS");
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
