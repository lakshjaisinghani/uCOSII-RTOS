/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*                             (c) Copyright 1998-2004, Micrium, Weston, FL
*                                          All Rights Reserved
*
*
*                                            WIN32 Sample Code
*
* File : APP.C
* By   : Eric Shufro
*		: Lindsay Kleeman added critical section and error checking code.
*         - app.c and example.h are included in all exampleN_*.c projects, N=1..8
*********************************************************************************************************
*/

#include <includes.h>
#include "example.h"


/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/
OS_STK        StartTaskStk[TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           FUNCTION Perror - checks for errors and aborts if necessary
*                                                    Perr - aborts with message unconditionally
*********************************************************************************************************
*/
void Perr(char *msg)
{
	OSSchedLock();
	printf("%s\n", msg);
	while (1) printf("Type <ctrl> C to exit\r"); // Burning CPU cycles but not going anywhere :-)
} /* Perr*/

void Perror(int error_code, char *msg)
{
	if (error_code != OS_NO_ERR)
	{
		printf("Error code: %d\n", error_code);
		Perr(msg);
	}
} /*Perror */

/*
*********************************************************************************************************
*                                           Initialise Critical Section Semaphore
*                                              
*********************************************************************************************************
*/

OS_EVENT     *SemMutex;

void InitCriticalSection(void){
	SemMutex = OSSemCreate(1);
	if (SemMutex == 0) 
		Perr("ERROR:\t\t Failed to create SemMutex for Critical Section Macro");
}

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C++ WIN32 code.  
* Arguments   : none
*********************************************************************************************************
*/

void main(int argc, char *argv[])
{
	INT8U  err;


    OSInit();                              /* Initialize "uC/OS-II, The Real-Time Kernel"                                      */
	InitCriticalSection();
    OS_Printf("main:\t\t Creating StartTask with priority %d\n", TASK_START_PRIO);
    err = OSTaskCreateExt(StartTask,  // StartTask() is defined in Example*.c files
                    (void *)0,
                    (OS_STK *)&StartTaskStk[TASK_STK_SIZE-1],
                    TASK_START_PRIO,
                    TASK_START_PRIO,
                    (OS_STK *)&StartTaskStk[0],
                    TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	Perror(err,"ERROR:\t\t OSTaskCreate(StartTask ..) failed");

    OSStart();                             /* Start multitasking (i.e. give control to uC/OS-II)                               */
}
