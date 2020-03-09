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
*                                          PORT FOR ST STR71x
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


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* ------------- Register Base Specifications ------------- */
#define  UART0_BASE             ((CPU_INT32U)0xC0004000)
#define  UART1_BASE             ((CPU_INT32U)0xC0005000)
#define  UART2_BASE             ((CPU_INT32U)0xC0006000)
#define  UART3_BASE             ((CPU_INT32U)0xC0007000)
#define  GPIO0_BASE             ((CPU_INT32U)0xE0003000)
#define  GPIO1_BASE             ((CPU_INT32U)0xE0004000)
#define  GPIO2_BASE             ((CPU_INT32U)0xE0005000)
#define  EIC_BASE               ((CPU_INT32U)0xFFFFF800)

                                                                /* ------------------------ IRQ IDs ----------------------- */
#define  IRQ_UART0                      9
#define  IRQ_UART1                     10
#define  IRQ_UART2                     11
#define  IRQ_UART3                     12

                                                                /* ---------------- UART0 Register Defines ---------------- */
#define  UART0_BR           (*(CPU_INT16U *)(UART0_BASE  + 0x0000))
#define  UART0_TXBUFR       (*(CPU_INT16U *)(UART0_BASE  + 0x0004))
#define  UART0_RXBUFR       (*(CPU_INT16U *)(UART0_BASE  + 0x0008))
#define  UART0_CR           (*(CPU_INT16U *)(UART0_BASE  + 0x000C))
#define  UART0_IER          (*(CPU_INT16U *)(UART0_BASE  + 0x0010))
#define  UART0_SR           (*(CPU_INT16U *)(UART0_BASE  + 0x0014))
#define  UART0_GTR          (*(CPU_INT16U *)(UART0_BASE  + 0x0018))
#define  UART0_TOR          (*(CPU_INT16U *)(UART0_BASE  + 0x001C))

                                                                /* ---------------- UART1 Register Defines ---------------- */
#define  UART1_BR           (*(CPU_INT16U *)(UART1_BASE  + 0x0000))
#define  UART1_TXBUFR       (*(CPU_INT16U *)(UART1_BASE  + 0x0004))
#define  UART1_RXBUFR       (*(CPU_INT16U *)(UART1_BASE  + 0x0008))
#define  UART1_CR           (*(CPU_INT16U *)(UART1_BASE  + 0x000C))
#define  UART1_IER          (*(CPU_INT16U *)(UART1_BASE  + 0x0010))
#define  UART1_SR           (*(CPU_INT16U *)(UART1_BASE  + 0x0014))
#define  UART1_GTR          (*(CPU_INT16U *)(UART1_BASE  + 0x0018))
#define  UART1_TOR          (*(CPU_INT16U *)(UART1_BASE  + 0x001C))

                                                                /* ---------------- UART2 Register Defines ---------------- */
#define  UART2_BR           (*(CPU_INT16U *)(UART2_BASE  + 0x0000))
#define  UART2_TXBUFR       (*(CPU_INT16U *)(UART2_BASE  + 0x0004))
#define  UART2_RXBUFR       (*(CPU_INT16U *)(UART2_BASE  + 0x0008))
#define  UART2_CR           (*(CPU_INT16U *)(UART2_BASE  + 0x000C))
#define  UART2_IER          (*(CPU_INT16U *)(UART2_BASE  + 0x0010))
#define  UART2_SR           (*(CPU_INT16U *)(UART2_BASE  + 0x0014))
#define  UART2_GTR          (*(CPU_INT16U *)(UART2_BASE  + 0x0018))
#define  UART2_TOR          (*(CPU_INT16U *)(UART2_BASE  + 0x001C))

                                                                /* ---------------- UART3 Register Defines ---------------- */
#define  UART3_BR           (*(CPU_INT16U *)(UART3_BASE  + 0x0000))
#define  UART3_TXBUFR       (*(CPU_INT16U *)(UART3_BASE  + 0x0004))
#define  UART3_RXBUFR       (*(CPU_INT16U *)(UART3_BASE  + 0x0008))
#define  UART3_CR           (*(CPU_INT16U *)(UART3_BASE  + 0x000C))
#define  UART3_IER          (*(CPU_INT16U *)(UART3_BASE  + 0x0010))
#define  UART3_SR           (*(CPU_INT16U *)(UART3_BASE  + 0x0014))
#define  UART3_GTR          (*(CPU_INT16U *)(UART3_BASE  + 0x0018))
#define  UART3_TOR          (*(CPU_INT16U *)(UART3_BASE  + 0x001C))

                                                                /* ---------------- GPIO0 Register Defines ---------------- */
#define  GPIO0_PC0          (*(CPU_INT16U *)(GPIO0_BASE  + 0x0000))
#define  GPIO0_PC1          (*(CPU_INT16U *)(GPIO0_BASE  + 0x0004))
#define  GPIO0_PC2          (*(CPU_INT16U *)(GPIO0_BASE  + 0x0008))

                                                                /* ----------------- EIC Register Defines ----------------- */
#define  EIC_IER            (*(CPU_INT32U *)(EIC_BASE    + 0x0020))
#define  EIC_IPR            (*(CPU_INT32U *)(EIC_BASE    + 0x0040))
#define  EIC_SIR(n)         (*(CPU_INT32U *)(EIC_BASE    + 0x0060 + 4 * (n)))


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

extern  CPU_INT08U  BSP_VectSet    (CPU_INT16U     device,
                                    CPU_FNCT_VOID  isr);

extern  CPU_INT32U  BSP_CPU_ClkFreq(void);


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0   ]   "
  #error  "                                  [     || PROBE_RS232_UART_1   ]   "
  #error  "                                  [     || PROBE_RS232_UART_2   ]   "
  #error  "                                  [     || PROBE_RS232_UART_3   ]   "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0   ]   "
  #error  "                                  [     || PROBE_RS232_UART_1   ]   "
  #error  "                                  [     || PROBE_RS232_UART_2   ]   "
  #error  "                                  [     || PROBE_RS232_UART_3   ]   "
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ----------------- CONFIGURE UART0 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    GPIO0_PC0          |=  DEF_BIT_08;
    GPIO0_PC1          &= ~DEF_BIT_08;
    GPIO0_PC2          &= ~DEF_BIT_08;
                                                                /* (2) TX pin configured as AF PP CMOS                      */
    GPIO0_PC0          |=  DEF_BIT_09;
    GPIO0_PC1          |=  DEF_BIT_09;
    GPIO0_PC2          |=  DEF_BIT_09;

                                                                /* ----------------- SETUP UART0 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART0, ProbeRS232_RxTxISRHandler);
    EIC_SIR(IRQ_UART0) |= 0x00000003;
    EIC_IER            |= (1 << IRQ_UART0);

                                                                /* --------------------- SETUP UART0 ---------------------- */
    UART0_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART0_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART0_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART0_IER           = 0x0000;                               /* Disable all interrputs                                   */
    UART0_CR            = 0x0189;                               /* Start the UART                                           */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ----------------- CONFIGURE UART1 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    GPIO0_PC0          |=  DEF_BIT_10;
    GPIO0_PC1          &= ~DEF_BIT_10;
    GPIO0_PC2          &= ~DEF_BIT_10;
                                                                /* (2) TX pin configured as AF PP CMOS                      */
    GPIO0_PC0          |=  DEF_BIT_11;
    GPIO0_PC1          |=  DEF_BIT_11;
    GPIO0_PC2          |=  DEF_BIT_11;

                                                                /* ----------------- SETUP UART1 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART1, ProbeRS232_RxTxISRHandler);
    EIC_SIR(IRQ_UART1) |= 0x00000003;
    EIC_IER            |= (1 << IRQ_UART1);

                                                                /* --------------------- SETUP UART1 ---------------------- */
    UART1_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART1_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART1_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART1_IER           = 0x0000;                               /* Disable all interrputs                                   */
    UART1_CR            = 0x0189;                               /* Start the UART                                           */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* ----------------- CONFIGURE UART2 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    GPIO0_PC0          |=  DEF_BIT_13;
    GPIO0_PC1          &= ~DEF_BIT_13;
    GPIO0_PC2          &= ~DEF_BIT_13;
                                                                /* (2) TX pin configured as AF PP CMOS                      */
    GPIO0_PC0          |=  DEF_BIT_14;
    GPIO0_PC1          |=  DEF_BIT_14;
    GPIO0_PC2          |=  DEF_BIT_14;

                                                                /* ----------------- SETUP UART2 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART2, ProbeRS232_RxTxISRHandler);
    EIC_SIR(IRQ_UART2) |= 0x00000003;
    EIC_IER            |= (1 << IRQ_UART2);

                                                                /* --------------------- SETUP UART2 ---------------------- */
    UART2_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART2_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART2_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART2_IER           = 0x0000;                               /* Disable all interrputs                                   */
    UART2_CR            = 0x0189;                               /* Start the UART                                           */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
                                                                /* ----------------- CONFIGURE UART3 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    GPIO0_PC0          |=  DEF_BIT_01;
    GPIO0_PC1          &= ~DEF_BIT_01;
    GPIO0_PC2          &= ~DEF_BIT_01;
                                                                /* (2) TX pin configured as AF PP CMOS                      */
    GPIO0_PC0          |=  DEF_BIT_00;
    GPIO0_PC1          |=  DEF_BIT_00;
    GPIO0_PC2          |=  DEF_BIT_00;

                                                                /* ----------------- SETUP UART3 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART3, ProbeRS232_RxTxISRHandler);
    EIC_SIR(IRQ_UART3) |= 0x00000003;
    EIC_IER            |= (1 << IRQ_UART3);

                                                                /* --------------------- SETUP UART3 ---------------------- */
    UART3_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART3_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART3_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART3_IER           = 0x0000;                               /* Disable all interrputs                                   */
    UART3_CR            = 0x0189;                               /* Start the UART                                           */
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
    CPU_INT16U  uart_status;
    CPU_INT16U  int_en;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    uart_status = UART0_SR;                                     /* Read the UART's Status Register                          */
    int_en      = UART0_IER;

    if ((uart_status & DEF_BIT_00) != 0) {

        uart_chr = (CPU_INT08U)(UART0_RXBUFR & 0x00FF);
        ProbeRS232_RxHandler(uart_chr);                         /* Call the processor-independent code                      */
    }

    if (((uart_status & DEF_BIT_01) != 0) &&
        ((int_en      & DEF_BIT_01) != 0)) {

        ProbeRS232_TxHandler();                                 /* Call the processor-independent code                      */
        UART0_GTR = DEF_BIT_04;                                 /* Reset the UART's Guard time                              */
    }

    if ((uart_status & DEF_BIT_04) != 0) {
        UART0_TOR = 0x00FF;
    }

    EIC_IPR = (1 << IRQ_UART0);                                 /* Clear the UART's bit in the EIC                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    uart_status = UART1_SR;                                     /* Read the UART's Status Register                          */
    int_en      = UART1_IER;

    if ((uart_status & DEF_BIT_00) != 0) {

        uart_chr = (CPU_INT08U)(UART1_RXBUFR & 0x00FF);
        ProbeRS232_RxHandler(uart_chr);                         /* Call the processor-independent code                      */
    }

    if (((uart_status & DEF_BIT_01) != 0) &&
        ((int_en      & DEF_BIT_01) != 0)) {

        ProbeRS232_TxHandler();                                 /* Call the processor-independent code                      */
        UART1_GTR = DEF_BIT_04;                                 /* Reset the UART's Guard time                              */
    }

    if ((uart_status & DEF_BIT_04) != 0) {
        UART1_TOR = 0x00FF;
    }

    EIC_IPR = (1 << IRQ_UART1);                                 /* Clear the UART's bit in the EIC                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    uart_status = UART2_SR;                                     /* Read the UART's Status Register                          */
    int_en      = UART2_IER;

    if ((uart_status & DEF_BIT_00) != 0) {

        uart_chr = (CPU_INT08U)(UART2_RXBUFR & 0x00FF);
        ProbeRS232_RxHandler(uart_chr);                         /* Call the processor-independent code                      */
    }

    if (((uart_status & DEF_BIT_01) != 0) &&
        ((int_en      & DEF_BIT_01) != 0)) {

        ProbeRS232_TxHandler();                                 /* Call the processor-independent code                      */
        UART2_GTR = DEF_BIT_04;                                 /* Reset the UART's Guard time                              */
    }

    if ((uart_status & DEF_BIT_04) != 0) {
        UART2_TOR = 0x00FF;
    }

    EIC_IPR = (1 << IRQ_UART2);                                 /* Clear the UART's bit in the EIC                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    uart_status = UART3_SR;                                     /* Read the UART's Status Register                          */
    int_en      = UART3_IER;

    if ((uart_status & DEF_BIT_00) != 0) {

        uart_chr = (CPU_INT08U)(UART3_RXBUFR & 0x00FF);
        ProbeRS232_RxHandler(uart_chr);                         /* Call the processor-independent code                      */
    }

    if (((uart_status & DEF_BIT_01) != 0) &&
        ((int_en      & DEF_BIT_01) != 0)) {

        ProbeRS232_TxHandler();                                 /* Call the processor-independent code                      */
        UART3_GTR = DEF_BIT_04;                                 /* Reset the UART's Guard time                              */
    }

    if ((uart_status & DEF_BIT_04) != 0) {
        UART3_TOR = 0x00FF;
    }

    EIC_IPR = (1 << IRQ_UART3);                                 /* Clear the UART's bit in the EIC                    */
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
    UART0_IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART2_IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART3_IER &= ~DEF_BIT_00;
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
    UART0_IER |= DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_IER |= DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART2_IER |= DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART3_IER |= DEF_BIT_00;
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
    UART0_TXBUFR = (CPU_INT16U)c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_TXBUFR = (CPU_INT16U)c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART2_TXBUFR = (CPU_INT16U)c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART3_TXBUFR = (CPU_INT16U)c;
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
    UART0_IER &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_IER &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART2_IER &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART3_IER &= ~DEF_BIT_01;
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
    UART0_IER |= DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_IER |= DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    UART2_IER |= DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    UART3_IER |= DEF_BIT_01;
#endif
}

