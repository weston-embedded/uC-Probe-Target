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
*                                    PORT FOR THE MICROCHIP PIC24
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
* Note(s)     : (1) Though the baud rate used may vary from application to application or target to
*                   target, other communication settings are constamt.  The hardware must always be
*                   configured for the following :
*
*                   (a) No parity
*                   (b) One stop bit
*                   (c) Eight data bits.
*
*               (2) Neither receive nor transmit interrupts should be enabled by this function.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT32U  fcy;
    CPU_INT32U  baud_div;


    fcy                 =  BSP_CPU_ClkFrq();                                /* Get the Fcy frequency which drives the UART          */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    baud_div            = (fcy / (16 * baud_rate)) - 1;                     /* Compute divisor for desired baud rate                */
    U2BRG               = (CPU_INT16U)baud_div;

    U2MODE              =  UART_EN;                                         /* Enable the UART with the parameters 8N1              */

    U2STA               =  UTXEN | UTXISEL;                                 /* Enable the transmitter, configure the Tx and Rx      */
                                                                            /* interrupt modes, but do not enable interrupts        */
                                                                            /* Tx Interrupt Mode is set for Tx Shift Reg Empty      */
                                                                            /* Rx Interrupt Mode is set to Interrupt for each Rx    */

    IFS1               &= ~(U2TXIF | U2RXIF);                               /* Clear pending interrupts                             */
    IPC6               &= ~U2TXIP_MASK;                                     /* Clear the UART2 Tx priority bits                     */
    IPC6               &= ~U2RXIP_MASK;                                     /* Clear the UART2 Rx priority bits                     */
    IPC6               |= (PROBE_INT_PRIO << 0) | (PROBE_INT_PRIO << 4);    /* Configure the UART2 Rx and Tx interrupt priorities   */
#endif                                                                      /* See bsp.h                                            */
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
* Note(s)     : (1) The interrupt source is cleared by __U2RXInterrupt located in probe_rs232a.s
*
*               (2) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h
*                   the file probe_rs232a.s will have to be adjusted accordingly.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IFS1 &= ~U2RXIF;                                                    /* Clear the interrupt flag                                 */
    ProbeRS232_RxHandler(U2RXREG);                                      /* Call the generic Rx handler with the received data       */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IEC1    &= ~U2RXIE;                                                 /* Disable Rx Interrupts                                    */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IEC1    |=  U2RXIE;                                                 /* Enable Rx interrupts                                     */
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
* Note(s)     : (1) The interrupt source is cleared by __U2TXInterrupt located in probe_rs232a.s
*
*               (2) When changing the value of PROBE_RS232_CFG_COMM_SEL from within probe_com_cfg.h
*                   the file probe_rs232a.s will have to be adjusted accordingly.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IFS1 &= ~U2TXIF;                                                    /* Clear the interrupt flag                                 */
    ProbeRS232_TxHandler();                                             /* Call the generic Tx complete handler                     */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2TXREG = c;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IEC1   &= ~U2TXIE;                                                  /* Disable Tx interrupts                                    */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    IEC1   |=  U2TXIE;                                                  /* Enable Tx interrupts                                     */
#endif
}
