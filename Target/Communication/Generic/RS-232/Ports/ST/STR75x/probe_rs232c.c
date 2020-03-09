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
*                                          PORT FOR ST STR75x
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

extern  CPU_INT08U  BSP_VectSet(CPU_INT16U     device,
                                CPU_FNCT_VOID  isr);


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "
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
    static  GPIO_InitTypeDef    gpio_init_struct;
    static  UART_InitTypeDef    uart_init_struct;
    static  EIC_IRQInitTypeDef  eic_irq_init_struct;


                                                                /* Define UART initialization structure for UART            */
    uart_init_struct.UART_WordLength          = UART_WordLength_8D;
    uart_init_struct.UART_StopBits            = UART_StopBits_1;
    uart_init_struct.UART_Parity              = UART_Parity_No;
    uart_init_struct.UART_BaudRate            = baud_rate;
    uart_init_struct.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
    uart_init_struct.UART_Mode                = UART_Mode_Tx_Rx;
    uart_init_struct.UART_FIFO                = UART_FIFO_Disable;
    uart_init_struct.UART_TxFIFOLevel         = UART_FIFOLevel_1_8;
    uart_init_struct.UART_RxFIFOLevel         = UART_FIFOLevel_1_8;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ----------------- CONFIGURE UART0 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pin  = DEF_BIT_11;                    /* ... On UART0 TX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */


                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;         /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pin =  DEF_BIT_10;                    /* ... On UART0 RX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */

                                                                /* --------------------- SETUP UART0 ---------------------- */
    MRCC_PeripheralClockConfig(MRCC_Peripheral_UART0, ENABLE);      /* Enable  UART clock                                   */
    MRCC_PeripheralSWResetConfig(MRCC_Peripheral_UART0, DISABLE);   /* Release UART reset                                   */

    UART_Init(UART0, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_ITConfig(UART0, UART_IT_Transmit, ENABLE);             /* Enable UART0 transmit interrupt                          */
    UART_Cmd(UART0, ENABLE);                                    /* Enable UART0                                             */


                                                                /* ----------------- SETUP UART0 INTERRUPT ---------------- */
    eic_irq_init_struct.EIC_IRQChannel          = UART0_IRQChannel;     /* ... Specify UART0 channel;                       */
    eic_irq_init_struct.EIC_IRQChannelPriority  = 3;                    /* ... Use priority of 3;                           */
    eic_irq_init_struct.EIC_IRQChannelCmd       = ENABLE;               /* ... Enable IRQ.                                  */
    EIC_IRQInit(&eic_irq_init_struct);                          /* Initialize UART1 IRQ                                     */

    BSP_VectSet(UART0_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ----------------- CONFIGURE UART1 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pin  = DEF_BIT_21;                    /* ... On UART1 TX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */


                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;         /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pin =  DEF_BIT_20;                    /* ... On UART1 RX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */

                                                                /* --------------------- SETUP UART1 ---------------------- */
    MRCC_PeripheralClockConfig(MRCC_Peripheral_UART1, ENABLE);      /* Enable  UART clock                                   */
    MRCC_PeripheralSWResetConfig(MRCC_Peripheral_UART1, DISABLE);   /* Release UART reset                                   */

    UART_Init(UART1, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_ITConfig(UART1, UART_IT_Transmit, ENABLE);             /* Enable UART0 transmit interrupt                          */
    UART_Cmd(UART1, ENABLE);                                    /* Enable UART0                                             */

                                                                /* ----------------- SETUP UART1 INTERRUPT ---------------- */
    eic_irq_init_struct.EIC_IRQChannel          = UART1_IRQChannel;     /* ... Specify UART1 channel;                       */
    eic_irq_init_struct.EIC_IRQChannelPriority  = 3;                    /* ... Use priority of 3;                           */
    eic_irq_init_struct.EIC_IRQChannelCmd       = ENABLE;               /* ... Enable IRQ.                                  */
    EIC_IRQInit(&eic_irq_init_struct);                          /* Initialize UART1 IRQ                                     */

    BSP_VectSet(UART1_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* ----------------- CONFIGURE UART2 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pin  = DEF_BIT_25;                    /* ... On UART2 TX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */


                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;         /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pin =  DEF_BIT_24;                    /* ... On UART2 RX pin.                                     */
    GPIO_Init(GPIO0, &gpio_init_struct);                        /* Initialize on GPIO0.                                     */

                                                                /* --------------------- SETUP UART2 ---------------------- */
    MRCC_PeripheralClockConfig(MRCC_Peripheral_UART2, ENABLE);      /* Enable  UART clock                                   */
    MRCC_PeripheralSWResetConfig(MRCC_Peripheral_UART2, DISABLE);   /* Release UART reset                                   */

    UART_Init(UART2, &uart_init_struct);                        /* Initialize UART2                                         */
    UART_ITConfig(UART2, UART_IT_Transmit, ENABLE);             /* Enable UART2 transmit interrupt                          */
    UART_Cmd(UART2, ENABLE);                                    /* Enable UART2                                             */

                                                                /* ----------------- SETUP UART2 INTERRUPT ---------------- */
    eic_irq_init_struct.EIC_IRQChannel          = UART2_IRQChannel;     /* ... Specify UART2 channel;                       */
    eic_irq_init_struct.EIC_IRQChannelPriority  = 3;                    /* ... Use priority of 3;                           */
    eic_irq_init_struct.EIC_IRQChannelCmd       = ENABLE;               /* ... Enable IRQ.                                  */
    EIC_IRQInit(&eic_irq_init_struct);                          /* Initialize UART1 IRQ                                     */

    BSP_VectSet(UART2_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART2 interrupt vector                               */
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
    CPU_INT08U  uart_chr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    if (UART_GetITStatus(UART0, UART_IT_Receive) == SET) {
        uart_chr = UART_ReceiveData(UART0);                     /* Receive byte of data                                     */
        UART_ClearITPendingBit(UART0, UART_IT_Receive);         /* Clear pending bit                                        */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_GetITStatus(UART0, UART_IT_Transmit) == SET) {
        UART_ClearITPendingBit(UART0, UART_IT_Transmit);        /* Clear pending bit                                        */
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }

    EIC->IPR = (CPU_INT32U)(1 << UART0_IRQChannel);             /* Clear interrupt pending bit in EIC_IPR register          */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    if (UART_GetITStatus(UART1, UART_IT_Receive) == SET) {
        uart_chr = UART_ReceiveData(UART1);                     /* Receive byte of data                                     */
        UART_ClearITPendingBit(UART1, UART_IT_Receive);         /* Clear pending bit                                        */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_GetITStatus(UART1, UART_IT_Transmit) == SET) {
        UART_ClearITPendingBit(UART1, UART_IT_Transmit);        /* Clear pending bit                                        */
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }

    EIC->IPR = (CPU_INT32U)(1 << UART1_IRQChannel);             /* Clear interrupt pending bit in EIC_IPR register          */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    if (UART_GetITStatus(UART2, UART_IT_Receive) == SET) {
        uart_chr = UART_ReceiveData(UART2);                     /* Receive byte of data                                     */
        UART_ClearITPendingBit(UART2, UART_IT_Receive);         /* Clear pending bit                                        */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_GetITStatus(UART2, UART_IT_Transmit) == SET) {
        UART_ClearITPendingBit(UART0, UART_IT_Transmit);        /* Clear pending bit                                        */
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }

    EIC->IPR = (CPU_INT32U)(1 << UART0_IRQChannel);             /* Clear interrupt pending bit in EIC_IPR register          */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ITConfig(UART0, UART_IT_Receive, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_Receive, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART2, UART_IT_Receive, DISABLE);
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
    UART_ITConfig(UART0, UART_IT_Receive, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_Receive, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_Receive, ENABLE);
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
    UART_SendData(UART0, c);                                    /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_SendData(UART1, c);                                    /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_SendData(UART2, c);                                    /* Place the character in the TX buffer                     */
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
    UART_ITConfig(UART0, UART_IT_Transmit, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_Transmit, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_Transmit, DISABLE);
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
    UART_ITConfig(UART0, UART_IT_Transmit, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_Transmit, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_Transmit, ENABLE);
#endif
}
