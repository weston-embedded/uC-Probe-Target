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
*
* Filename : probe_rs232.h
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) The abbreviations RX and TX refer to communication from the target's perspective.
*
*            (2) The abbreviations RD and WR refer to reading data from the target memory and
*                writing data to the target memory, respectively.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_RS232 present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_RS232_PRESENT                                    /* See Note #1.                                         */
#define  PROBE_RS232_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef    PROBE_RS232_MODULE
#define   PROBE_RS232_EXT
#else
#define   PROBE_RS232_EXT  extern
#endif


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232c.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) RS-232 communication is enabled/disabled via the PROBE_COM_CFG_RS232_EN configuration define
*               (see 'probe_com_cfg.h  COMMUNICATION MODULE CONFIGURATION').
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
                                                                    /* ------------------- BYTE COUNTERS ------------------ */
PROBE_RS232_EXT  CPU_INT32U  ProbeRS232_RxCtr;                      /* Number of bytes received.                            */
PROBE_RS232_EXT  CPU_INT32U  ProbeRS232_TxCtr;                      /* Number of bytes transmitted.                         */
#endif


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

CPU_BOOLEAN  ProbeRS232_Init          (CPU_INT32U  baud_rate);      /* Initialize Probe RS-232 Communication Module.    */

void         ProbeRS232_RxHandler     (CPU_INT08U  rx_data);        /* Processor-independent receive handler.           */

void         ProbeRS232_TxHandler     (void);                       /* Processor-independent transmit handler.          */

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
void         ProbeRS232_Task          (void);
#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                  DEFINED IN PORT's probe_rs232c.c
*********************************************************************************************************
*/

void         ProbeRS232_InitTarget    (CPU_INT32U  baud_rate);      /* Initialize the target specifics.                 */

void         ProbeRS232_RxTxISRHandler(void);                       /* Receive/transmit interrupt handler.              */

void         ProbeRS232_RxISRHandler  (void);                       /* Receive          interrupt handler.              */

void         ProbeRS232_TxISRHandler  (void);                       /* Rransmit         interrupt handler.              */


void         ProbeRS232_RxIntEn       (void);                       /* Enable  Rx interrupts.                           */

void         ProbeRS232_RxIntDis      (void);                       /* Disable Rx interrupts.                           */


void         ProbeRS232_TxIntEn       (void);                       /* Enable  Tx interrupts.                           */

void         ProbeRS232_TxIntDis      (void);                       /* Disable Tx interrupts.                           */

void         ProbeRS232_Tx1           (CPU_INT08U  c);              /* Transmit single character.                       */

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                  DEFINED IN OS's probe_rs232_os.c
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
CPU_BOOLEAN  ProbeRS232_OS_Init       (void);

void         ProbeRS232_OS_Pend       (void);

void         ProbeRS232_OS_Post       (void);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  PROBE_RS232_CFG_PARSE_TASK_EN
#error  "PROBE_RS232_CFG_PARSE_TASK_EN         not #define'd in 'probe_com_cfg.h'   "
#error  "                                [MUST be  DEF_TRUE   Packet parsed in task]"
#error  "                                [     ||  DEF_FALSE  Packet parsed in ISR ]"

#elif  ((PROBE_RS232_CFG_PARSE_TASK_EN != DEF_ENABLED ) && \
        (PROBE_RS232_CFG_PARSE_TASK_EN != DEF_DISABLED))
#error  "PROBE_RS232_CFG_PARSE_TASK_EN   illegally #define'd in 'probe_com_cfg.h'   "
#error  "                                [MUST be  DEF_TRUE   Packet parsed in task]"
#error  "                                [     ||  DEF_FALSE  Packet parsed in ISR ]"

#elif   (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)

#ifndef  PROBE_RS232_CFG_TASK_PRIO
#error  "PROBE_RS232_CFG_TASK_PRIO             not #define'd in 'probe_com_cfg.h'  "
#endif

#ifndef  PROBE_RS232_CFG_TASK_STK_SIZE
#error  "PROBE_RS232_CFG_TASK_STK_SIZE         not #define'd in 'probe_com_cfg.h'  "
#endif
#endif



#ifndef  PROBE_RS232_CFG_RX_BUF_SIZE
#error  "PROBE_RS232_CFG_RX_BUF_SIZE           not #define'd in 'probe_com_cfg.h'  "
#error  "                                [MUST be  >= 32   ]                       "
#error  "                                [     &&  <= 65535]                       "

#elif  ((PROBE_RS232_CFG_RX_BUF_SIZE > 65535) || \
        (PROBE_RS232_CFG_RX_BUF_SIZE < 32   ))
#error  "PROBE_RS232_CFG_RX_BUF_SIZE     illegally #define'd in 'probe_com_cfg.h'  "
#error  "                                [MUST be  >= 32   ]                       "
#error  "                                [     &&  <= 65535]                       "
#endif



#ifndef  PROBE_RS232_CFG_TX_BUF_SIZE
#error  "PROBE_RS232_CFG_TX_BUF_SIZE           not #define'd in 'probe_com_cfg.h'  "
#error  "                                [MUST be  >= 32   ]                       "
#error  "                                [     &&  <= 65535]                       "

#elif  ((PROBE_RS232_CFG_TX_BUF_SIZE > 65535) || \
        (PROBE_RS232_CFG_TX_BUF_SIZE < 32   ))
#error  "PROBE_RS232_CFG_TX_BUF_SIZE     illegally #define'd in 'probe_com_cfg.h'  "
#error  "                                [MUST be  >= 32   ]                       "
#error  "                                [     &&  <= 65535]                       "
#endif



#ifndef  PROBE_RS232_CFG_COMM_SEL
#error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'      "
#endif


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif


/*
*********************************************************************************************************
*                                              MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_COM_RS232 module include (see Note #1). */
