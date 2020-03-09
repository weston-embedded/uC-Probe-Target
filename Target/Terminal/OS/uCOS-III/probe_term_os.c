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
*                            UC/PROBE TERMINAL WINDOW OPERATING SYSTEM LAYER
*                                         Micrium uC/OS-III
*
* File    : probe_term_os.c
* Version : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    MICRIUM_SOURCE
#include  <os.h>
#include  <probe_term.h>
#include  <probe_term_cfg.h>


/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
#ifndef  PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE
#error  "PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE      not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                            [MUST be  > 0]                    "
#endif

#ifndef  PROBE_TERM_OS_CFG_CMD_RX_TASK_PRIO
#error  "PROBE_TERM_OS_CFG_CMD_RX_TASK_PRIO          not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                            [MUST be  > 0]                    "
#endif
#ifndef  PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE
#error  "PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE      not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                            [MUST be  > 0]                    "
#endif

#ifndef  PROBE_TERM_OS_CFG_CMD_TX_TASK_PRIO
#error  "PROBE_TERM_OS_CFG_CMD_TX_TASK_PRIO          not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                            [MUST be  > 0]                    "
#endif
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
#ifndef  PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE
#error  "PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE      not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                   [MUST be  > 0]                    "
#endif

#ifndef  PROBE_TERM_OS_CFG_TRC_TASK_PRIO
#error  "PROBE_TERM_OS_CFG_TRC_TASK_PRIO          not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                   [MUST be  > 0]                    "
#endif
#endif

#if (PROBE_TERM_CFG_CMD_EN > 0 || PROBE_TERM_CFG_TRC_EN > 0)
#ifndef  PROBE_TERM_OS_CFG_TASK_DLY_MSEC
#error  "PROBE_TERM_OS_CFG_TASK_DLY_MSEC          not #define'd in 'app_cfg.h or probe_term_cfg.h'"
#error  "                                   [MUST be  > 0 and < 1000]         "
#endif
#endif

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

#if PROBE_TERM_CFG_CMD_EN > 0
static  OS_TCB     ProbeTermOS_CmdRxTaskTCB;
static  CPU_STK    ProbeTermOS_CmdRxTaskStk[PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE];
static  OS_TCB     ProbeTermOS_CmdTxTaskTCB;
static  CPU_STK    ProbeTermOS_CmdTxTaskStk[PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE];
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
static  OS_TCB     ProbeTermOS_TrcTaskTCB;
static  CPU_STK    ProbeTermOS_TrcTaskStk[PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE];
#endif


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
static  void  ProbeTermOS_CmdRxTask (void  *p_arg);
static  void  ProbeTermOS_CmdTxTask (void  *p_arg);
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
static  void  ProbeTermOS_TrcTask (void  *p_arg);
#endif


/*
*********************************************************************************************************
*                                         ProbeTerm_OS_Init()
*
* Description : Initialize the OS layer for the terminal window.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               PROBE_TERM_ERR_NONE           OS initialization successful.
*                               PROBE_TERM_ERR_OS_INIT_FAIL   OS objects NOT successfully initialized.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermInit().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (PROBE_TERM_CFG_CMD_EN > 0 || PROBE_TERM_CFG_TRC_EN > 0)
void  ProbeTermOS_Init (PROBE_TERM_ERR  *p_err)
{
    OS_ERR  err_os;


#if PROBE_TERM_CFG_CMD_EN > 0
    OSTaskCreate(         &ProbeTermOS_CmdRxTaskTCB,
                          "uC/Probe-Term Cmd-Line Rx Task",
                           ProbeTermOS_CmdRxTask,
                  (void *) 0,
                           PROBE_TERM_OS_CFG_CMD_RX_TASK_PRIO,
                          &ProbeTermOS_CmdRxTaskStk[0],
                           PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE / 10u,
                           PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE,
                           0u,
                           0u,
                  (void *) 0,
                           OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                          &err_os);

    if (err_os !=  OS_ERR_NONE) {
       *p_err = PROBE_TERM_ERR_OS_INIT_FAIL;
        return;
    }

    OSTaskCreate(         &ProbeTermOS_CmdTxTaskTCB,
                          "uC/Probe-Term Cmd-Line Tx Task",
                           ProbeTermOS_CmdTxTask,
                  (void *) 0,
                           PROBE_TERM_OS_CFG_CMD_TX_TASK_PRIO,
                          &ProbeTermOS_CmdTxTaskStk[0],
                           PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE / 10u,
                           PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE,
                           0u,
                           0u,
                  (void *) 0,
                           OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                          &err_os);

    if (err_os !=  OS_ERR_NONE) {
       *p_err = PROBE_TERM_ERR_OS_INIT_FAIL;
        return;
    }
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
    OSTaskCreate(         &ProbeTermOS_TrcTaskTCB,
                          "uC/Probe-Term Trace Task",
                           ProbeTermOS_TrcTask,
                  (void *) 0,
                           PROBE_TERM_OS_CFG_TRC_TASK_PRIO,
                          &ProbeTermOS_TrcTaskStk[0],
                           PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE / 10u,
                           PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE,
                           0u,
                           0u,
                  (void *) 0,
                           OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                          &err_os);

    if (err_os !=  OS_ERR_NONE) {
       *p_err = PROBE_TERM_ERR_OS_INIT_FAIL;
        return;
    }
#endif

   *p_err = PROBE_TERM_ERR_NONE;
}
#endif

/*
*********************************************************************************************************
*                                         ProbeTermOS_CmdRxTask()
*
* Description : OS-dependent shell task to process strings for the command line interface responses.
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by uC/OS-III).
*
* Return(s)   : none.
*
* Created by  : PROBE_TERM_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
static  void  ProbeTermOS_CmdRxTask (void *p_arg)
{
    OS_ERR  err;


    p_arg = p_arg;

    while (DEF_ON) {
        OSTimeDlyHMSM(0, 0, 0, PROBE_TERM_OS_CFG_TASK_DLY_MSEC,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);

        ProbeTermCmdRxTaskHandler();
    }
}
#endif


/*
*********************************************************************************************************
*                                         ProbeTermOS_CmdTxTask()
*
* Description : OS-dependent shell task to process strings for the command line interface responses.
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by uC/OS-III).
*
* Return(s)   : none.
*
* Created by  : PROBE_TERM_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
static  void  ProbeTermOS_CmdTxTask (void *p_arg)
{
    OS_ERR  err;


    p_arg = p_arg;

    while (DEF_ON) {
        OSTimeDlyHMSM(0, 0, 0, PROBE_TERM_OS_CFG_TASK_DLY_MSEC,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);

        ProbeTermCmdTxTaskHandler();
    }
}
#endif


/*
*********************************************************************************************************
*                                         ProbeTermOS_TrcTask()
*
* Description : OS-dependent shell task to process strings for tracing purposes.
*
* Argument(s) : p_arg       Pointer to task initialization argument (required by uC/OS-III).
*
* Return(s)   : none.
*
* Created by  : PROBE_TERM_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_TRC_EN > 0
static  void  ProbeTermOS_TrcTask (void *p_arg)
{
    OS_ERR  err;


    p_arg = p_arg;

    while (DEF_ON) {
        OSTimeDlyHMSM(0, 0, 0, PROBE_TERM_OS_CFG_TASK_DLY_MSEC,
                      OS_OPT_TIME_HMSM_STRICT,
                     &err);

        ProbeTermTrcTaskHandler();
    }
}
#endif


/*
*********************************************************************************************************
*                                       ProbeTermOS_Q_CmdMsgRdy()
*
* Description : Signals that a command line response message is ready for processing.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermPrintCmd().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
void  ProbeTermOS_Q_CmdMsgRdy (void)
{
    OS_ERR  err;


    (void)OSTaskSemPost(&ProbeTermOS_CmdTxTaskTCB,
                         OS_OPT_POST_NONE,
                        &err);
}
#endif


/*
*********************************************************************************************************
*                                       ProbeTermOS_Q_TrcMsgRdy()
*
* Description : Signals that a trace message is ready for processing.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermPrintTrc().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_TRC_EN > 0
void  ProbeTermOS_Q_TrcMsgRdy (void)
{
    OS_ERR  err;


    (void)OSTaskSemPost(&ProbeTermOS_TrcTaskTCB,
                         OS_OPT_POST_NONE,
                        &err);
}
#endif


/*
*********************************************************************************************************
*                                     ProbeTermOS_Q_CmdMsgWait()
*
* Description : Waits until a command line response message is available.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermCmdTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeTermOS_Q_CmdMsgWait (void)
{
    OS_ERR  err;


    (void)OSTaskSemPend(           0,
                                   OS_OPT_PEND_BLOCKING,
                        (CPU_TS *) 0,
                                  &err);
}


/*
*********************************************************************************************************
*                                     ProbeTermOS_Q_TrcMsgWait()
*
* Description : Waits until a trace message is available.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermTrcTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeTermOS_Q_TrcMsgWait (void)
{
    OS_ERR  err;


    (void)OSTaskSemPend(           0,
                                   OS_OPT_PEND_BLOCKING,
                        (CPU_TS *) 0,
                                  &err);
}
