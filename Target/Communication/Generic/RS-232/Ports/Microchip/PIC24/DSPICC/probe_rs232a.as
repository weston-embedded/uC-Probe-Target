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
;                                           Microchip dsPIC33
;                                           MPLAB and DSPICC
;                                            uC/Probe Port
;
;
; File : probe_rs232a.as
;********************************************************************************************************
;

;
;********************************************************************************************************
;                                                INCLUDES
;********************************************************************************************************
;

    #include <asdspic.h>                                                ; Include assembly equates for various CPU registers and bit masks
    #include "os_cpu_util_a.as"                   						; Include an assembly utility files with macros for saving and restoring the CPU registers

;
;********************************************************************************************************
;                                          GLOBALS AND EXTERNALS
;********************************************************************************************************
;

    GLOBAL     _ProbeRS232_RxISRHandler
    GLOBAL     _ProbeRS232_TxISRHandler

;
;********************************************************************************************************
;                                             LINKER / ASSEMBLER SPECIFICS
;********************************************************************************************************
;

    psect	vectors,class=VECTORS,delta=2
    ORG     04Ah		                                                ;vector 38 is at address 0x50, use address-6 because "vector" psect starts at 0x6 and the ORG is relative to this.
    DDW    __U2RXInterrupt	                                            ;put down the address of the ISR routine

    ORG     04Ch		                                                ;vector 39 is at address 0x52, use address-6 because "vector" psect starts at 0x6 and the ORG is relative to this.
    DDW    __U2TXInterrupt	                                            ;put down the address of the ISR routine

    psect   text,global,reloc=4,class=CODE,delta=2                      ; Place the following ISR code in the 'text' section within memory
    psect   text

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



