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
*                                            PORT TEMPLATE
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

#include  <probe_com.h>
#include  <probe_rs232.h>

#include  <xmc_gpio.h>
#include  <xmc_uart.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define UART_TX P1_5
#define UART_RX P1_4

#define ProbeRS232_TxISRHandler USIC0_0_IRQHandler
#define ProbeRS232_RxISRHandler USIC0_1_IRQHandler


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

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    XMC_UART_CH_CONFIG_t uart_config;
    XMC_GPIO_CONFIG_t    gpio_config;

    XMC_STRUCT_INIT(uart_config);

    uart_config.baudrate = baud_rate;
    uart_config.data_bits = 8;
    uart_config.parity_mode = XMC_USIC_CH_PARITY_MODE_NONE;
    uart_config.stop_bits = 1;

    XMC_UART_CH_Init(XMC_UART0_CH0, &uart_config);
    XMC_UART_CH_SetInputSource(XMC_UART0_CH0, XMC_UART_CH_INPUT_RXD, USIC0_C0_DX0_P1_4);
    XMC_UART_CH_Start(XMC_UART0_CH0);

    gpio_config.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
    XMC_GPIO_Init(UART_RX, &gpio_config);

    gpio_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL | P1_5_AF_U0C0_DOUT0;
    gpio_config.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH;
    gpio_config.output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_MEDIUM_EDGE;
    XMC_GPIO_Init(UART_TX, &gpio_config);

    XMC_USIC_CH_SetInterruptNodePointer(XMC_UART0_CH0, XMC_USIC_CH_INTERRUPT_NODE_POINTER_TRANSMIT_BUFFER, 0);
    XMC_USIC_CH_SetInterruptNodePointer(XMC_UART0_CH0, XMC_USIC_CH_INTERRUPT_NODE_POINTER_RECEIVE, 1);

    NVIC_SetPriority(USIC0_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 63U, 0U));
    NVIC_SetPriority(USIC0_1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 62U, 0U));
    NVIC_EnableIRQ(USIC0_0_IRQn);
    NVIC_EnableIRQ(USIC0_1_IRQn);
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
* Note(s)     : (1) This function may be empty if receive and transmit interrupts are handled in the
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
* Note(s)     : (1) This function may be empty if receive interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    CPU_INT08U rx_data;


    rx_data = XMC_UART_CH_GetReceivedData(XMC_UART0_CH0)  & 0xFF; /* Rd rx’d byte. */
    ProbeRS232_RxHandler(rx_data);
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
    XMC_UART_CH_DisableEvent(XMC_UART0_CH0, XMC_UART_CH_EVENT_STANDARD_RECEIVE | XMC_UART_CH_EVENT_ALTERNATIVE_RECEIVE);
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
    XMC_UART_CH_EnableEvent(XMC_UART0_CH0, XMC_UART_CH_EVENT_STANDARD_RECEIVE | XMC_UART_CH_EVENT_ALTERNATIVE_RECEIVE);
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
* Note(s)     : (1) This function may be empty if transmit interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
    ProbeRS232_TxHandler();
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
    XMC_UART_CH_Transmit(XMC_UART0_CH0, c);
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
    XMC_UART_CH_DisableEvent(XMC_UART0_CH0, XMC_UART_CH_EVENT_TRANSMIT_BUFFER);
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
    XMC_UART_CH_EnableEvent(XMC_UART0_CH0, XMC_UART_CH_EVENT_TRANSMIT_BUFFER);
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
