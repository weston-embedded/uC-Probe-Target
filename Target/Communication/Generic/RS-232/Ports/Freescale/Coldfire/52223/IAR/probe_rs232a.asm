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
*                                  Port for the Freescale CF52223
*
* Filename : probe_rs232a.asm
* Version  : V2.30
*********************************************************************************************************
*/

#include  <os_cpu_i.asm>


/*
;*************************************************************************************************
;                                       PUBLIC DECLARATIONS
;*************************************************************************************************
*/

        PUBLIC   ProbeRS232_RxTxISR

/*
;**************************************************************************************************
;                                     EXTERNAL DECLARATIONS
;**************************************************************************************************
*/

        EXTERN   OSIntNesting
        EXTERN   OSIntExit
        EXTERN   OSTCBCur
        EXTERN   ProbeRS232_RxTxISRHandler

        RSEG     CODE:CODE:NOROOT(2)         /* Align to power 2, 4 bytes.                 */

/*
;*******************************************************************************************
;                                     uC/Probe's Rx/Tx ISR
;
; Description : This function services uC/Probe's receive and transmit interrupts
;
; Arguments   : none
;*******************************************************************************************
*/

ProbeRS232_RxTxISR:
        MOVE.W     #0x2700,SR               /* Disable interrupts                         */

        LEA        (-60, A7), A7            /* Save processor registers onto stack        */
        MOVEM.L    D0-D7/A0-A6,(A7)

        OS_EMAC_SAVE                        /* Save the EMAC registers                    */

        MOVEQ.L    #0,D0                    /* OSIntNesting++                             */
        MOVE.B     (OSIntNesting),D0
        ADDQ.L     #1,D0
        MOVE.B     D0,(OSIntNesting)

        CMPI.L     #1, D0                   /* if (OSIntNesting == 1)                     */
        BNE.W       ProbeRS232_RxTxISR_1
        MOVEA.L    (OSTCBCur), A1           /*     OSTCBCur-<OSTCBStkPtr = SP             */
        MOVE.L     A7,(A1)

ProbeRS232_RxTxISR_1:
                                            /* ProbeRS232_RxTxISRHandler()                */
        JSR        (ProbeRS232_RxTxISRHandler)

        JSR        (OSIntExit)              /* Exit the ISR                               */

        OS_EMAC_RESTORE                     /* Restore the EMAC registers                 */

        MOVEM.L    (A7),D0-D7/A0-A6         /* Restore processor registers from stack     */
        LEA        (60, A7), A7

        RTE                                 /* Return to task or nested ISR               */



        END

