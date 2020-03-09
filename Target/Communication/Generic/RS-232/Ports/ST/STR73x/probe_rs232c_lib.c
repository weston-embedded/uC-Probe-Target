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
*                                      Port for the ST STR73x
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
  #error  "                                  [     || PROBE_RS232_UART_3]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "
  #error  "                                  [     || PROBE_RS232_UART_3]      "
#endif


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

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    UART_InitTypeDef  uart_init_struct;
    GPIO_InitTypeDef  gpio_init_struct;


                                                                /* ------------------- INIT UART STRUCT ------------------- */
    UART_StructInit(&uart_init_struct);
    uart_init_struct.UART_BaudRate      = baud_rate;
    uart_init_struct.UART_Mode          = UART_Mode_8D;
    uart_init_struct.UART_StopBits      = UART_StopBits_1;
    uart_init_struct.UART_Loop_Standard = UART_Standard;
    uart_init_struct.UART_Rx            = UART_Rx_Enable;
    uart_init_struct.UART_FIFO          = UART_FIFO_Disable;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ----------------- CONFIGURE UART0 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;          /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pins =  DEF_BIT_08;                   /* ... On UART0 RX pin.                                     */
    GPIO_Init(GPIO6, &gpio_init_struct);                        /* Initialize on GPIO6.                                     */

                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pins = DEF_BIT_09;                    /* ... On UART0 TX pin.                                     */
    GPIO_Init(GPIO6, &gpio_init_struct);                        /* Initialize on GPIO6.                                     */

                                                                /* ----------------- SETUP UART0 INTERRUPT ---------------- */
    EIC_IRQChannelConfig(UART0_IRQChannel, ENABLE);             /* Enable UART0 IRQ                                         */
    EIC_IRQChannelPriorityConfig(UART0_IRQChannel, 11);         /* Assign UART0 IRQ priority level 11                       */
    BSP_VectSet(UART0_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */

                                                                /* --------------------- SETUP UART0 ---------------------- */
    CFG_PeripheralClockConfig(CFG_CLK_UART0 , ENABLE);          /* Enable  UART clock                                       */
    UART_Init(UART0, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_Cmd(UART0, ENABLE);                                    /* Enable UART0                                             */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ----------------- CONFIGURE UART1 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;          /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pins =  DEF_BIT_09;                   /* ... On UART0 RX pin.                                     */
    GPIO_Init(GPIO2, &gpio_init_struct);                        /* Initialize on GPIO2.                                     */

                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pins = DEF_BIT_08;                    /* ... On UART0 TX pin.                                     */
    GPIO_Init(GPIO2, &gpio_init_struct);                        /* Initialize on GPIO2.                                     */

                                                                /* ----------------- SETUP UART1 INTERRUPT ---------------- */
    EIC_IRQChannelConfig(UART1_IRQChannel, ENABLE);             /* Enable UART0 IRQ                                         */
    EIC_IRQChannelPriorityConfig(UART1_IRQChannel, 11);         /* Assign UART0 IRQ priority level 11                       */
    BSP_VectSet(UART1_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */

                                                                /* --------------------- SETUP UART1 ---------------------- */
    CFG_PeripheralClockConfig(CFG_CLK_UART1 , ENABLE);          /* Enable  UART clock                                       */
    UART_Init(UART1, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_Cmd(UART1, ENABLE);                                    /* Enable UART0                                             */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* ----------------- CONFIGURE UART2 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;          /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pins =  DEF_BIT_10;                   /* ... On UART0 RX pin.                                     */
    GPIO_Init(GPIO5, &gpio_init_struct);                        /* Initialize on GPIO5.                                     */

                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pins = DEF_BIT_11;                    /* ... On UART0 TX pin.                                     */
    GPIO_Init(GPIO5, &gpio_init_struct);                        /* Initialize on GPIO5.                                     */

                                                                /* ----------------- SETUP UART2 INTERRUPT ---------------- */
    EIC_IRQChannelConfig(UART2_IRQChannel, ENABLE);             /* Enable UART0 IRQ                                         */
    EIC_IRQChannelPriorityConfig(UART2_IRQChannel, 11);         /* Assign UART0 IRQ priority level 11                       */
    BSP_VectSet(UART2_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */

                                                                /* --------------------- SETUP UART2 ---------------------- */
    CFG_PeripheralClockConfig(CFG_CLK_UART2 , ENABLE);          /* Enable  UART clock                                       */
    UART_Init(UART2, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_Cmd(UART2, ENABLE);                                    /* Enable UART0                                             */

#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
                                                                /* ----------------- CONFIGURE UART3 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;          /* ... Use INput FLOATING;                                  */
    gpio_init_struct.GPIO_Pins =  DEF_BIT_02;                   /* ... On UART0 RX pin.                                     */
    GPIO_Init(GPIO6, &gpio_init_struct);                        /* Initialize on GPIO6.                                     */

                                                                /* (2) TX pin configured as AF PP CMOS                      */
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;               /* ... Use Alternate Function Push-Pull;                    */
    gpio_init_struct.GPIO_Pins = DEF_BIT_04;                    /* ... On UART0 TX pin.                                     */
    GPIO_Init(GPIO6, &gpio_init_struct);                        /* Initialize on GPIO6.                                     */

                                                                /* ----------------- SETUP UART3 INTERRUPT ---------------- */
    EIC_IRQChannelConfig(UART3_IRQChannel, ENABLE);             /* Enable UART0 IRQ                                         */
    EIC_IRQChannelPriorityConfig(UART3_IRQChannel, 11);         /* Assign UART0 IRQ priority level 11                       */
    BSP_VectSet(UART3_IRQChannel, ProbeRS232_RxTxISRHandler);   /* Set UART0 interrupt vector                               */

                                                                /* --------------------- SETUP UART3 ---------------------- */
    CFG_PeripheralClockConfig(CFG_CLK_UART3 , ENABLE);          /* Enable  UART clock                                       */
    UART_Init(UART3, &uart_init_struct);                        /* Initialize UART0                                         */
    UART_Cmd(UART3, ENABLE);                                    /* Enable UART0                                             */

#endif
}

/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description: Handle Rx and Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT08U  uart_chr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    if (UART_FlagStatus(UART0, UART_IT_RxBufFull) == SET) {
        uart_chr = UART_ByteReceive(UART0);                     /* Receive byte of data                                     */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_FlagStatus(UART0, UART_IT_TxEmpty) == SET) {
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    if (UART_FlagStatus(UART1, UART_IT_RxBufFull) == SET) {
        uart_chr = UART_ByteReceive(UART1);                     /* Receive byte of data                                     */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_FlagStatus(UART1, UART_IT_TxEmpty) == SET) {
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    if (UART_FlagStatus(UART2, UART_IT_RxBufFull) == SET) {
        uart_chr = UART_ByteReceive(UART2);                     /* Receive byte of data                                     */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_FlagStatus(UART2, UART_IT_TxEmpty) == SET) {
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    if (UART_FlagStatus(UART3, UART_IT_RxBufFull) == SET) {
        uart_chr = UART_ByteReceive(UART3);                     /* Receive byte of data                                     */
        ProbeRS232_RxHandler(uart_chr);                         /* Handle byte of data                                      */
    }

    if (UART_FlagStatus(UART3, UART_IT_TxEmpty) == SET) {
        ProbeRS232_TxHandler();                                 /* Handle data transmission                                 */
    }
#endif
}

/*
*********************************************************************************************************
*                                    ProbeRS232_RxISRHandler()
*
* Description: Handle Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : This function is empty because Rx interrupts are handled by ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntDis()
*
* Description: Disable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ITConfig(UART0, UART_IT_RxBufFull, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_RxBufFull, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_RxBufFull, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART_ITConfig(UART3, UART_IT_RxBufFull, DISABLE);
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntEn()
*
* Description: Enable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ITConfig(UART0, UART_IT_RxBufFull, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_RxBufFull, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_RxBufFull, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART_ITConfig(UART3, UART_IT_RxBufFull, ENABLE);
#endif
}


/*
*********************************************************************************************************
*                                    ProbeRS232_TxISRHandler()
*
* Description: Handle Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : This function is empty because Tx interrupts are handled by ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                      ProbeRS232_Tx1()
*
* Description: Transmit one byte.
*
* Argument(s): c            The byte to transmit.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ByteSend(UART0, c);                                    /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ByteSend(UART1, c);                                    /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ByteSend(UART2, c);                                    /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART_ByteSend(UART3, c);                                    /* Place the character in the TX buffer                     */
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntDis()
*
* Description: Disable Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ITConfig(UART0, UART_IT_TxEmpty, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_TxEmpty, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_TxEmpty, DISABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART_ITConfig(UART3, UART_IT_TxEmpty, DISABLE);
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntEn()
*
* Description: Enable Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    UART_ITConfig(UART0, UART_IT_TxEmpty, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART_ITConfig(UART1, UART_IT_TxEmpty, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART_ITConfig(UART2, UART_IT_TxEmpty, ENABLE);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART_ITConfig(UART3, UART_IT_TxEmpty, ENABLE);
#endif
}
