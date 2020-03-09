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
*                                        COMMUNICATION: TCP/IP
*                                        Micrium uC/OS-II PORT
*
* Filename : probe_tcpip_os.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file is the uC/OS-II layer for the uC/Probe TCP/IP Communication Module.
*
*            (2) Assumes uC/OS-II V2.87+ is included in the project build.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_tcpip.h>
#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_tcpip.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TCPIP_EN == DEF_ENABLED)                     /* See Note #1.                                         */


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

static  OS_STK  ProbeTCPIP_OS_TaskStk[PROBE_TCPIP_CFG_TASK_STK_SIZE];   /* Stack for TCPIP server task.                 */


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  ProbeTCPIP_OS_Task(void *p_arg);                          /* TCPIP server task.                           */


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        ProbeTCPIP_OS_Init()
*
* Description : Initialize the UDP server task for Probe communication.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTCPIP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeTCPIP_OS_Init (void)
{
    INT8U  err;


#if (OS_TASK_CREATE_EXT_EN > 0)
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreateExt( ProbeTCPIP_OS_Task,
                           (void *)0,
                          &ProbeTCPIP_OS_TaskStk[PROBE_TCPIP_CFG_TASK_STK_SIZE - 1],/* Set Top-Of-Stack.                    */
                           PROBE_TCPIP_CFG_TASK_PRIO,
                           PROBE_TCPIP_CFG_TASK_PRIO,
                          &ProbeTCPIP_OS_TaskStk[0],                                /* Set Bottom-Of-Stack.                 */
                           PROBE_TCPIP_CFG_TASK_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #else
    err = OSTaskCreateExt( ProbeTCPIP_OS_Task,
                           (void *)0,
                          &ProbeTCPIP_OS_TaskStk[0],                                /* Set Top-Of-Stack.                    */
                           PROBE_TCPIP_CFG_TASK_PRIO,
                           PROBE_TCPIP_CFG_TASK_PRIO,
                          &ProbeTCPIP_OS_TaskStk[PROBE_TCPIP_CFG_TASK_STK_SIZE - 1],/* Set Bottom-Of-Stack.                 */
                           PROBE_TCPIP_CFG_TASK_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #endif
#else
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreate( ProbeTCPIP_OS_Task,
                        (void *)0,
                       &ProbeTCPIP_OS_TaskStk[PROBE_TCPIP_CFG_TASK_STK_SIZE - 1],
                        PROBE_TCPIP_CFG_TASK_PRIO);
    #else
    err = OSTaskCreate( ProbeTCPIP_OS_Task,
                        (void *)0,
                       &ProbeTCPIP_OS_TaskStk[0],
                        PROBE_TCPIP_CFG_TASK_PRIO);
    #endif
#endif

#if (OS_VERSION < 287)
#if (OS_TASK_NAME_SIZE > 1)
    OSTaskNameSet(PROBE_TCPIP_CFG_TASK_PRIO, (INT8U *)"Probe TCPIP", &err);
#endif
#else
#if (OS_TASK_NAME_EN   > 0)
    OSTaskNameSet(PROBE_TCPIP_CFG_TASK_PRIO, (INT8U *)"Probe TCPIP", &err);
#endif
#endif
}


/*
*********************************************************************************************************
*                                         ProbeTCPIP_OS_Dly()
*
* Description : Delay the UDP server task.
*
* Argument(s) : ms          Number of milliseconds for which the UDP task should be delayed.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTCPIP_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeTCPIP_OS_Dly (CPU_INT16U  ms)
{
    INT32U  dly_ticks;


    dly_ticks = OS_TICKS_PER_SEC * ((INT32U)ms + 500L / OS_TICKS_PER_SEC) / 1000L;
    OSTimeDly(dly_ticks);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        ProbeTCPIP_OS_Task()
*
* Description : Receives and transmits packets.
*
* Argument(s) : p_arg       Argument passed to ProbeTCPIP_OS_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeTCPIP_OS_Task (void *p_arg)
{
   (void)&p_arg;
    ProbeTCPIP_Task();
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
