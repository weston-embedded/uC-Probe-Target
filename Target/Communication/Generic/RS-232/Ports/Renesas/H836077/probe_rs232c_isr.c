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
*                                              uC/Probe
*
*                                      Communication: RS-232
*                                    H8SX/1664  Specific code
*
*
* Filename : probe_rs232_isr.c
* Version  : V2.30
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "includes.h"

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  ProbeRS232_EriISR (void);
void  ProbeRS232_RxISR  (void);
void  ProbeRS232_TxISR  (void);

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)

#pragma asm
    .IMPORT   _OSIntExit
    .IMPORT   _OSTCBCur
    .IMPORT   _OSIntNesting
    .IMPORT   _ProbeRS232_EriISRHandler
    .IMPORT   _ProbeRS232_RxISRHandler
    .IMPORT   _ProbeRS232_TxISRHandler
#pragma endasm

/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/

#pragma asm
          .MACRO   PUSHALL
          PUSH.L    ER0
          PUSH.L    ER1
          PUSH.L    ER2
          PUSH.L    ER3
          PUSH.L    ER4
          PUSH.L    ER5
          PUSH.L    ER6
          .ENDM

          .MACRO   POPALL
          POP.L     ER6
          POP.L     ER5
          POP.L     ER4
          POP.L     ER3
          POP.L     ER2
          POP.L     ER1
          POP.L     ER0
          .ENDM
#pragma endasm

/*
*********************************************************************************************************
*                                    ProbeRS232_RxISR()
*
* Description : Handle Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
*********************************************************************************************************
*/

#pragma noregsave ProbeRS232_RxISR
#if PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_1
#pragma interrupt (ProbeRS232_RxISR(vect=89))
#endif

void ProbeRS232_RxISR (void)
{
#pragma asm
          PUSHALL

_ProbeRS232_RxISR1:
          MOV.B    @_OSIntNesting, R6L
          INC.B    R6L
          MOV.B    R6L, @_OSIntNesting
          CMP.B    #1,R6L
          BNE      _ProbeRS232_RxISR1_1

          MOV.L    @_OSTCBCur, ER6
          MOV.L    ER7, @ER6

_ProbeRS232_RxISR1_1:
          JSR      @_ProbeRS232_RxISRHandler
          JSR      @_OSIntExit

          POPALL
#pragma endasm
}

/*
*********************************************************************************************************
*                                    ProbeRS232_TxISR()
*
* Description : Handle Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
*********************************************************************************************************
*/

#pragma noregsave ProbeRS232_TxISR
#if PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_1
#pragma interrupt (ProbeRS232_TxISR(vect=90))
#endif

void ProbeRS232_TxISR (void)
{
#pragma asm
          PUSHALL

_ProbeRS232_TxISR1:
          MOV.B    @_OSIntNesting, R6L
          INC.B    R6L
          MOV.B    R6L, @_OSIntNesting
          CMP.B    #1,R6L
          BNE      _ProbeRS232_TxISR1_1

          MOV.L    @_OSTCBCur, ER6
          MOV.L    ER7, @ER6

_ProbeRS232_TxISR1_1:
          JSR      @_ProbeRS232_TxISRHandler
          JSR      @_OSIntExit

          POPALL
#pragma endasm
}

/*
*********************************************************************************************************
*                                    ProbeRS232_EriISR()
*
* Description : Handle Eri interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
*********************************************************************************************************
*/

#pragma noregsave ProbeRS232_EriISR
#if PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_1
#pragma interrupt (ProbeRS232_EriISR(vect=88))
#endif

void ProbeRS232_EriISR (void)
{
#pragma asm
          PUSHALL

_ProbeRS232_EriISR1:
          MOV.B    @_OSIntNesting, R6L
          INC.B    R6L
          MOV.B    R6L, @_OSIntNesting
          CMP.B    #1,R6L
          BNE      _ProbeRS232_EriISR1_1

          MOV.L    @_OSTCBCur, ER6
          MOV.L    ER7, @ER6

_ProbeRS232_EriISR1_1:
          JSR      @_ProbeRS232_EriISRHandler
          JSR      @_OSIntExit

          POPALL
#pragma endasm
}

#endif