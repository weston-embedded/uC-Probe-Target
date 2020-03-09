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
*                                    PORT FOR FREESCALE MCF51QE128
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
    CPU_INT32U  bus_freq;
    CPU_INT32U  div;


    bus_freq  =   BSP_CPU_ClkFreq();                                    /* Obtain the CPU clock frequency                           */
    bus_freq /=   2;                                                    /* Convert the CPU clock frequency to the bus clock freq    */
    div       =    bus_freq / (16 * baud_rate);                         /* Compute the baud rate divider (round up)                 */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CPU_VectSet(64 + 12, (void (*)(void))ProbeRS232_ErrISR);            /* Point to the vector for the int handler.                 */
    CPU_VectSet(64 + 13, (void (*)(void))ProbeRS232_RxISR);             /* Point to the vector for the int handler.                 */
    CPU_VectSet(64 + 14, (void (*)(void))ProbeRS232_TxISR);             /* Point to the vector for the int handler.                 */

    SCGC1    |=   SCGC1_SCI1_MASK;                                      /* Enable the SCI1 module clock                             */
    SCI1C2    =   0x00;                                                 /* disable the receiver, transmitter and interrupts         */
    SCI1C1    =   0x00;                                                 /* 8 data bits, no parity, 1 stop bit                       */
    SCI1BDH   =   div >> 8;                                             /* Calculated Baudrate                                      */
    SCI1BDL   =   div & 0xFF;
   (void)SCI1S1;                                                        /* Clear pending interrupt flags                            */
   (void)SCI1D;
    SCI1C2    =   SCI1C2_TE_MASK | SCI1C2_RE_MASK;                      /* Enable the receiver and transmiter                       */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CPU_VectSet(64 + 19, (void (*)(void))ProbeRS232_ErrISR);            /* Point to the vector for the int handler.                 */
    CPU_VectSet(64 + 20, (void (*)(void))ProbeRS232_RxISR);             /* Point to the vector for the int handler.                 */
    CPU_VectSet(64 + 21, (void (*)(void))ProbeRS232_TxISR);             /* Point to the vector for the int handler.                 */

    SCGC1    |=   SCGC1_SCI2_MASK;                                      /* Enable the SCI2 module clock                             */
    SCI2C2    =   0x00;                                                 /* disable the receiver, transmitter and interrupts         */
    SCI2C1    =   0x00;                                                 /* 8 data bits, no parity, 1 stop bit                       */
    SCI2BDH   =   div >> 8;                                             /* Calculated Baudrate                                      */
    SCI2BDL   =   div & 0xFF;
   (void)SCI2S1;                                                        /* Clear pending interrupt flags                            */
   (void)SCI2D;
    SCI2C2    =   SCI2C2_TE_MASK | SCI2C2_RE_MASK;                      /* Enable the receiver and transmiter                       */
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_ErrISRHandler()
*
* Description : Handle error interrupts.
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
*               (2) The vector table should use ProbeRS232_RxTxISR() (See probe_rs232_a.s) for both
*                   receive and transmit SCI interrupts.
*
*               (3) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h,
*                   the interrupt vector table within vectors.c must be updated as well.
*********************************************************************************************************
*/

void  ProbeRS232_ErrISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
   (void)SCI1S1;                                                        /* Read the status register (part of interrupt clearing)    */
   (void)SCI1D;                                                         /* Read the SCI data register (part of interrupt clearing)  */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
   (void)SCI2S1;                                                        /* Read the status register (part of interrupt clearing)    */
   (void)SCI2D;                                                         /* Read the SCI data register (part of interrupt clearing)  */
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
* Note(s)     : (1) This function is empty because receive and transmit interrupts are handled in the
*                   separate interrupt handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
}

/*
*********************************************************************************************************
*                                      ProbeRS232_RxISRHandler()
*
* Description : Handle receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) The receive and transmit interrupts are clearned by reading the status register and
*                   then either reading from or writing to the SCI data register.
*
*               (2) The vector table should use ProbeRS232_RxTxISR() (See probe_rs232_a.asm) for both
*                   receive and transmit SCI interrupts.
*
*               (3) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h,
*                   the interrupt vector table within vectors.c must be updated as well.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
   (void)SCI1S1;                                                        /* Read the status register (part of interrupt clearing)    */
    ProbeRS232_RxHandler(SCI1D);                                        /* Call the generic Rx handler with the received data       */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
   (void)SCI2S1;                                                        /* Read the status register (part of interrupt clearing)    */
    ProbeRS232_RxHandler(SCI2D);                                        /* Call the generic Rx handler with the received data       */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1C2 &= ~SCI1C2_RIE_MASK;                                         /* Disable Rx interrupts                                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2C2 &= ~SCI2C2_RIE_MASK;                                         /* Disable Rx interrupts                                    */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1C2 |=  SCI1C2_RIE_MASK;                                         /* Enable Rx interrupts                                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2C2 |=  SCI2C2_RIE_MASK;                                         /* Enable Rx interrupts                                     */
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxISRHandler()
*
* Description : Handle transmit interrupts.
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
*               (2) The vector table should use ProbeRS232_RxTxISR() (See probe_rs232_a.s) for both
*                   receive and transmit SCI interrupts.
*
*               (3) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h,
*                   the interrupt vector table within vectors.c must be updated as well.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
   (void)SCI1S1;                                                        /* Read the status register (part of interrupt clearing)    */
    ProbeRS232_TxHandler();
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
   (void)SCI2S1;                                                        /* Read the status register (part of interrupt clearing)    */
    ProbeRS232_TxHandler();
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

void  ProbeRS232_Tx1 (CPU_INT08U  c)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1D = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2D = c;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1C2  &= ~SCI1C2_TCIE_MASK;                                       /* Disable Tx interrupts                                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2C2  &= ~SCI2C2_TCIE_MASK;                                       /* Disable Tx interrupts                                    */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    SCI1C2  |=  SCI1C2_TCIE_MASK;                                       /* Enable Tx interrupts                                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2C2  |=  SCI2C2_TCIE_MASK;                                       /* Enable Tx interrupts                                     */
#endif
}
