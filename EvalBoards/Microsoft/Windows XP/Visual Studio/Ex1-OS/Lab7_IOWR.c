#include <stdio.h>
#include "includes.h"
#include "IOWR.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      6


void task1(void* pdata)
{
	int rotate = 0xff;
	int rotate_ctr = 0;

	while (1)
	{
		system("cls");
		// Example of writing to the HEX displays
		IOWR_em(HEX3_0_BASE_ADDR, 0, rotate << rotate_ctr++);
		IOWR_em(HEX7_4_BASE_ADDR, 0, 0xff00ff00);
		update_hex_display();

		OSTimeDlyHMSM(0, 0, 1, 0);
	}

}

/* The main function creates two task and starts multi-tasking */
void StartTask(void *p_arg)
{	
	allocate_mem(); // Allocate memory for LEDR and LEDG

	INT8U  err;
	err = OSTaskCreateExt(task1,
		NULL,
		(void *)&task1_stk[TASK_STACKSIZE - 1],
		TASK1_PRIORITY,
		TASK1_PRIORITY,
		task1_stk,
		TASK_STACKSIZE,
		NULL,
		0);
	Perror(err, "ERROR - OSTaskCreate(Task1 ..) failed"); // checks err and exits if an error printing message - see app.c


	int ctr = 506; // To watch it overflow for the LEDG
	int value = 0;

	while (1)
	{
		
		// Example of writing to the LEDRs and LEDGs
		ctr++;
		OS_Printf("Writing to LEDR %d\n", ctr);
		IOWR_em(LEDR_BASE_ADDR, 0, ctr);

		OS_Printf("Writing to LEDG %d\n", ctr);
		IOWR_em(LEDG_BASE_ADDR, 0, ctr);
		OSTimeDlyHMSM(0, 0, 1, 0);

		value = IORD_em(LEDR_BASE_ADDR, 0);
		OS_Printf("Reading from LEDR %d\n", value);

		value = IORD_em(LEDG_BASE_ADDR, 0);
		OS_Printf("Reading from LEDG %d\n", value);

		// Update LEDR display
		update_led_display(1);

		// Update LEDG display
		update_led_display(0);
		OSTimeDlyHMSM(0, 0, 1, 0);
	}


}