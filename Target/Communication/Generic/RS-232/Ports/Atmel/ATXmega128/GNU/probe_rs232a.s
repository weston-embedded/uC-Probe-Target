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
;                                              uC/Probe
;
;                                      Communication: RS-232
;                                   Port for the Atmel Xmega128
;                                           AVR  GCC PORT
;
; Filename : probe_rs232a.s
; Version  : V2.30
;********************************************************************************************************
;

#include  <os_cpu_i.h>
#include  <app_cfg.h>
#include  <os_cfg.h>

        .global  ProbeRS232_RxISR
        .global  ProbeRS232_TxISR

        .extern  OSIntExit
        .extern  OSIntNesting
        .extern  OSTCBCur

        .extern  ProbeRS232_RxISRHandler
        .extern  ProbeRS232_TxISRHandler

        
        .text

;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

        

;/*$PAGE*/
;**********************************************************************************************
;*                                  uC/Probe COM Rx ISR
;*                                 void ProbeRS232_RxISR(void)
;*
;* Description: This function is invoked when a character is received from the serial port
;*
;* Arguments  : none
;*
;* Note(s)    : 1) Pseudo code:
;*                 Disable Interrupts
;*                 Save all registers
;*                 OSIntNesting++
;*                 if (OSIntNesting == 1) {
;*                     OSTCBCur->OSTCBStkPtr = SP
;*                 }
;*                 ProbeRS232_RxHandler();
;*                 OSIntExit();
;*                 Restore all registers
;*                 Return from interrupt;
;**********************************************************************************************
        
ProbeRS232_RxISR:
		//CLI        
        PUSH_ALL                                                ; Save all registers and status register        

        LDS     R16,OSIntNesting                                ; Notify uC/OS-II of ISR
        INC     R16                                             ;
        STS     OSIntNesting,R16                                ;

        CPI     R16,1                                           ; if (OSIntNesting == 1) {
        BRNE    ProbeRS232_RxISR_1

        SAVE_SP				                                    ; X = SP 		
		LDS     R28,OSTCBCur                                    ; OSTCBCur->OSTCBStkPtr = X
        LDS     R29,OSTCBCur+1                                  ;    
        
		ST      Y+,R26
        ST      Y+,R27                                          ; }


ProbeRS232_RxISR_1:
        CALL    ProbeRS232_RxISRHandler                         ; Call tick Handler written in C

        CALL    OSIntExit                                       ; Notify uC/OS-II about end of ISR

        LDS     R26,OSTCBCur                                    ;     OSTCBCur->OSTCBStkPtr = X
        LDS     R27,OSTCBCur+1                                  ;                         X = Y = SP
        
        POP_ALL                                                 ; Restore all registers
		//SEI        
		RETI

;/*$PAGE*/
;**********************************************************************************************
;*                                  uC/Probe COM Tx ISR
;*                                 void ProbeRS232_TxISR(void)
;*
;* Description: This function is invoked when a character has been transmitted to the serial port
;*
;* Arguments  : none
;*
;* Note(s)    : 1) Pseudo code:
;*                 Disable Interrupts
;*                 Save all registers
;*                 OSIntNesting++
;*                 if (OSIntNesting == 1) {
;*                     OSTCBCur->OSTCBStkPtr = SP
;*                 }
;*                 ProbeRS232_TxHandler();
;*                 OSIntExit();
;*                 Restore all registers
;*                 Return from interrupt;
;**********************************************************************************************

ProbeRS232_TxISR:
		//CLI        
        PUSH_ALL                                                ; Save all registers and status register      

        LDS     R16,OSIntNesting                                ; Notify uC/OS-II of ISR
        INC     R16                                             ;
        STS     OSIntNesting,R16                                ;

        CPI     R16,1                                           ; if (OSIntNesting == 1) {
        BRNE    ProbeRS232_TxISR_1

        SAVE_SP				                                    ; X = SP 		
		LDS     R28,OSTCBCur                                    ; OSTCBCur->OSTCBStkPtr = X
        LDS     R29,OSTCBCur+1                                  ;    
        
		ST      Y+,R26
        ST      Y+,R27                                          ; }


ProbeRS232_TxISR_1:
        CALL    ProbeRS232_TxISRHandler                         ; Call tick Handler written in C

        CALL    OSIntExit                                       ; Notify uC/OS-II about end of ISR

	    POP_ALL                                                 ; Restore all registers
		//SEI		
        RETI


;$PAGE
;********************************************************************************************************
;                                     OS_CPU ASSEMBLY PORT FILE END
;********************************************************************************************************

