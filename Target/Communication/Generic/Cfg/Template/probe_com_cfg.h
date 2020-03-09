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
*                                    COMMUNICATION: CONFIGURATION
*
* Filename : probe_com_cfg.h
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file contains configuration constants for uC/Probe Communication Modules.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                 COMMUNICATION METHOD CONFIGURATION
*********************************************************************************************************
*/

#define  PROBE_COM_CFG_RS232_EN                    DEF_ENABLED  /* Configure RS-232 communication availability.         */
#define  PROBE_COM_CFG_TCPIP_EN                    DEF_ENABLED  /* Configure TCP/IP communication availability.         */
#define  PROBE_COM_CFG_USB_EN                      DEF_ENABLED  /* Configure USB    communication availability.         */


/*
*********************************************************************************************************
*                                 GENERAL COMMUNICATION CONFIGURATION
*
* Note(s) : (1) Configure PROBE_COM_CFG_RX_MAX_SIZE & PROBE_COM_CFG_TX_MAX_SIZE to the maximum receive
*               & transmit packet sizes, respectively.  Larger maximum packet sizes will result in more
*               efficient communication with better throughput.  The largest receive & transmit packets
*               will be of comparable size, typically.
*
*           (2) Configure PROBE_COM_CFG_WR_REQ_EN to enable/disable write request availability.  If
*               disabled, code to handle write requests (which allowing the Windows application to
*               write target memory locations) will not be included.
*
*           (3) Configure PROBE_COM_CFG_STR_REQ_EN to enable/disable string request availability.  If
*               disabled, code to handle string requests will not be included, and the string read/write
*               interface functions will not be available.
*
*               (a) Configure PROBE_COM_CFG_STR_IN_BUF_SIZE to the length of the string input buffer, in
*                   bytes.
*
*               (b) Configure PROBE_COM_CFG_STR_OUT_BUF_SIZE to the length of the string output buffer,
*                   in bytes.
*
*           (4) Configure PROBE_COM_CFG_TERMINAL_REQ_EN to enable/disable terminal request availability.
*               If disabled, code to handle terminal requests will not be compiled, and the terminal
*               interface functions will not be available.
*
*           (5) Configure PROBE_COM_CFG_STAT_EN to enable disable statistics/counters.
*********************************************************************************************************
*/

#define  PROBE_COM_CFG_RX_MAX_SIZE                       256    /* Configure maximum receive  packet size (see Note #1).*/
#define  PROBE_COM_CFG_TX_MAX_SIZE                       256    /* Configure maximum transmit packet size (see Note #1).*/

#define  PROBE_COM_CFG_WR_REQ_EN                   DEF_ENABLED  /* Configure write request availability (see Note #2).  */

#define  PROBE_COM_CFG_STR_REQ_EN                  DEF_ENABLED  /* Configure string request availability (see Note #3). */
#define  PROBE_COM_CFG_STR_IN_BUF_SIZE                   128    /* Configure size of string input  buf   (see Note #3a).*/
#define  PROBE_COM_CFG_STR_OUT_BUF_SIZE                 2048    /* Configure size of string output buf   (see Note #3b).*/

#define  PROBE_COM_CFG_TERMINAL_REQ_EN             DEF_ENABLED  /* Configure terminal request availability ...          */
                                                                /* ... (see Note #4).                                   */

#define  PROBE_COM_CFG_STAT_EN                     DEF_ENABLED  /* Configure statistics/counters (see Note #5).         */


/*
*********************************************************************************************************
*                                 RS-232 COMMUNICATION CONFIGURATION
*
* Note(s) : (1) Configure PROBE_RS232_CFG_RX_BUF_SIZE & PROBE_RS232_CFG_TX_BUF_SIZE to the receive &
*               transmit buffer sizes, respectively.  These are, effectively, the sizes of the maximum
*               receivable & transmittable packets, so should generally be configured to
*               PROBE_COM_CFG_RX_MAX_SIZE & PROBE_COM_CFG_TX_MAX_SIZE, respectively.
*
*           (2) Configure PROBE_RS232_CFG_PARSE_TASK_EN to enable/disable parsing of received packets at
*               task level.  If enabled, a task will be created to parse received packets.  If disabled,
*               packets will be parsed at interrupt-level.
*
*               (a) Configure PROBE_RS232_CFG_TASK_PRIO to the priority of the parse task.
*
*               (b) Configure PROBE_RS232_CFG_TASK_STK_SIZE to the stack size of the parse task.
*
*           (3) Configure PROBE_RS232_CFG_COMM_SEL to the UART or serial communication interface selected.
*********************************************************************************************************
*/

#define  PROBE_RS232_CFG_RX_BUF_SIZE  PROBE_COM_CFG_RX_MAX_SIZE /*  Configure receive  buffer size (see Note #1).       */
#define  PROBE_RS232_CFG_TX_BUF_SIZE  PROBE_COM_CFG_TX_MAX_SIZE /*  Configure transmit buffer size (see Note #1).       */

#define  PROBE_RS232_CFG_PARSE_TASK_EN             DEF_ENABLED  /*  Configure parse task presence   (see Note #2).      */
#define  PROBE_RS232_CFG_TASK_PRIO                        10    /*  Configure parse task priority   (see Note #2a).     */
#define  PROBE_RS232_CFG_TASK_STK_SIZE                   128    /*  Configure parse task stack size (see Note #2b).     */

#define  PROBE_RS232_UART_0                                1
#define  PROBE_RS232_UART_1                                2
#define  PROBE_RS232_UART_2                                3
#define  PROBE_RS232_UART_3                                4
#define  PROBE_RS232_UART_4                                5
#define  PROBE_RS232_UART_5                                6
#define  PROBE_RS232_UART_6                                7
#define  PROBE_RS232_UART_7                                8
#define  PROBE_RS232_UART_8                                9
#define  PROBE_RS232_UART_9                               10
#define  PROBE_RS232_UART_DBG                             63
#define  PROBE_RS232_CFG_COMM_SEL         PROBE_RS232_UART_2    /* Configure UART selection (see Note #3).              */


/*
*********************************************************************************************************
*                                 TCP/IP COMMUNICATION CONFIGURATION
*
* Note(s) : (1) Configure PROBE_TCPIP_CFG_RX_BUF_SIZE & PROBE_TCPIP_CFG_TX_BUF_SIZE to the receive &
*               transmit buffer sizes, respectively.  These are, effectively, the sizes of the maximum
*               receivable & transmittable packets, so should generally be configured to
*               PROBE_COM_CFG_RX_MAX_SIZE & PROBE_COM_CFG_TX_MAX_SIZE, respectively.
*
*           (2) (a) Configure PROBE_TCPIP_CFG_TASK_PRIO to the priority of the server task.
*
*               (b) Configure PROBE_TCPIP_CFG_TASK_STK_SIZE to the stack size of the server task.
*
*           (3) Configure PROBE_TCPIP_CFG_PORT to the port on which the server listens for packets.
*********************************************************************************************************
*/

#define  PROBE_TCPIP_CFG_RX_BUF_SIZE  PROBE_COM_CFG_RX_MAX_SIZE /*  Configure receive  buffer size (see Note #1).       */
#define  PROBE_TCPIP_CFG_TX_BUF_SIZE  PROBE_COM_CFG_TX_MAX_SIZE /*  Configure transmit buffer size (see Note #1).       */

#define  PROBE_TCPIP_CFG_TASK_PRIO                        11    /*  Configure parse task priority   (see Note #2a).     */
#define  PROBE_TCPIP_CFG_TASK_STK_SIZE                   512    /*  Configure parse task stack size (see Note #2b).     */

#define  PROBE_TCPIP_CFG_PORT                           9930    /* Configure listening port of server (see Note #3).    */

/*
*********************************************************************************************************
*                                  USB COMMUNICATION CONFIGURATION
*
* Note(s) : (1) Configure PROBE_USB_CFG_RX_BUF_SIZE & PROBE_USB_CFG_TX_BUF_SIZE to the receive &
*               transmit buffer sizes, respectively.  These are, effectively, the sizes of the maximum
*               receivable & transmittable packets, so should generally be configured to
*               PROBE_COM_CFG_RX_MAX_SIZE & PROBE_COM_CFG_TX_MAX_SIZE, respectively.
*
*           (2) (a) Configure PROBE_USB_CFG_TASK_PRIO to the priority of the server task.
*
*               (b) Configure PROBE_USB_CFG_TASK_STK_SIZE to the stack size of the server task.
*********************************************************************************************************
*/


#define  PROBE_USB_CFG_RX_BUF_SIZE    PROBE_COM_CFG_RX_MAX_SIZE /*  (a) Set Rx buffer size.                             */
#define  PROBE_USB_CFG_TX_BUF_SIZE    PROBE_COM_CFG_TX_MAX_SIZE /*  (b) Set Tx buffer size.                             */

#define  PROBE_USB_CFG_TX_TIMEOUT_MS                    100u
#define  PROBE_USB_CFG_RX_TIMEOUT_MS                    100u

#define  PROBE_USB_CFG_TASK_PRIO                          12    /*  (c) Set task priority.                              */
#define  PROBE_USB_CFG_TASK_STK_SIZE                     256    /*  (d) Set task stack size.                            */

#define  PROBE_USB_CFG_INIT_STACK                  DEF_TRUE     /*  (e) The USB comm module will init stack.            */
