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
*                                          PORT FOR ST STR73x
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
#define  UART0_BASE             ((CPU_INT32U)0xFFFF9C00)
#define  UART1_BASE             ((CPU_INT32U)0xFFFFA000)
#define  UART2_BASE             ((CPU_INT32U)0xFFFF9E00)
#define  UART3_BASE             ((CPU_INT32U)0xFFFFA200)
#define  GPIO_BASE              ((CPU_INT32U)0xFFFFD400)
#define  EIC_BASE               ((CPU_INT32U)0xFFFFFC00)
#define  CFG_BASE               ((CPU_INT32U)0x40000000)

                                                                /* ------------------------ IRQ IDs ----------------------- */
#define  IRQ_UART0                     45
#define  IRQ_UART1                     46
#define  IRQ_UART2                     31
#define  IRQ_UART3                     32

#define  CLK_UART0                      4
#define  CLK_UART1                      5
#define  CLK_UART2                     40
#define  CLK_UART3                     41

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

                                                                /* ---------------- GPIO Register Defines ----------------- */
#define  GPIO2_PC0          (*(CPU_INT16U *)(GPIO_BASE   + 0x0020))
#define  GPIO2_PC1          (*(CPU_INT16U *)(GPIO_BASE   + 0x0024))
#define  GPIO2_PC2          (*(CPU_INT16U *)(GPIO_BASE   + 0x0028))

#define  GPIO6_PC0          (*(CPU_INT16U *)(GPIO_BASE   + 0x0060))
#define  GPIO6_PC1          (*(CPU_INT16U *)(GPIO_BASE   + 0x0064))
#define  GPIO6_PC2          (*(CPU_INT16U *)(GPIO_BASE   + 0x0068))

                                                                /* ----------------- EIC Register Defines ----------------- */
#define  EIC_IER0           (*(CPU_INT32U *)(EIC_BASE    + 0x0020))
#define  EIC_IER1           (*(CPU_INT32U *)(EIC_BASE    + 0x0024))
#define  EIC_IPR0           (*(CPU_INT32U *)(EIC_BASE    + 0x0040))
#define  EIC_IPR1           (*(CPU_INT32U *)(EIC_BASE    + 0x0044))
#define  EIC_SIR(n)         (*(CPU_INT32U *)(EIC_BASE    + 0x0060 + 4 * (n)))

                                                                /* ----------------- CFG Register Defines ----------------- */
#define  CFG_PCGR0          (*(CPU_INT32U *)(CFG_BASE    + 0x0008))
#define  CFG_PCGR1          (*(CPU_INT32U *)(CFG_BASE    + 0x000C))


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
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "
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
    GPIO6_PC0          |=  DEF_BIT_08;
    GPIO6_PC1          &= ~DEF_BIT_08;
    GPIO6_PC2          &= ~DEF_BIT_08;

                                                                /* (2) TX pin configured as AF PP                           */
    GPIO6_PC0          |=  DEF_BIT_09;
    GPIO6_PC1          |=  DEF_BIT_09;
    GPIO6_PC2          |=  DEF_BIT_09;

                                                                /* ----------------- SETUP UART0 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART0, ProbeRS232_RxTxISRHandler);          /* Set UART0 interrupt vector                               */
    EIC_IER1           |= (1 << (IRQ_UART0 - 32));              /* Enable UART0 IRQ                                         */
    EIC_SIR(IRQ_UART0) |= 0x0B;                                 /* Assign UART0 IRQ priority level 11                       */

                                                                /* --------------------- SETUP UART0 ---------------------- */
    CFG_PCGR0          |= 1 << CLK_UART0;                       /* Enable  UART clock                                       */

    UART0_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART0_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART0_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART0_IER           = 0x0000;                               /* Disable all interupts                                    */
    UART0_CR            = 0x0189;                               /* Start the UART                                           */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ----------------- CONFIGURE UART1 I/Os ----------------- */
                                                                /* (1) RX pin configured as IN TRI TTL                      */
    GPIO2_PC0          |=  DEF_BIT_09;
    GPIO2_PC1          &= ~DEF_BIT_09;
    GPIO2_PC2          &= ~DEF_BIT_09;

                                                                /* (2) TX pin configured as AF PP                           */
    GPIO2_PC0          |=  DEF_BIT_08;
    GPIO2_PC1          |=  DEF_BIT_08;
    GPIO2_PC2          |=  DEF_BIT_08;

                                                                /* ----------------- SETUP UART1 INTERRUPT ---------------- */
    BSP_VectSet(IRQ_UART1, ProbeRS232_RxTxISRHandler);          /* Set UART0 interrupt vector                               */
    EIC_IER1           |= (1 << (IRQ_UART1 - 32));              /* Enable UART0 IRQ                                         */
    EIC_SIR(IRQ_UART1) |= 0x0B;                                 /* Assign UART0 IRQ priority level 11                       */

                                                                /* --------------------- SETUP UART1 ---------------------- */
    CFG_PCGR0          |= 1 << CLK_UART1;                       /* Enable  UART clock                                       */

    UART1_BR            = BSP_CPU_ClkFreq() / (16 * baud_rate); /* Set the UART's baud rate                                 */
    UART1_TOR           = 0x00FF;                               /* Set the UART's timeout register                          */
    UART1_GTR           = 0x0080;                               /* Set the UART's guard time                                */
    UART1_IER           = 0x0000;                               /* Disable all interupts                                    */
    UART1_CR            = 0x0189;                               /* Start the UART                                           */
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
    UART0_TXBUFR = (CPU_INT16U)c;                               /* Place the character in the TX buffer                     */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1_TXBUFR = (CPU_INT16U)c;                               /* Place the character in the TX buffer                     */
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
}
