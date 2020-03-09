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
*                                        COMMUNICATION: RS-232
*
*                                      PORT FOR FREESCALE MC9S12X
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
#include  <probe_rs232.h>
#include  <probe_com_cfg.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


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


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       ProbeRS232_InitTarget()
*
* Description : Initialize the UART for Probe communication.
*
* Argument(s) : baud_rate   Intended baud rate of the RS-232.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Init().
*
* Note(s)     : (1) The baud rate is computed as the (bus clock) / (2 * 16 * baud), where
*                   the bus clock = cpu clock / 2.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT32U  div;


    div  = BSP_CPU_ClkFreq();                                           /* Get the CPU frq. Note: Divide by 2 to get the BUS Frq    */
    div /= (2 * baud_rate * 16);                                        /* Calculate the baud rate divider, Include Div by 2        */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0CR2 = 0x00;                                                     /* disable the receiver, transmitter and interrupts         */
    SCI0BDH = div >> 8;                                                 /* Calculated Baudrate                                      */
    SCI0BDL = div & 0xFF;
    SCI0CR1 = 0x00;                                                     /* 8 data bits, no parity, 1 stop bit                       */
    SCI0CR2 = 0x0C;                                                     /* Eenable the receiver and transmiter                      */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1CR2 = 0x00;                                                     /* disable the receiver, transmitter and interrupts         */
    SCI1BDH = div >> 8;                                                 /* Calculated Baudrate                                      */
    SCI1BDL = div & 0xFF;
    SCI1CR1 = 0x00;                                                     /* 8 data bits, no parity, 1 stop bit                       */
    SCI1CR2 = 0x0C;                                                     /* Enable the receiver and transmiter                       */
#endif
}


/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description : Handle receive and transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) The receive and transmit interrupts are clearned by reading the status register
*                   and then either reading from or writing to the SCI data register.
*
*               (2) The vector table should plug the correct vector with ProbeRS232_RxISR() or
*                   ProbeRS232_TxISR() for both Rx and Tx SCI interrupts (See probe_rs232_ba.s).
(
*               (3) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h,
*                   the interrupt vector table within vectors.c must be updated as well.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT08U  status;
    CPU_INT08U  data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    status  =  SCI0SR1;                                                 /* Read the status register (part of interrupt clearing)    */
    if ((status & SCI0SR1_RDRF_MASK) == SCI0SR1_RDRF_MASK) {            /* If Receive Interrupt                                     */
        ProbeRS232_RxHandler(SCI0DRL);                                  /* Call the generic Rx handler                              */
    }
    if ((status & SCI0SR1_TC_MASK) == SCI0SR1_TC_MASK) {                /* If Transmit Complete Interrupt                           */
        ProbeRS232_TxHandler();
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    status  =  SCI1SR1;                                                 /* Read the status register (part of interrupt clearing)    */
    if ((status & SCI1SR1_RDRF_MASK) == SCI1SR1_RDRF_MASK) {            /* If Receive Interrupt                                     */
        ProbeRS232_RxHandler(SCI1DRL);                                  /* Call the generic Rx handler                              */
    } else {
        if ((status & SCI1SR1_TC_MASK) == SCI1SR1_TC_MASK) {            /* If Transmit Complete Interrupt                           */
            ProbeRS232_TxHandler();
        }
    }
#endif
}



/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntDis()
*
* Description : Disable receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to stop communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0CR2 &= ~SCI0SR1_RDRF_MASK;                                      /* Disable SCI Rx Interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1CR2 &= ~SCI1SR1_RDRF_MASK;                                      /* Disable SCI Rx Interrupts                                */
#endif
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntEn()
*
* Description : Enable receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to start communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0CR2 |=  SCI0SR1_RDRF_MASK;                                      /* Enable SCI Rx Interrupts                                 */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1CR2 |=  SCI1SR1_RDRF_MASK;                                      /* Enable SCI Rx Interrupts                                 */
#endif
}


/*
*********************************************************************************************************
*                                          ProbeRS232_Tx1()
*
* Description : Transmit one byte.
*
* Argument(s) : c           The byte to transmit.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0DRL = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1DRL = c;
#endif
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntDis()
*
* Description : Disable transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0CR2 &= ~SCI0SR1_TC_MASK;                                        /* disable tx interrupts                                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1CR2 &= ~SCI1SR1_TC_MASK;                                        /* disable tx interrupts                                    */
#endif
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntEn()
*
* Description : Enable transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    SCI0CR2 |=  SCI0SR1_TC_MASK;                                        /* enable tx interrupts                                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1CR2 |=  SCI1SR1_TC_MASK;                                        /* enable tx interrupts                                     */
#endif
}
