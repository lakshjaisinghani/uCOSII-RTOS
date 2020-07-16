//*         - app.c and example.h are included in all exampleN_*.c projects, N=1..8


/*
*********************************************************************************************************
*                                                CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE    128
#define  TASK_START_PRIO    5

/*
*********************************************************************************************************
*                                            SHARED FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void Perror(int error_code, char *msg);
void Perr(char *msg);

void  StartTask(void *p_arg);

/*
*********************************************************************************************************
*                                            MACROS
*********************************************************************************************************
*/


extern OS_EVENT     *SemMutex;
#define CS(x) 		OSSemPend(SemMutex, 0, &err); x; OSSemPost(SemMutex);

// the following strips path from the FILE macro and leaves the filename with extension
#define __FILENAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : __FILE__ )
