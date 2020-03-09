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
*                                   Port for the Renesas RX62N
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

#include  <includes.h>
#include  <iodefine.h>

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

#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
#define  PROBE_RS232                          SCI0
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
#define  PROBE_RS232                          SCI1
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
#define  PROBE_RS232                          SCI2
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
#define  PROBE_RS232                          SCI3
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
#define  PROBE_RS232                          SCI4
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
#define  PROBE_RS232                          SCI5
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
#define  PROBE_RS232                          SCI6
#endif


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

static  void        ProbeRS232_CfgGPIO     (void);

static  void        ProbeRS232_En          (void);

static  void        ProbeRS232_CfgIntCtrl  (void);

static  CPU_INT16U  ProbeRS232_GetBRR      (CPU_INT08U  bclk,
                                            CPU_INT32U  baud_rate);


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

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    CPU_INT08U  baud_clk;
    CPU_INT16U  brr;
    CPU_REG32   i;


    ProbeRS232_En();

    PROBE_RS232.SCR.BIT.TIE  = 0;                               /* Disable transmit interrupts                          */
    PROBE_RS232.SCR.BIT.RIE  = 0;                               /* Disable receive interrupts                           */
    PROBE_RS232.SCR.BIT.TE   = 0;                               /* Disable transmitter                                  */
    PROBE_RS232.SCR.BIT.RE   = 0;                               /* Disable receiver                                     */
    PROBE_RS232.SCR.BIT.TEIE = 0;                               /* Disable transmit end interrupts                      */


    ProbeRS232_CfgGPIO();
    ProbeRS232_CfgIntCtrl();

                                                                /* Read each flag before clearing                       */
    PROBE_RS232.SSR.BIT.TEND;
    PROBE_RS232.SSR.BIT.TEND = 0;
    PROBE_RS232.SSR.BIT.PER;
    PROBE_RS232.SSR.BIT.PER  = 0;
    PROBE_RS232.SSR.BIT.FER;
    PROBE_RS232.SSR.BIT.FER  = 0;
    PROBE_RS232.SSR.BIT.ORER;
    PROBE_RS232.SSR.BIT.ORER = 0;

    PROBE_RS232.SCR.BIT.CKE  = 0;                               /* Use internal clock, set SCK pin as input pin         */
    PROBE_RS232.SMR.BIT.CM   = 0;                               /* Set for asynchronous mode                            */
    PROBE_RS232.SMR.BIT.CHR  = 0;                               /* 8-bit data                                           */
    PROBE_RS232.SMR.BIT.PE   = 0;                               /* Parity mode disabled                                 */
    PROBE_RS232.SMR.BIT.PM   = 0;                               /* No parity                                            */
    PROBE_RS232.SMR.BIT.STOP = 0;                               /* 1 stop bit                                           */

    baud_clk = 0;                                               /* Algorithm for finding BRR value                      */
    do {                                                        /* N = BRR (0-255), B = bit rate, n = baud clock        */
        brr = ProbeRS232_GetBRR(baud_clk, baud_rate);           /* N = Pclk / (64 * 2^(2 * n - 1) * B) - 1              */
        if (brr > 255) {
             baud_clk++;
        }
    } while (brr > 255);

    PROBE_RS232.SMR.BIT.CKS = baud_clk;
//    PROBE_RS232.BRR         = brr + 1;
    PROBE_RS232.BRR         = brr;

    for (i = 0; i < 10000; i++);                                /* Wait for at least 1 bit time                         */

    PROBE_RS232.SCR.BIT.TIE = 0;                                /* Enable Transmit Interrupts                           */
    PROBE_RS232.SCR.BIT.RIE = 1;                                /* Enable Receive Interrupts                            */
    PROBE_RS232.SCR.BYTE |= DEF_BIT_05 | DEF_BIT_04;            /* Enable Tx & Rx (cannot be done thru bit-fields).     */
}

/*
*********************************************************************************************************
*                                        ProbeRS232_CfgIntCtrl()
*
* Description : This function will set the interrupt vectors within the interrupts table.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_CfgIntCtrl (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    IR(SCI0, ERI0) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI0, RXI0) = 0;
    IR(SCI0, TXI0) = 0;
    IR(SCI0, TEI0) = 0;

    IPR(SCI0, ERI0) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI0, RXI0) = 3;
    IPR(SCI0, TXI0) = 3;
    IPR(SCI0, TEI0) = 3;
    
    IEN(SCI0, ERI0) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI0, RXI0) = 1;
    IEN(SCI0, TXI0) = 1;
    IEN(SCI0, TEI0) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    IR(SCI1, ERI1) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI1, RXI1) = 0;
    IR(SCI1, TXI1) = 0;
    IR(SCI1, TEI1) = 0;

    IPR(SCI1, ERI1) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI1, RXI1) = 3;
    IPR(SCI1, TXI1) = 3;
    IPR(SCI1, TEI1) = 3;

    IEN(SCI1, ERI1) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI1, RXI1) = 1;
    IEN(SCI1, TXI1) = 1;
    IEN(SCI1, TEI1) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IR(SCI2, ERI2) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI2, RXI2) = 0;
    IR(SCI2, TXI2) = 0;
    IR(SCI2, TEI2) = 0;

    IPR(SCI2, ERI2) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI2, RXI2) = 3;
    IPR(SCI2, TXI2) = 3;
    IPR(SCI2, TEI2) = 3;

    IEN(SCI2, ERI2) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI2, RXI2) = 1;
    IEN(SCI2, TXI2) = 1;
    IEN(SCI2, TEI2) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    IR(SCI3, ERI3) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI3, RXI3) = 0;
    IR(SCI3, TXI3) = 0;
    IR(SCI3, TEI3) = 0;

    IPR(SCI3, ERI3) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI3, RXI3) = 3;
    IPR(SCI3, TXI3) = 3;
    IPR(SCI3, TEI3) = 3;

    IEN(SCI3, ERI3) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI3, RXI3) = 1;
    IEN(SCI3, TXI3) = 1;
    IEN(SCI3, TEI3) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    IR(SCI4, ERI4) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI4, RXI4) = 0;
    IR(SCI4, TXI4) = 0;
    IR(SCI4, TEI4) = 0;

    IPR(SCI4, ERI4) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI4, RXI4) = 3;
    IPR(SCI4, TXI4) = 3;
    IPR(SCI4, TEI4) = 3;

    IEN(SCI4, ERI4) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI4, RXI4) = 1;
    IEN(SCI4, TXI4) = 1;
    IEN(SCI4, TEI4) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    IR(SCI5, ERI5) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI5, RXI5) = 0;
    IR(SCI5, TXI5) = 0;
    IR(SCI5, TEI5) = 0;

    IPR(SCI5, ERI5) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI5, RXI5) = 3;
    IPR(SCI5, TXI5) = 3;
    IPR(SCI5, TEI5) = 3;

    IEN(SCI5, ERI5) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI5, RXI5) = 1;
    IEN(SCI5, TXI5) = 1;
    IEN(SCI5, TEI5) = 1;

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    IR(SCI6, ERI6) = 0;                                         /* Clear any pending ISR.                               */
    IR(SCI6, RXI6) = 0;
    IR(SCI6, TXI6) = 0;
    IR(SCI6, TEI6) = 0;

    IPR(SCI6, ERI6) = 3;                                        /* Set interrupt priority.                              */
    IPR(SCI6, RXI6) = 3;
    IPR(SCI6, TXI6) = 3;
    IPR(SCI6, TEI6) = 3;

    IEN(SCI6, ERI6) = 1;                                        /* Enable interrupt source.                             */
    IEN(SCI6, RXI6) = 1;
    IEN(SCI6, TXI6) = 1;
    IEN(SCI6, TEI6) = 1;
#endif
}

/*
*********************************************************************************************************
*                                         ProbeRS232_En()
*
* Description : Enable the module.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_En (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    MSTP(SCI0) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    MSTP(SCI1) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    MSTP(SCI2) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    MSTP(SCI3) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    MSTP(SCI4) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    MSTP(SCI5) = 0;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    MSTP(SCI6) = 0;
#endif
}

/*
*********************************************************************************************************
*                                        ProbeRS232_CfgGPIO()
*
* Description : Configures the necessary port pins.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_CfgGPIO (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    PORT2.ICR.BIT.B0 = 1;                                       /* TxD0                                                 */
    PORT2.ICR.BIT.B1 = 1;                                       /* RxD0                                                 */


#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)

#ifndef PROBE_CFG_RS232_UART_ALT
    IOPORT.PFFSCI.BIT.SCI1S = 0;
    PORT2.ICR.BIT.B6 = 1;                                       /* TxD1-A                                               */
    PORT3.ICR.BIT.B0 = 1;                                       /* RxD1-A                                               */
#else
    IOPORT.PFFSCI.BIT.SCI1S = 1;
    PORTF.ICR.BIT.B0 = 1;                                       /* TxD1-B                                               */
    PORTF.ICR.BIT.B2 = 1;                                       /* RxD1-B                                               */
#endif


#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
#ifndef PROBE_CFG_RS232_UART_ALT
    IOPORT.PFFSCI.BIT.SCI2S = 0;
    PORT1.ICR.BIT.B3 = 1;                                       /* TxD2-A                                               */
    PORT1.ICR.BIT.B2 = 1;                                       /* RxD2-A                                               */
#else
    IOPORT.PFFSCI.BIT.SCI2S = 1;
    PORT5.ICR.BIT.B0 = 1;                                       /* TxD2-B                                               */
    PORT5.ICR.BIT.B2 = 1;                                       /* RxD2-B                                               */
#endif


#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
#ifndef PROBE_CFG_RS232_UART_ALT
    IOPORT.PFFSCI.BIT.SCI3S = 0;
    PORT1.ICR.BIT.B7 = 1;                                       /* TxD3-A                                               */
    PORT1.ICR.BIT.B6 = 1;                                       /* RxD3-A                                               */
#else
    IOPORT.PFFSCI.BIT.SCI3S = 1;
    PORT2.ICR.BIT.B3 = 1;                                       /* TxD3-B                                               */
    PORT2.ICR.BIT.B5 = 1;                                       /* RxD3-B                                               */
#endif


#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    PORTC.ICR.BIT.B3 = 1;                                       /* TxD5                                                 */
    PORTC.ICR.BIT.B2 = 1;                                       /* RxD5                                                 */

#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
#ifndef PROBE_CFG_RS232_UART_ALT
    IOPORT.PFFSCI.BIT.SCI6S = 0;
    PORT0.ICR.BIT.B0 = 1;                                       /* TxD6-A                                               */
    PORT0.ICR.BIT.B1 = 1;                                       /* RxD6-A                                               */
#else
    IOPORT.PFFSCI.BIT.SCI6S = 1;
    PORT3.ICR.BIT.B2 = 1;                                       /* TxD6-B                                               */
    PORT3.ICR.BIT.B3 = 1;                                       /* RxD6-B                                               */
#endif
#endif
}

/*
*********************************************************************************************************
*                                         ProbeRS232_GetBRR()
*
* Description   : This function calculates the BRR for the settings chosen.
*
* Argument      : bclk      The baud rate generator clock source (0-3)
*
*                 baud_rate The baud rate.
*
* Returns       : The calculated BRR.
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeRS232_GetBRR (CPU_INT08U  bclk,
                                       CPU_INT32U  baud_rate)
{
    CPU_INT16U  mult;
    CPU_INT16U  val;

                                                                /* Switch for (64 * 2^(2 * baud_clk - 1)) values.       */
    switch (bclk) {
        case 0:
             mult = 32;
             break;
             
        case 1:
             mult = 128;
             break;
             
        case 2:
             mult = 512;
             break;
             
        case 3:
        default:
             mult = 2048;
             break;
    }

    val =  BSP_CPU_PerClkFreq() / (mult * baud_rate) - 1;

    return (val);
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
    CPU_CHAR  data;


    data = PROBE_RS232.RDR;
    ProbeRS232_RxHandler(data);
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
    PROBE_RS232.SCR.BIT.RIE = 0;                                /* Disable rx int.                                      */
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
    PROBE_RS232.SCR.BIT.RIE = 1;                                /* Enable rx int.                                       */
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
* Argument(s) : data        The byte to transmit.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U  data)
{
    CPU_BOOLEAN  en;
    
    
    en = DEF_FALSE;
    
    if (PROBE_RS232.SCR.BIT.TIE == 0) {                         /* If tx int disabled, recover tx.                      */
        en = DEF_TRUE;
        ProbeRS232_TxIntDis();
    }
    
    PROBE_RS232.TDR = data;                                     /* Place character to be transmitted into register      */
    
    if (en == DEF_TRUE) {
        ProbeRS232_TxIntEn();
    }    
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
    do {
        PROBE_RS232.SCR.BIT.TIE = 0;                            /* Disable tx int.                                      */
    } while (PROBE_RS232.SCR.BIT.TIE > 0);

    while (PROBE_RS232.SSR.BIT.TEND == 0) {                     /* Read transmission end flag until set.                */
        ;
    }
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
    PROBE_RS232.SCR.BIT.TIE = 1;                                /* Enable tx int.                                       */
}

/*
*********************************************************************************************************
*                                      ProbeRS232_ErISRHandler()
*
* Description : Probe receive error handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void ProbeRS232_ErISRHandler (void)
{
    while (PROBE_RS232.SSR.BIT.FER > 0) {                       /* Framing Error Flag                                   */
           PROBE_RS232.SSR.BIT.FER = 0;
    }
    while (PROBE_RS232.SSR.BIT.PER > 0) {                       /* Parity Error Flag                                    */
           PROBE_RS232.SSR.BIT.PER = 0;
    }
    if (PROBE_RS232.SSR.BIT.ORER > 0) {                         /* Overrun Error Flag                                   */
        PROBE_RS232.RDR;
        
        while (PROBE_RS232.SSR.BIT.ORER > 0) {
               PROBE_RS232.SSR.BIT.ORER = 0;
        }
    }
}

#endif
