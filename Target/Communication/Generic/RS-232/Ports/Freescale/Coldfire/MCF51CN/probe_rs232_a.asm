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
;************************************************************************************************
; File : probe_rs232_a.asm
;*************************************************************************************************
;

;
;*************************************************************************************************
;                                       PUBLIC DECLARATIONS
;*************************************************************************************************
;

		.global  _ProbeRS232_RxISR
		.global  _ProbeRS232_TxISR
		.global  _ProbeRS232_ErrISR

;
;**************************************************************************************************
;                                     EXTERNAL DECLARATIONS
;**************************************************************************************************
;

        .extern  _OSIntNesting
        .extern  _OSIntExit
        .extern  _OSTCBCur
        .extern  _ProbeRS232_RxISRHandler
        .extern  _ProbeRS232_TxISRHandler
        .extern  _ProbeRS232_ErrISRHandler                

        .text

;      
;********************************************************************************************************
;                                           uC/Probe Rx ISR
;
; Description : This routine is the uC/Probe Rx interrupt service routine
;
; Arguments   : none
;
; Notes       : 1) All USER interrupts should be modeled EXACTLY like this where the only
;                  line to be modified is the call to your ISR_Handler and perhaps the call to
;                  the label name ProbeRS232_RxISRHandler.
;********************************************************************************************************
;

_ProbeRS232_RxISR:
        MOVE.W     #0x2700,SR               ; Disable interrupts                         
                                            
        LEA        -60(A7),A7               ; Save processor registers onto stack        
        MOVEM.L    D0-D7/A0-A6,(A7)

        MOVEQ.L    #0,D0                    ; OSIntNesting++                             
        MOVE.B     (_OSIntNesting),D0
        ADDQ.L     #1,D0            
        MOVE.B     D0,(_OSIntNesting)

        CMPI.L     #1, D0                   ; if (OSIntNesting == 1)                     
        BNE        _ProbeRS232_RxISR1
        MOVE.L     (_OSTCBCur), A1          ;     OSTCBCur-<OSTCBStkPtr = SP             
        MOVE.L     A7,(A1)

_ProbeRS232_RxISR1:
        JSR        _ProbeRS232_RxISRHandler ; ProbeRS232_RxISRHandler()                        

        JSR        _OSIntExit               ; Exit the ISR                               

        MOVEM.L    (A7),D0-D7/A0-A6         ; Restore processor registers from stack     
        LEA        60(A7),A7

        RTE                                 ; Return to task or nested ISR               
             
;    
;********************************************************************************************************
;                                           uC/Probe Tx ISR
;
; Description : This routine is the uC/Probe Tx interrupt service routine
;
; Arguments   : none
;
; Notes       : 1) All USER interrupts should be modeled EXACTLY like this where the only
;                  line to be modified is the call to your ISR_Handler and perhaps the call to
;                  the label name ProbeRS232_TxISRHandler.
;********************************************************************************************************
;

_ProbeRS232_TxISR:
        MOVE.W     #0x2700,SR               ; Disable interrupts                         
                                            
        LEA        -60(A7),A7               ; Save processor registers onto stack        
        MOVEM.L    D0-D7/A0-A6,(A7)

        MOVEQ.L    #0,D0                    ; OSIntNesting++                             
        MOVE.B     (_OSIntNesting),D0
        ADDQ.L     #1,D0            
        MOVE.B     D0,(_OSIntNesting)

        CMPI.L     #1, D0                   ; if (OSIntNesting == 1)                     
        BNE        _ProbeRS232_TxISR1
        MOVE.L     (_OSTCBCur), A1          ;     OSTCBCur-<OSTCBStkPtr = SP             
        MOVE.L     A7,(A1)

_ProbeRS232_TxISR1:
        JSR        _ProbeRS232_TxISRHandler ; ProbeRS232_TxISRHandler()                        

        JSR        _OSIntExit               ; Exit the ISR                               

        MOVEM.L    (A7),D0-D7/A0-A6         ; Restore processor registers from stack     
        LEA        60(A7),A7

        RTE                                 ; Return to task or nested ISR  

;    
;********************************************************************************************************
;                                           uC/Probe SCI Error ISR
;
; Description : This routine is the SCI Error interrupt service routine
;
; Arguments   : none
;
; Notes       : 1) All USER interrupts should be modeled EXACTLY like this where the only
;                  line to be modified is the call to your ISR_Handler and perhaps the call to
;                  the label name ProbeRS232_ErrISRHandler.
;********************************************************************************************************
;

_ProbeRS232_ErrISR:
        MOVE.W     #0x2700,SR               ; Disable interrupts                         
                                            
        LEA        -60(A7),A7               ; Save processor registers onto stack        
        MOVEM.L    D0-D7/A0-A6,(A7)

        MOVEQ.L    #0,D0                    ; OSIntNesting++                             
        MOVE.B     (_OSIntNesting),D0
        ADDQ.L     #1,D0            
        MOVE.B     D0,(_OSIntNesting)

        CMPI.L     #1, D0                   ; if (OSIntNesting == 1)                     
        BNE        _ProbeRS232_ErrISR1
        MOVE.L     (_OSTCBCur), A1          ;     OSTCBCur-<OSTCBStkPtr = SP             
        MOVE.L     A7,(A1)

_ProbeRS232_ErrISR1:
                                            ; ProbeRS232_ErrISRHandler()                        
        JSR        _ProbeRS232_ErrISRHandler 

        JSR        _OSIntExit               ; Exit the ISR                               

        MOVEM.L    (A7),D0-D7/A0-A6         ; Restore processor registers from stack     
        LEA        60(A7),A7

        RTE                                 ; Return to task or nested ISR  


    
