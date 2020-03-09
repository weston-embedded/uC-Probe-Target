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
* Filename : probe_rs232c.c
* Version  : V2.30
*********************************************************************************************************
*/

#include <includes.h>

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        FUNCTION PROTOTYPES
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
*                       Initialize COM port for uC/Probe RS-232 Communication Module
*
* Description: Initialize the UART for uC/Probe communication.
*
* Argument(s): baud_rate    is intended baud rate of the RS-232.
*
* Returns    : None
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


    CPU_CRITICAL_ENTER();
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_Init(0, baud_rate);

    MCF_INTC0_ICR(13) = PROBE_RS232_RX_TX_ISR_IP_IL;                    /* See APP_CFG.H                                            */
	CPU_VectSet(64 + 13, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #0 Interrupt                                        */
    MCF_INTC0_IMRL &= ~MCF_INTC_IMRL_INT_MASK13;                        /* Unmask Tx & Rx interrupts                                */
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_Init(1, baud_rate);

    MCF_INTC0_ICR(14) = PROBE_RS232_RX_TX_ISR_IP_IL;                    /* See APP_CFG.H                                            */
	CPU_VectSet(64 + 14, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #1 Interrupt                                        */
    MCF_INTC0_IMRL &= ~MCF_INTC_IMRL_INT_MASK14;                        /* Unmask Tx & Rx interrupts                                */
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_Init(2, baud_rate);

    MCF_INTC0_ICR(15) = PROBE_RS232_RX_TX_ISR_IP_IL;                    /* See APP_CFG.H                                            */
	CPU_VectSet(64 + 15, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #2 Interrupt                                        */
    MCF_INTC0_IMRL &= ~MCF_INTC_IMRL_INT_MASK15;                        /* Unmask Tx & Rx interrupts                                */
#endif
    CPU_CRITICAL_EXIT();
}
#endif

/*
*********************************************************************************************************
*                                 Rx and Tx Communication Handler
*
* Description: This function handles both Rx and Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*
* Note(s)    : This function is empty because Rx and Tx interrupts are both handled in separate
*              handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    volatile  INT8U  usr;
    volatile  INT8U  rx_data;


#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    usr = MCF_UART0_USR;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    usr = MCF_UART1_USR;
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    usr = MCF_UART2_USR;
#endif


    if (usr & MCF_UART_USR_RXRDY) {                                     /* Received a character?                                    */
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
        rx_data = BSP_USART_ByteRd(0);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
        rx_data = BSP_USART_ByteRd(1);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
        rx_data = BSP_USART_ByteRd(2);
#endif

        ProbeRS232_RxHandler(rx_data);                                  /* Call the generic Rx handler                              */
    }

    if (usr & MCF_UART_USR_TXRDY) {                                     /* Transmitted character?                                   */
        ProbeRS232_TxHandler();
    }
}

/*
*********************************************************************************************************
*                                 Rx and Tx Communication Handlers
*
* Description: These functions handle the Rx and Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    /* The Coldfire has both the Rx and Tx interrupt on the same Interrupt Vector */
}

void  ProbeRS232_TxISRHandler (void)
{
    /* The Coldfire has both the Rx and Tx interrupt on the same Interrupt Vector */
}

/*
*********************************************************************************************************
*                                      Transmit One Byte
*
* Description: This function transmits one byte.
*
* Argument(s): c    is the byte to transmit.
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (INT8U c)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_ByteWr(0, c);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_ByteWr(1, c);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_ByteWr(2, c);
#endif
}

/*
*********************************************************************************************************
*                                      Enable/disable Tx Interrupts
*
* Description: Enables or disables Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntDis(0, MCF_UART_UIMR_TXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_TXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntDis(2, MCF_UART_UIMR_TXRDY);
#endif
}

void  ProbeRS232_TxIntEn (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntEn(0, MCF_UART_UIMR_TXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_TXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntEn(2, MCF_UART_UIMR_TXRDY);
#endif
}

/*
*********************************************************************************************************
*                                      Enable/disable Rx Interrupts
*
* Description: Enables or disables Rx interrupts.
*
* Argument(s): None
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntDis(0, MCF_UART_UIMR_FFULL_RXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_FFULL_RXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntDis(2, MCF_UART_UIMR_FFULL_RXRDY);
#endif
}

void  ProbeRS232_RxIntEn (void)
{
#if   (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntEn(0, MCF_UART_UIMR_FFULL_RXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_FFULL_RXRDY);
#elif (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntEn(2, MCF_UART_UIMR_FFULL_RXRDY);
#endif
}