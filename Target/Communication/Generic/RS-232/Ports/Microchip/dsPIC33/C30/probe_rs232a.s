;
;********************************************************************************************************
;                                       uC/Probe Communication
;
;                    Copyright 2007-2020 Silicon Laboratories Inc. www.silabs.com
;
;                                 SPDX-License-Identifier: APACHE-2.0
;
;               This software is subject to an open source license and is distributed by
;                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
;                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
;
;********************************************************************************************************
;

;
;********************************************************************************************************
;
;                                  dsPIC33 MPLab and C30 uC/Probe Port
;
;
; File : probe_rs232a.s
;********************************************************************************************************
;

;
;********************************************************************************************************
;                                                CONSTANTS
;********************************************************************************************************
;
    .equ     __33FJ256GP710, 1                                          ; Inform the p33FG256GP710 header file that we are using a dsPIC33FJ256GP710

;
;********************************************************************************************************
;                                                INCLUDES
;********************************************************************************************************
;

    .include "p33FJ256GP710.inc"                                        ; Include assembly equates for various CPU registers and bit masks
    .include "os_cpu_util_a.s"                                          ; Include an assembly utility files with macros for saving and restoring the CPU registers

;
;********************************************************************************************************
;                                             LINKER SPECIFICS
;********************************************************************************************************
;

    .text                                                               ; Locate this file in the text region of the build

;
;********************************************************************************************************
;                                          GLOBALS AND EXTERNALS
;********************************************************************************************************
;

    .global __U2RXInterrupt
    .global __U2TXInterrupt

;
;********************************************************************************************************
;                                            U2RXInterrupt
;
; Description : This function is the UART2 Rx Interrupt Service Routine
;
; Notes       : All user interrupts should be defined as shown below.
;********************************************************************************************************
;

__U2RXInterrupt:
    OS_REGS_SAVE                                                        ; 1) Save processor registers

    mov   #_OSIntNesting, w1
    inc.b [w1], [w1]                                                    ; 2) Call OSIntEnter() or increment OSIntNesting

    dec.b _OSIntNesting, wreg                                           ; 3) Check OSIntNesting. if OSIntNesting == 1, then save the stack pointer, otherwise jump to T2_Cont
    bra nz, U2RXCont
    mov _OSTCBCur, w0
    mov w15, [w0]

U2RXCont:
    call _ProbeRS232_RxISRHandler                                       ; 4) Call YOUR ISR Handler (May be a C function). In this case, the OSView Rx ISR Handler
    call _OSIntExit                                                     ; 5) Call OSIntExit() or decrement 1 from OSIntNesting

    OS_REGS_RESTORE                                                     ; 6) Restore registers

    retfie                                                              ; 7) Return from interrupt

;
;********************************************************************************************************
;                                            U2TXInterrupt
;
; Description : This function is the UART2 Tx Interrupt Service Routine
;
; Notes       : All user interrupts should be defined as shown below.
;********************************************************************************************************
;

__U2TXInterrupt:
    OS_REGS_SAVE                                                        ; 1) Save processor registers

    mov   #_OSIntNesting, w1
    inc.b [w1], [w1]                                                    ; 2) Call OSIntEnter() or increment OSIntNesting

    dec.b _OSIntNesting, wreg                                           ; 3) Check OSIntNesting. if OSIntNesting == 1, then save the stack pointer, otherwise jump to T2_Cont
    bra nz, U2TXCont
    mov _OSTCBCur, w0
    mov w15, [w0]

U2TXCont:
    call _ProbeRS232_TxISRHandler                                       ; 4) Call YOUR ISR Handler (May be a C function). In this case, the OSView Tx ISR Handler
    call _OSIntExit                                                     ; 5) Call OSIntExit() or decrement 1 from OSIntNesting

    OS_REGS_RESTORE                                                     ; 6) Restore registers

    retfie                                                              ; 7) Return from interrupt



