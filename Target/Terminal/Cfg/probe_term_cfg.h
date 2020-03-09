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
*                             UC/PROBE TERMINAL WINDOW CONFIGURATION FILE
*
*                                              TEMPLATE
*
* File    : probe_term_cfg.h
* Version : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This uC/Probe terminal window configuration header file is protected from multiple
*               pre-processor inclusion through the use of the pre-processor macro definition
*               PROBE_TERM_CFG_MODULE_PRESENT.
*********************************************************************************************************
*/

#ifndef  PROBE_TERM_CFG_MODULE_PRESENT                          /* See Note #1.                                         */
#define  PROBE_TERM_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                             UC/PROBE TERMINAL WINDOW GENERIC CONFIGURATION
*********************************************************************************************************
*/
                                                                /* ------------- COMMAND LINE INTERFACE --------------- */
#define  PROBE_TERM_CFG_CMD_EN                        1         /* Interface enable.                                    */
#define  PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE     128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_CMD_RX_TASK_PRIO          10         /* OS task priority.                                    */
#define  PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE     128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_CMD_TX_TASK_PRIO          11         /* OS task priority.                                    */
                                                                /* ----------------- TRACE INTERFACE ------------------ */
#define  PROBE_TERM_CFG_TRC_EN                        1         /* Interface enable.                                    */
#define  PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE        128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_TRC_TASK_PRIO             12         /* OS task priority.                                    */

#define  PROBE_TERM_CFG_BUF_SIZE                     64         /* Max size of the Rx and Tx message arrays.            */
#define  PROBE_TERM_CFG_Q_SIZE                       16         /* Max number of message arrays in the queues.          */

#define  PROBE_TERM_OS_CFG_TASK_DLY_MSEC            100         /* OS task delay in milliseconds to yield the CPU.      */


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
