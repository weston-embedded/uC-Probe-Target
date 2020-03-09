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
*                                      PORT FOR FREESCALE M52233DEMO
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

extern  void  ProbeRS232_RxTxISR(void);


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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_Init(0, baud_rate);

    CPU_VectSet(64 + 13, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #0 Interrupt                                        */
    MCF_INTC0_ICR(13)   =  PROBE_RS232_RX_TX_ISR_IP_IL;                 /* See PROBE_COM_CFG.H                                      */
    MCF_INTC0_IMRL     &= ~MCF_INTC_IMRL_INT_MASK13;                    /* Unmask Tx & Rx interrupts                                */
#endif   

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_Init(1, baud_rate);

    CPU_VectSet(64 + 14, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #1 Interrupt                                        */
    MCF_INTC0_ICR(14)   =  PROBE_RS232_RX_TX_ISR_IP_IL;                 /* See PROBE_COM_CFG.H                                      */
    MCF_INTC0_IMRL     &= ~MCF_INTC_IMRL_INT_MASK14;                    /* Unmask Tx & Rx interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_Init(2, baud_rate);

    CPU_VectSet(64 + 15, (void (*)(void))ProbeRS232_RxTxISR);           /* UART #2 Interrupt                                        */
    MCF_INTC0_ICR(15)   =  PROBE_RS232_RX_TX_ISR_IP_IL;                 /* See PROBE_COM_CFG.H                                      */
    MCF_INTC0_IMRL     &= ~MCF_INTC_IMRL_INT_MASK15;                    /* Unmask Tx & Rx interrupts                                */
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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_RxTxISRHandler (void)
{
    volatile  INT8U  usr;
    volatile  INT8U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    usr = MCF_UART0_USR; // & (MCF_UART_USR_TXRDY | MCF_UART_USR_RXRDY);
#endif
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    usr = MCF_UART1_USR; // & (MCF_UART_USR_TXRDY | MCF_UART_USR_RXRDY);
#endif
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    usr = MCF_UART2_USR; // & (MCF_UART_USR_TXRDY | MCF_UART_USR_RXRDY);
#endif

    if (usr & MCF_UART_USR_RXRDY) {                                     /* Received a character?                                    */
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
        rx_data = BSP_USART_ByteRd(0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
        rx_data = BSP_USART_ByteRd(1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
        rx_data = BSP_USART_ByteRd(2);
#endif
        ProbeRS232_RxHandler(rx_data);                                  /* Call the generic Rx handler                              */
    }
    if (usr & MCF_UART_USR_TXRDY) {                                     /* Transmitted character?                                   */
        ProbeRS232_TxHandler();
    }
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_RxISRHandler (void)
{
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_RxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntDis(0, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntDis(2, MCF_UART_UIMR_FFULL_RXRDY);
#endif
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_RxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntEn(0, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_FFULL_RXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntEn(2, MCF_UART_UIMR_FFULL_RXRDY);
#endif
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_TxISRHandler (void)
{
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_Tx1 (CPU_INT08U  c)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_ByteWr(0, c);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_ByteWr(1, c);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_ByteWr(2, c);
#endif
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_TxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntDis(0, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntDis(1, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntDis(2, MCF_UART_UIMR_TXRDY);
#endif
}
#endif


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

#if (PROBE_COM_CFG_RS232_EN == DEF_TRUE)                                /* For the RS-232 communication method                      */
void  ProbeRS232_TxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_USART_IntEn(0, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    BSP_USART_IntEn(1, MCF_UART_UIMR_TXRDY);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    BSP_USART_IntEn(2, MCF_UART_UIMR_TXRDY);
#endif
}
#endif
