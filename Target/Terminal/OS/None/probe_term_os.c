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
*                                              No Kernel
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

#include  <probe_term.h>
#include  <probe_term_cfg.h>


/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/


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
   *p_err = PROBE_TERM_ERR_NONE;
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

}