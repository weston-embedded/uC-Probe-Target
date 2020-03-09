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
*                                       COMMUNICATION: GENERIC
*                                        Micrium uC/OS-II PORT
*
* Filename : probe_com_os.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file is the uC/OS-II layer for the uC/Probe Generic Communication Module.
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
#include  <ucos_ii.h>


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

#if ((PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED) && (OS_SEM_EN > 0))
static  OS_EVENT  *ProbeCom_OS_TerminalSem;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if ((PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED) && (OS_SEM_EN == 0))
#error  "OS_SEM_EN              illegally #define'd in 'os_cfg.h'   "
#error  "                       [MUST be  > 0]                      "
#endif


/*
*********************************************************************************************************
*                                         ProbeCom_OS_Init()
*
* Description : Create a semaphore for terminal output completion notification.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if OS initialization successful.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : ProbeCom_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
CPU_BOOLEAN  ProbeCom_OS_Init (void)
{
#if (OS_SEM_EN > 0)
    INT8U  err;


    ProbeCom_OS_TerminalSem = OSSemCreate(0);
    if (ProbeCom_OS_TerminalSem == (OS_EVENT *)0) {
        return (DEF_FAIL);
    }

#if (OS_VERSION < 287)
#if (OS_EVENT_NAME_SIZE > 1)
    OSEventNameSet(ProbeCom_OS_TerminalSem, (INT8U *)"Probe Term", &err);
#endif
#else
#if (OS_EVENT_NAME_EN   > 0)
    OSEventNameSet(ProbeCom_OS_TerminalSem, (INT8U *)"Probe Term", &err);
#endif
#endif

    (void)&err;

    return (DEF_OK);
#endif
}
#endif


/*
*********************************************************************************************************
*                                    ProbeCom_OS_TerminalOutWait()
*
* Description : Wait for terminal output to complete.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeCom_TerminalOut().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
void  ProbeCom_OS_TerminalOutWait (void)
{
#if (OS_SEM_EN > 0)
    INT8U  err;


    OSSemPend(ProbeCom_OS_TerminalSem, 0, &err);                /* Wait for output to complete.                         */
#endif
}
#endif


/*
*********************************************************************************************************
*                                   ProbeCom_OS_TerminalOutSignal()
*
* Description : Signal terminal output completion.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeCom_CmdTerminalOut().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
void  ProbeCom_OS_TerminalOutSignal (void)
{
#if (OS_SEM_EN > 0)
    (void)OSSemPost(ProbeCom_OS_TerminalSem);                   /* Output has completed.                                */
#endif
}
#endif
