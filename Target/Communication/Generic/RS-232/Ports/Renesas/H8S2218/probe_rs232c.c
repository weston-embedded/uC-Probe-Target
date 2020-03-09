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
* Filename : probe_rs232c.c
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
*                                               ENABLE
*
* Note(s) : (1) See 'probe_rs232.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  PROBE_RS232    SCI2

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  ProbeRS232_Config        (void);
void  ProbeRS232_EnSCIF        (void);
void  ProbeRS232_EriISRHandler (void);
void  ProbeRS232_RxISRHandler  (void);
void  ProbeRS232_TxISRHandler  (void);

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      ProbeRS232_InitTarget()
*
* Description : Initialize the UART for uC/Probe communication.
*
* Argument(s) : baud_rate   Intended baud rate of the RS-232.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT08U baud_clk;
    CPU_INT16U brr;


    ProbeRS232_EnSCIF();

    PROBE_RS232.SCR.BIT.TIE   =  0;                                     /* Disable transmit interrupts                      */
    PROBE_RS232.SCR.BIT.TE    =  0;                                     /* Disable transmitting                             */
    PROBE_RS232.SCR.BIT.RIE   =  0;                                     /* Disable receive interrupts                       */
    PROBE_RS232.SCR.BIT.RE    =  0;                                     /* Disable receiving                                */
    PROBE_RS232.TDR           =  0;                                                                    /* Clear error flags                                */
    PROBE_RS232.SSR.BIT.FER   =  0;
    PROBE_RS232.SSR.BIT.PER   =  0;
    PROBE_RS232.SSR.BIT.ORER  =  0;
    PROBE_RS232.SCR.BIT.CKE   =  0;                                     /* Use internal clock, set SCK pin as input pin     */
    PROBE_RS232.SMR.BIT.CA    =  0;                                     /* Set for asynchronous mode                        */
    PROBE_RS232.SMR.BIT.CHR   =  0;                                     /* 8-bit data                                       */
    PROBE_RS232.SMR.BIT._PE   =  0;                                     /* Parity mode disabled                             */
    PROBE_RS232.SMR.BIT.OE    =  0;                                     /* No parity                                        */
    PROBE_RS232.SMR.BIT.STOP  =  0;                                     /* 1 stop bit                                       */
    baud_clk                  =  0;                                     /* Algorithm for finding SCBRR value                */

    do {                                                                /* N = BRR (0-255), B = bit rate, n = baud clock    */
        brr = SP_Brr(baud_rate, baud_clk);                              /* N = Pclk / (64 * 2^(2 * n - 1) * B) - 1          */

        if (brr > 255) {
             baud_clk++;
        }
    } while (brr > 255);

    PROBE_RS232.SMR.BIT.CKS   =  baud_clk;
    PROBE_RS232.BRR           =  brr;
    ProbeRS232_Config();
    BSP_Dly(1);
    PROBE_RS232.SCR.BIT.TE    =  1;                                     /* Enable transmitting                              */
    PROBE_RS232.SCR.BIT.TIE   =  0;                                     /* Enable Transmit Interrupts                       */
    PROBE_RS232.SCR.BIT.RE    =  1;                                     /* Enable receiving                                 */
    PROBE_RS232.SCR.BIT.RIE   =  0;                                     /* Enable Receive Interrupts                        */
}

void  ProbeRS232_EnSCIF (void)
{
    MSTP.CRB.BIT._SCI2 =  0;                                            /* Enable SCIF1 module                              */
}

void  ProbeRS232_Config (void)
{
    P3.DR.BYTE          = 0;
    INTC.IPRJ.BIT._SCI0 = 0;
}

/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description : Handle Rx and Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may be empty if Rx and Tx interrupts are handled in the separate
*                   interrupt handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{

}


/*
*********************************************************************************************************
*                                    ProbeRS232_RxISRHandler()
*
* Description : Handle Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may empty if Rx interrupts are handled by the combined Rx/Tx interrupt
*                   handler, ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    CPU_CHAR rx_data;


    rx_data  = PROBE_RS232.RDR;
    PROBE_RS232.SSR.BIT.RDRF = 0;                                       /* Read then clear the read flag                    */
    (void)PROBE_RS232.SSR.BIT.RDRF;

    ProbeRS232_RxHandler(rx_data);
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntDis()
*
* Description : Disable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
    PROBE_RS232.SCR.BIT.RIE = 0;                                        /* Enable receiving                                 */
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntEn()
*
* Description : Enable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
    PROBE_RS232.SCR.BIT.RIE = 1;                                        /* Enable Transmit Interrupts                       */
}


/*
*********************************************************************************************************
*                                    ProbeRS232_TxISRHandler()
*
* Description : Handle Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may empty if Tx interrupts are handled by the combined Rx/Tx interrupt
*                   handler, ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
    ProbeRS232_TxHandler();
}


/*
*********************************************************************************************************
*                                      ProbeRS232_Tx1()
*
* Description : Transmit one byte.
*
* Argument(s) : c           The byte to transmit.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
    PROBE_RS232.TDR          = c;                                       /* Place character to be transmitted into register  */
    PROBE_RS232.SSR.BIT.TDRE = 0;                                       /* Clear register empty flag                        */
    (void)PROBE_RS232.SSR.BIT.TDRE;
    PROBE_RS232.SSR.BIT.TEND = 0;                                       /* Clear transmission end flag                      */
}

/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntDis()
*
* Description : Disable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
    PROBE_RS232.SCR.BIT.TIE = 0;                                        /* Disable Transmit Interrupts                      */
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntEn()
*
* Description : Enable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
    PROBE_RS232.SCR.BIT.TIE = 1;                                        /* Enable Transmit Interrupts                       */
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

void ProbeRS232_EriISRHandler (void)
{
    if (PROBE_RS232.SSR.BIT.FER  == 1) {
        PROBE_RS232.SSR.BIT.FER   = 0;
        (void)PROBE_RS232.SSR.BIT.FER;
    }
    if (PROBE_RS232.SSR.BIT.ORER == 1) {
        PROBE_RS232.SSR.BIT.ORER  = 0;
        (void)PROBE_RS232.SSR.BIT.ORER;
    }
    if (PROBE_RS232.SSR.BIT.PER  == 1) {
        PROBE_RS232.SSR.BIT.PER   = 0;
        (void)PROBE_RS232.SSR.BIT.PER;
    }
}
#endif
