/*
*********************************************************************************************************
*                                       uC/Probe Communication
*
*                    Copyright 2007-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        COMMUNICATION: RS-232
*                                        Micrium uC/OS-II PORT
*
* Filename : probe_rs232_os.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file is the uC/OS-II layer for the uC/Probe RS-232 Communication Module.
*
*            (2) Assumes uC/OS-II V2.87+ is included in the project build.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232.h>
#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_rs232.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
static  OS_STK     ProbeRS232_OS_TaskStk[PROBE_RS232_CFG_TASK_STK_SIZE];/* Probe RS-232 task stack.                         */

#if (OS_SEM_EN > 0)
static  OS_EVENT  *ProbeRS232_OS_Sem;                                   /* Packet receive signal.                           */
#endif

#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
static  void  ProbeRS232_OS_Task(void *p_arg);                          /* Probe RS-232 task.                               */
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED) && (OS_SEM_EN == 0)
#error  "OS_SEM_EN              illegally #define'd in 'os_cfg.h'   "
#error  "                       [MUST be  > 0]                      "
#endif


/*
*********************************************************************************************************
*                                         ProbeRS232_OS_Init()
*
* Description : Create RTOS objects for RS-232 communication.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if OS initialization successful.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : ProbeRS232_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
CPU_BOOLEAN  ProbeRS232_OS_Init (void)
{
    INT8U  err;


    ProbeRS232_OS_Sem = OSSemCreate(0);
    if (ProbeRS232_OS_Sem == (OS_EVENT *)0) {
        return (DEF_FAIL);
    }

#if (OS_VERSION < 287)
#if (OS_EVENT_NAME_SIZE > 1)
    OSEventNameSet(ProbeRS232_OS_Sem, (INT8U *)"Probe RS-232", &err);
#endif
#else
#if (OS_EVENT_NAME_EN   > 0)
    OSEventNameSet(ProbeRS232_OS_Sem, (INT8U *)"Probe RS-232", &err);
#endif
#endif

#if (OS_TASK_CREATE_EXT_EN > 0)
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreateExt( ProbeRS232_OS_Task,
                           (void *)0,
                          &ProbeRS232_OS_TaskStk[PROBE_RS232_CFG_TASK_STK_SIZE - 1],/* Set Top-Of-Stack.                    */
                           PROBE_RS232_CFG_TASK_PRIO,
                           PROBE_RS232_CFG_TASK_PRIO,
                          &ProbeRS232_OS_TaskStk[0],                                /* Set Bottom-Of-Stack.                 */
                           PROBE_RS232_CFG_TASK_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #else
    err = OSTaskCreateExt( ProbeRS232_OS_Task,
                           (void *)0,
                          &ProbeRS232_OS_TaskStk[0],                                /* Set Top-Of-Stack.                    */
                           PROBE_RS232_CFG_TASK_PRIO,
                           PROBE_RS232_CFG_TASK_PRIO,
                          &ProbeRS232_OS_TaskStk[PROBE_RS232_CFG_TASK_STK_SIZE - 1],/* Set Bottom-Of-Stack.                 */
                           PROBE_RS232_TASK_CFG_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #endif
#else
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreate( ProbeRS232_OS_Task,
                        (void *)0,
                       &ProbeRS232_OS_TaskStk[PROBE_RS232_CFG_TASK_STK_SIZE - 1],
                        PROBE_RS232_CFG_TASK_PRIO);
    #else
    err = OSTaskCreate( ProbeRS232_OS_Task,
                        (void *)0,
                       &ProbeRS232_OS_TaskStk[0],
                        PROBE_RS232_CFG_TASK_PRIO);
    #endif
#endif

    if (err != OS_ERR_NONE) {
        return (DEF_FAIL);
    }

#if (OS_VERSION < 287)
#if (OS_TASK_NAME_SIZE > 1)
    OSTaskNameSet(PROBE_RS232_CFG_TASK_PRIO, (INT8U *)"Probe RS232", &err);
#endif
#else
#if (OS_TASK_NAME_EN   > 0)
    OSTaskNameSet(PROBE_RS232_CFG_TASK_PRIO, (INT8U *)"Probe RS232", &err);
#endif
#endif

    return (DEF_OK);
}
#endif


/*
*********************************************************************************************************
*                                       ProbeRS232_OS_Pend()
*
* Description : Wait for a packet to be received.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
void  ProbeRS232_OS_Pend (void)
{
#if (OS_SEM_EN > 0)
    INT8U  err;


    OSSemPend(ProbeRS232_OS_Sem, 0, &err);                          /* Wait for a packet to be received                     */
#endif
}
#endif


/*
*********************************************************************************************************
*                                       ProbeRS232_OS_Post()
*
* Description : Notify a pending task that a packet has been receieved.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
void  ProbeRS232_OS_Post (void)
{
#if (OS_SEM_EN > 0)
    (void)OSSemPost(ProbeRS232_OS_Sem);                             /* A packet has been received                           */
#endif
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ProbeRS232_OS_Task()
*
* Description : Task which waits for packets to be received, formalates responses, and begins transmission.
*
* Argument(s) : p_arg       Argument passed to ProbeRS232_OS_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
static  void  ProbeRS232_OS_Task (void *p_arg)
{
   (void)&p_arg;
    ProbeRS232_Task();
}
#endif


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
