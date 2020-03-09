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
*
* Filename : probe_com.h
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file contains code to respond to generic (non protocol-dependent)
*                commands received by the target.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_COM present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_COM_PRESENT                                      /* See Note #1.                                         */
#define  PROBE_COM_PRESENT


/*
*********************************************************************************************************
*                              PROBE COMMUNICATION MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The Probe communication module software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                  V     denotes 'Version' label
*                                  x     denotes major software version revision number
*                                  yy    denotes minor software version revision number
*
*               (b) The Probe communication module software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                  ver   denotes software version number scaled as
*                                        an integer value
*                                  x.yy  denotes software version number
*********************************************************************************************************
*/

#define  PROBE_COM_VERSION         410u                         /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   PROBE_COM_MODULE
#define  PROBE_COM_EXT
#else
#define  PROBE_COM_EXT  extern
#endif


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>

#include  <lib_def.h>
#include  <lib_mem.h>
#include  <lib_str.h>

#include  <probe_com_cfg.h>


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef  void  (*PROBE_COM_TERMINAL_EXEC_FNCT)(CPU_CHAR    *pstr,
                                               CPU_SIZE_T   len);

typedef  void  (*PROBE_COM_TERMINAL_IN_FNCT)  (CPU_CHAR    *pstr,
                                               CPU_SIZE_T   len);


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

                                                                /* --------------------- COUNTERS --------------------- */
#if (PROBE_COM_CFG_STAT_EN    == DEF_ENABLED)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxPktCtr;                   /* Number of packets received                           */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxPktCtr;                   /* Number of packets tranmitted                         */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxSymCtr;                   /* Number of symbols transmitted                        */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxSymByteCtr;               /* Number of symbol bytes transmitted                   */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_ErrPktCtr;                  /* Number of error packets transmitted                  */

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_StrRxCtr;                   /* Number of string characters received.                */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_StrRxOvfErrCtr;             /* Number of string receive overflow errors.            */

PROBE_COM_EXT  CPU_INT32U  ProbeCom_StrTxCtr;                   /* Number of string characters transmitted              */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_StrTxOvfErrCtr;             /* Number of string transmit overflow errors.           */
#endif

#if (PROBE_COM_CFG_WR_REQ_EN  == DEF_ENABLED)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxSymCtr;                   /* Number of symbols received.                          */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxSymByteCtr;               /* Number of symbol bytes received.                     */
#endif
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

CPU_BOOLEAN  ProbeCom_Init                (void);                                   /* Init comm module.                */

CPU_SIZE_T   ProbeCom_ParseRxPkt          (void                          *prx_pkt,  /* Parse rx'd pkt & fmt resp.       */
                                           void                          *ptx_pkt,
                                           CPU_SIZE_T                     rx_pkt_size,
                                           CPU_SIZE_T                     tx_buf_size);

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
CPU_SIZE_T   ProbeCom_StrWr               (CPU_CHAR                      *pstr,
                                           CPU_SIZE_T                     len);

CPU_SIZE_T   ProbeCom_StrRd               (CPU_CHAR                      *pstr,
                                           CPU_SIZE_T                     len);
#endif

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
CPU_SIZE_T   ProbeCom_TerminalOut         (CPU_CHAR                      *pstr,     /* Terminal output.                 */
                                           CPU_SIZE_T                     len);

void         ProbeCom_TerminalExecComplete(void);                                   /* Terminal cmd exec complete.      */

void         ProbeCom_TerminalExecSet     (PROBE_COM_TERMINAL_EXEC_FNCT   handler); /* Set terminal cmd exec fnct.      */

void         ProbeCom_TerminalInSet       (PROBE_COM_TERMINAL_IN_FNCT     handler); /* Set terminal in fnct.            */
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                    DEFINED IN OS's probe_com_os.c
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
CPU_BOOLEAN  ProbeCom_OS_Init             (void);

void         ProbeCom_OS_TerminalOutWait  (void);

void         ProbeCom_OS_TerminalOutSignal(void);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  PROBE_COM_CFG_STAT_EN
#error  "PROBE_COM_CFG_STAT_EN                 not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "

#elif  ((PROBE_COM_CFG_STAT_EN != DEF_ENABLED ) && \
        (PROBE_COM_CFG_STAT_EN != DEF_DISABLED))
#error  "PROBE_COM_CFG_STAT_EN           illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "
#endif



#ifndef  PROBE_COM_CFG_WR_REQ_EN
#error  "PROBE_COM_CFG_WR_REQ_EN               not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "

#elif  ((PROBE_COM_CFG_WR_REQ_EN != DEF_ENABLED ) && \
        (PROBE_COM_CFG_WR_REQ_EN != DEF_DISABLED))
#error  "PROBE_COM_CFG_WR_REQ_EN         illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "
#endif



#ifndef  PROBE_COM_CFG_STR_REQ_EN
#error  "PROBE_COM_CFG_STR_REQ_EN              not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "

#elif  ((PROBE_COM_CFG_STR_REQ_EN != DEF_ENABLED ) && \
        (PROBE_COM_CFG_STR_REQ_EN != DEF_DISABLED))
#error  "PROBE_COM_CFG_STR_REQ_EN        illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "

#elif   (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)

#ifndef  PROBE_COM_CFG_STR_IN_BUF_SIZE
#error  "PROBE_COM_CFG_STR_IN_BUF_SIZE         not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "

#elif  ((PROBE_COM_CFG_STR_IN_BUF_SIZE > 65535) || \
        (PROBE_COM_CFG_STR_IN_BUF_SIZE < 32   ))
#error  "PROBE_COM_CFG_STR_IN_BUF_SIZE   illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "
#endif

#ifndef  PROBE_COM_CFG_STR_OUT_BUF_SIZE
#error  "PROBE_COM_CFG_STR_OUT_BUF_SIZE        not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "

#elif  ((PROBE_COM_CFG_STR_OUT_BUF_SIZE > 65535) || \
        (PROBE_COM_CFG_STR_OUT_BUF_SIZE < 32   ))
#error  "PROBE_COM_CFG_STR_OUT_BUF_SIZE  illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "
#endif

#endif



#ifndef  PROBE_COM_CFG_TERMINAL_REQ_EN
#error  "PROBE_COM_CFG_TERMINAL_REQ_EN         not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "

#elif  ((PROBE_COM_CFG_TERMINAL_REQ_EN != DEF_ENABLED ) && \
        (PROBE_COM_CFG_TERMINAL_REQ_EN != DEF_DISABLED))
#error  "PROBE_COM_CFG_TERMINAL_REQ_EN   illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  DEF_ENABLED ]                 "
#error  "                                [     ||  DEF_DISABLED]                 "
#endif



#ifndef  PROBE_COM_CFG_RX_MAX_SIZE
#error  "PROBE_COM_CFG_RX_MAX_SIZE             not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "

#elif  ((PROBE_COM_CFG_RX_MAX_SIZE > 65535) || \
        (PROBE_COM_CFG_RX_MAX_SIZE < 32   ))
#error  "PROBE_COM_CFG_RX_MAX_SIZE       illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "
#endif




#ifndef  PROBE_COM_CFG_TX_MAX_SIZE
#error  "PROBE_COM_CFG_TX_MAX_SIZE             not #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "

#elif  ((PROBE_COM_CFG_TX_MAX_SIZE > 65535) || \
        (PROBE_COM_CFG_TX_MAX_SIZE < 32   ))
#error  "PROBE_COM_CFG_TX_MAX_SIZE       illegally #define'd in 'probe_com_cfg.h'"
#error  "                                [MUST be  >= 32   ]                     "
#error  "                                [     &&  <= 65535]                     "

#endif


/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_COM module include (see Note #1).       */
