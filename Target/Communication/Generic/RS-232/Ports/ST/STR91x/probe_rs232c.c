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
*                                          PORT FOR ST STR91x
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

#include  <probe_rs232.h>
#include  <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  UART1_INT_PRIO             15


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
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
#endif


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
    UART_InitTypeDef  uart_init;
    GPIO_InitTypeDef  gpio_init;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ------------------- ENABLE UART1 I/Os ------------------ */
                                                                /* GPIO3.2, GPIO3.3                                         */
    gpio_init.GPIO_Pin              = DEF_BIT_02;
    gpio_init.GPIO_Direction        = GPIO_PinInput;
    gpio_init.GPIO_Type             = GPIO_Type_PushPull;
    gpio_init.GPIO_IPInputConnected = GPIO_IPInputConnected_Enable;
    gpio_init.GPIO_Alternate        = GPIO_InputAlt1;
    GPIO_Init(GPIO3, &gpio_init);

    gpio_init.GPIO_Pin              = DEF_BIT_03;
    gpio_init.GPIO_Direction        = GPIO_PinOutput;
    gpio_init.GPIO_IPInputConnected = GPIO_IPInputConnected_Disable;
    gpio_init.GPIO_Alternate        = GPIO_OutputAlt2;
    GPIO_Init(GPIO3, &gpio_init);


                                                                /* --------------------- SETUP UART1 ---------------------- */
    SCU_APBPeriphClockConfig(__UART1, ENABLE);                  /* Enable the UART's clock signal                           */
    SCU_APBPeriphReset(__UART1, DISABLE);                       /* Remove the UART from reset state                         */

    UART_DeInit(UART1);                                         /* Ensure that UART registers are at reset values           */
    UART_ITConfig(UART1, UART_IT_Transmit, ENABLE);

    BSP_IntVectSet(UART1_ITLine, UART1_INT_PRIO, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(UART1_ITLine, UART1_INT_PRIO, BSP_INT_TYPE_IRQ);

    uart_init.UART_WordLength          = UART_WordLength_8D;
    uart_init.UART_StopBits            = UART_StopBits_1;
    uart_init.UART_Parity              = UART_Parity_No;
    uart_init.UART_BaudRate            = baud_rate;
    uart_init.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
    uart_init.UART_Mode                = UART_Mode_Tx_Rx;
    uart_init.UART_FIFO                = UART_FIFO_Disable;
    uart_init.UART_TxFIFOLevel         = UART_FIFOLevel_1_8;
    uart_init.UART_RxFIFOLevel         = UART_FIFOLevel_1_8;

    UART_Init(UART1, &uart_init);                               /* Use 8 data bits, 1 stop bit, and no parity               */
    UART_Cmd(UART1, ENABLE);                                    /* Enable UART1                                             */
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
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CPU_INT08U  uart_chr;

                                                                /* An Rx interrupt has occurred                   */
    if ((UART_GetITStatus(UART1, UART_IT_Receive)) == SET) {
        uart_chr = UART_ReceiveData(UART1);                     /* Read a single byte                             */
                                                                /* Clear the interrupt                            */
        UART_ClearITPendingBit(UART1, UART_IT_Receive);
        ProbeRS232_RxHandler(uart_chr);                         /* Call the device-independent code               */
    }                                                           /* A Tx interrupt has occurred                    */
    if ((UART_GetITStatus(UART1, UART_IT_Transmit)) == SET) {
                                                                /* Clear the interrupt                            */
        UART_ClearITPendingBit(UART1, UART_IT_Transmit);
        ProbeRS232_TxHandler();                                 /* Call the device-independent code               */
    }

    VIC1->VAR = 0x00000000;                                     /* Signal the end of the interrupt                */
#endif
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_Receive, DISABLE);
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
    UART_ITConfig(UART1, UART_IT_Receive, ENABLE);
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_SendData(UART1, c);
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
    UART_ITConfig(UART1, UART_IT_Transmit, DISABLE);
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
    UART_ITConfig(UART1, UART_IT_Transmit, ENABLE);
#endif
}
