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
*                                      PORT FOR FREESCALE CF5253
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

#include <includes.h>


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT08U  level;
    CPU_INT08U  vector;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_Init(0, baud_rate);

    MCF_INTC1_ICR(4) =  MCF_INTC1_ICR_AVEC
                     |  PROBE_RS232_RX_TX_ISR_IP_IL;                    /* See APP_CFG.H                                            */

    level            = (PROBE_RS232_RX_TX_ISR_IP_IL >> 2) & 0x7;
    vector           = (CPU_INT08U)(24 + level);

    CPU_VectSet(vector, (void (*)(void))ProbeRS232_RxTxISR);            /* UART #0 Interrupt                                        */
    MCF_INTC1_IMR   &= ~MCF_INTC1_IMR_UART0;                            /* Unmask Tx & Rx interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_Init(1, baud_rate);

    MCF_INTC1_ICR(5) =  MCF_INTC1_ICR_AVEC
                     |  PROBE_RS232_RX_TX_ISR_IP_IL;                    /* See APP_CFG.H                                            */

    level            = (PROBE_RS232_RX_TX_ISR_IP_IL >> 2) & 0x7;
    vector           = (CPU_INT08U)(24 + level);

    CPU_VectSet(vector, (void (*)(void))ProbeRS232_RxTxISR);            /* UART #1 Interrupt                                        */
    MCF_INTC1_IMR   &= ~MCF_INTC1_IMR_UART1;                            /* Unmask Tx & Rx interrupts                                */
#endif
}
#endif


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
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    volatile  CPU_INT08U  usr;
    volatile  CPU_INT08U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    usr = MCF_UART0_USR;
#endif
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    usr = MCF_UART1_USR;
#endif


    if (usr & MCF_UART_USR_RXRDY) {                                     /* Received a character?                                    */
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
        rx_data = BSP_USART_ByteRd(0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
        rx_data = BSP_USART_ByteRd(1);
#endif

        ProbeRS232_RxHandler(rx_data);                                  /* Call the generic Rx handler                              */
    }

    if (usr & MCF_UART_USR_TXRDY) {                                     /* Transmitted character?                                   */
        ProbeRS232_TxHandler();
    }
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
* Note(s)     : (1) This function is empty because receive interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
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
    BSP_USART_IntDis(0, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_FFULL_RXRDY);
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
    BSP_USART_IntEn(0, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_FFULL_RXRDY);
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
* Note(s)     : (1) This function is empty because transmit interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
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
    BSP_USART_ByteWr(0, c);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_ByteWr(1, c);
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
    BSP_USART_IntDis(0, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_TXRDY);
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
    BSP_USART_IntEn(0, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_TXRDY);
#endif
}
