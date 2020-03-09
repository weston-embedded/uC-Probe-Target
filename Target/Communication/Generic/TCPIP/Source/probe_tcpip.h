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
*
* Filename : probe_tcpip.h
* Version  : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_TCPIP present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_TCPIP_PRESENT                                    /* See Note #1.                                         */
#define  PROBE_TCPIP_PRESENT


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef    PROBE_TCPIP_MODULE
#define   PROBE_TCPIP_EXT
#else
#define   PROBE_TCPIP_EXT  extern
#endif


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <net.h>
#include  <net_bsd.h>
#include  <net_util.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) TCP/IP communication is enabled/disabled via the PROBE_COM_CFG_TCPIP_EN configuration define
*               (see 'probe_com_cfg.h  COMMUNICATION MODULE CONFIGURATION').
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TCPIP_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/

#ifndef  PROBE_TCPIP_CFG_SOCK_BLOCK_EN
#define  PROBE_TCPIP_CFG_SOCK_BLOCK_EN           DEF_ENABLED
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  ProbeTCPIP_Init   (void);

void  ProbeTCPIP_Task   (void);


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                  DEFINED IN OS'S  probe_tcpip_os.c
*********************************************************************************************************
*/

void  ProbeTCPIP_OS_Init(void);

void  ProbeTCPIP_OS_Dly (CPU_INT16U  ms);


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  PROBE_TCPIP_CFG_TASK_PRIO
#error  "PROBE_TCPIP_CFG_TASK_PRIO              not #define'd in 'probe_com_cfg.h'"
#endif



#ifndef  PROBE_TCPIP_CFG_TASK_STK_SIZE
#error  "PROBE_TCPIP_CFG_TASK_STK_SIZE          not #define'd in 'probe_com_cfg.h'"
#endif




#ifndef  PROBE_TCPIP_CFG_RX_BUF_SIZE
#error  "PROBE_TCPIP_CFG_RX_BUF_SIZE            not #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 32   ]                     "
#error  "                                 [     &&  <= 65535]                     "

#elif  ((PROBE_TCPIP_CFG_RX_BUF_SIZE > 65535) || \
        (PROBE_TCPIP_CFG_RX_BUF_SIZE < 32   ))
#error  "PROBE_TCPIP_CFG_RX_BUF_SIZE      illegally #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 32   ]                     "
#error  "                                 [     &&  <= 65535]                     "
#endif



#ifndef  PROBE_TCPIP_CFG_TX_BUF_SIZE
#error  "PROBE_TCPIP_CFG_TX_BUF_SIZE            not #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 32   ]                     "
#error  "                                 [     &&  <= 65535]                     "

#elif  ((PROBE_TCPIP_CFG_TX_BUF_SIZE > 65535) || \
        (PROBE_TCPIP_CFG_TX_BUF_SIZE < 32   ))
#error  "PROBE_TCPIP_CFG_TX_BUF_SIZE      illegally #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 32   ]                     "
#error  "                                 [     &&  <= 65535]                     "
#endif



#ifndef  PROBE_TCPIP_CFG_PORT
#error  "PROBE_TCPIP_CFG_PORT                   not #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 1    ]                     "
#error  "                                 [     &&  <= 65535]                     "

#elif  ((PROBE_TCPIP_CFG_PORT > 65535) || \
        (PROBE_TCPIP_CFG_PORT < 1    ))
#error  "PROBE_TCPIP_CFG_PORT             illegally #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be  >= 1    ]                     "
#error  "                                 [     &&  <= 65535]                     "
#endif



#ifndef  PROBE_TCPIP_CFG_SOCK_BLOCK_EN
#error  "PROBE_TCPIP_CFG_SOCK_BLOCK_EN          not #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be DEF_ENABLED ]                  "
#error  "                                 [     || DEF_DISABLED]                  "

#elif  ((PROBE_TCPIP_CFG_SOCK_BLOCK_EN != DEF_ENABLED) && \
        (PROBE_TCPIP_CFG_SOCK_BLOCK_EN != DEF_DISABLED))
#error  "PROBE_TCPIP_CFG_SOCK_BLOCK_EN    illegally #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be DEF_ENABLED ]                  "
#error  "                                 [     || DEF_DISABLED]                  "
#endif



#if     (PROBE_TCPIP_CFG_SOCK_BLOCK_EN == DEF_DISABLED)
#ifndef  PROBE_TCPIP_CFG_SOCK_WAIT_DLY
#error  "PROBE_TCPIP_CFG_SOCK_WAIT_DLY          not #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be > 0]                           "

#elif   (PROBE_TCPIP_CFG_SOCK_WAIT_DLY < 1)
#error  "PROBE_TCPIP_CFG_SOCK_WAIT_DLY    illegally #define'd in 'probe_com_cfg.h'"
#error  "                                 [MUST be > 0]                           "
#endif
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
*                                             MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_TCPIP module include (see Note #1).     */
