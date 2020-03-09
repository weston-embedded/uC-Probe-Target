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
*                                      PORT FOR FREESCALE MAC71xx
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
#define  ESCIA_BASE             ((CPU_INT32U)0xFC0C4000)
#define  ESCIB_BASE             ((CPU_INT32U)0xFC0C8000)
#define  ESCIC_BASE             ((CPU_INT32U)0xFC0CC000)
#define  ESCID_BASE             ((CPU_INT32U)0xFC0D0000)
#define  PIM_BASE               ((CPU_INT32U)0xFC0E8000)
                                                                /* ----------------------- INTC IDs ----------------------- */
#define INTC_ESCI_A                    39
#define INTC_ESCI_B                    40
#define INTC_ESCI_C                    41
#define INTC_ESCI_D                    42

                                                                /* ---------------- ESCIA Register Defines ---------------- */
#define  ESCIA_BD           (*(volatile CPU_INT16U *)(ESCIA_BASE + 0x0000))
#define  ESCIA_CR1          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0002))
#define  ESCIA_CR2          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0003))
#define  ESCIA_CR3          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0004))
#define  ESCIA_CR4          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0005))
#define  ESCIA_DRL          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0007))
#define  ESCIA_SR1          (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x0008))
#define  ESCIA_LINCTRL1     (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x000C))
#define  ESCIA_LINCTRL2     (*(volatile CPU_INT08U *)(ESCIA_BASE + 0x000D))

                                                                /* ---------------- ESCIB Register Defines ---------------- */
#define  ESCIB_BD           (*(volatile CPU_INT16U *)(ESCIB_BASE + 0x0000))
#define  ESCIB_CR1          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0002))
#define  ESCIB_CR2          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0003))
#define  ESCIB_CR3          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0004))
#define  ESCIB_CR4          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0005))
#define  ESCIB_DRL          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0007))
#define  ESCIB_SR1          (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x0008))
#define  ESCIB_LINCTRL1     (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x000C))
#define  ESCIB_LINCTRL2     (*(volatile CPU_INT08U *)(ESCIB_BASE + 0x000D))

                                                                /* ---------------- ESCIC Register Defines ---------------- */
#define  ESCIC_BD           (*(volatile CPU_INT16U *)(ESCIC_BASE + 0x0000))
#define  ESCIC_CR1          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0002))
#define  ESCIC_CR2          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0003))
#define  ESCIC_CR3          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0004))
#define  ESCIC_CR4          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0005))
#define  ESCIC_DRL          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0007))
#define  ESCIC_SR1          (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x0008))
#define  ESCIC_LINCTRL1     (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x000C))
#define  ESCIC_LINCTRL2     (*(volatile CPU_INT08U *)(ESCIC_BASE + 0x000D))

                                                                /* ---------------- ESCID Register Defines ---------------- */
#define  ESCID_BD           (*(volatile CPU_INT16U *)(ESCID_BASE + 0x0000))
#define  ESCID_CR1          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0002))
#define  ESCID_CR2          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0003))
#define  ESCID_CR3          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0004))
#define  ESCID_CR4          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0005))
#define  ESCID_DRL          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0007))
#define  ESCID_SR1          (*(volatile CPU_INT08U *)(ESCID_BASE + 0x0008))
#define  ESCID_LINCTRL1     (*(volatile CPU_INT08U *)(ESCID_BASE + 0x000C))
#define  ESCID_LINCTRL2     (*(volatile CPU_INT08U *)(ESCID_BASE + 0x000D))

                                                                /* ----------------- PIM Register Defines ----------------- */
#define  PIM_CONFIG0_G      (*(volatile CPU_INT16U *)(PIM_BASE   + 0x0180))
#define  PIM_CONFIG1_G      (*(volatile CPU_INT16U *)(PIM_BASE   + 0x0182))
#define  PIM_CONFIG2_G      (*(volatile CPU_INT16U *)(PIM_BASE   + 0x0184))
#define  PIM_CONFIG3_G      (*(volatile CPU_INT16U *)(PIM_BASE   + 0x0186))
#define  PIM_CONFIG12_G     (*(volatile CPU_INT16U *)(PIM_BASE   + 0x0198))
#define  PIM_CONFIG13_G     (*(volatile CPU_INT16U *)(PIM_BASE   + 0x019A))
#define  PIM_CONFIG14_G     (*(volatile CPU_INT16U *)(PIM_BASE   + 0x019C))
#define  PIM_CONFIG15_G     (*(volatile CPU_INT16U *)(PIM_BASE   + 0x019E))


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*
* Note(s) : (1) 'BSP_CPU_ClkFreq()' should return the Fsys frequency in Hz.
*
*           (2) 'BSP_IntInstall()' should install an interrupt service routine into the BSP's
*               interrupt vector table.  The arguments should be
*
*               irq_num     The interrupt source number.
*               isr         The interrupt service routine (ISR).
*               priority    The level to assign to the interrupt.
*********************************************************************************************************
*/

extern  CPU_INT32U  BSP_CPU_ClkFreq(void);
extern  void        BSP_IntInstall (CPU_INT08U     irq_num,
                                    CPU_FNCT_VOID  isr,
                                    CPU_INT08U     priority);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*
* Note(s) : (1) PROBE_RS232_UART_0 selects Extended Serial Communication Interface A (ESCIA),
*               PROBE_RS232_UART_1 selects     "      "          "           "     B (ESCIB),
*               PROBE_RS232_UART_2 selects     "      "          "           "     C (ESCIC),
*               PROBE_RS232_UART_3 selects     "      "          "           "     D (ESCID).
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [MUST be PROBE_RS232_UART_2]      "
  #error  "                                  [MUST be PROBE_RS232_UART_3]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [MUST be PROBE_RS232_UART_2]      "
  #error  "                                  [MUST be PROBE_RS232_UART_3]      "
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
    CPU_INT32U  clk_freq;
    CPU_INT16U  div;


    clk_freq       = BSP_CPU_ClkFreq();                             /* Calculate clock frequency.                           */
    div            = (CPU_INT16U)(clk_freq / (2 * baud_rate * 16)); /* Calculate baud rate divisor.                         */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ----------------- CONFIGURE ESCIA I/Os ----------------- */
                                                                /* Set PORTG pins 2 & 3 as SCI pins.                        */
    PIM_CONFIG2_G  = DEF_BIT_07;
    PIM_CONFIG3_G  = DEF_BIT_07;

                                                                /* --------------------- SETUP ESCIA ---------------------- */
    ESCIA_CR1      = 0x00;                                      /* 1 stop bit, 8 data bits, 1 start bit, no parity.         */
    ESCIA_CR2      = 0x00;                                      /* Disable interrupts; disable receiver and transmitter.    */
    ESCIA_CR3      = 0x00;                                      /* Enable ESCIA.                                            */
    ESCIA_CR4      = 0x00;                                      /* Disable interrupts.                                      */
    ESCIA_LINCTRL1 = 0x00;                                      /* Disable line control mode.                               */
    ESCIA_LINCTRL2 = 0x00;
    ESCIA_BD       = div;                                       /* Set baud rate.                                           */
    ESCIA_CR2      = DEF_BIT_02 | DEF_BIT_03;                   /* Receiver and transmitter enabled.                        */

                                                                /* -------------- INITIALIZE INT. FOR ESCIA --------------- */
    BSP_IntInstall(INTC_ESCI_A, ProbeRS232_RxTxISRHandler, 5);  /* Assign ESCIA interrupt.                                  */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* ----------------- CONFIGURE ESCIB I/Os ----------------- */
                                                                /* Set PORTG pins 0 & 1 as SCI pins.                        */
    PIM_CONFIG0_G  = DEF_BIT_07;
    PIM_CONFIG1_G  = DEF_BIT_07;

                                                                /* --------------------- SETUP ESCIB ---------------------- */
    ESCIB_CR1      = 0x00;                                      /* 1 stop bit, 8 data bits, 1 start bit, no parity.         */
    ESCIB_CR2      = 0x00;                                      /* Disable interrupts; disable receiver and transmitter.    */
    ESCIB_CR3      = 0x00;                                      /* Enable ESCIB.                                            */
    ESCIB_CR4      = 0x00;                                      /* Disable interrupts.                                      */
    ESCIB_LINCTRL1 = 0x00;                                      /* Disable line control mode.                               */
    ESCIB_LINCTRL2 = 0x00;
    ESCIB_BD       = div;                                       /* Set baud rate.                                           */
    ESCIB_CR2      = DEF_BIT_02 | DEF_BIT_03;                   /* Receiver and transmitter enabled.                        */

                                                                /* -------------- INITIALIZE INT. FOR ESCIB --------------- */
    BSP_IntInstall(INTC_ESCI_B, ProbeRS232_RxTxISRHandler, 5);  /* Assign ESCIB interrupt.                                  */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* ----------------- CONFIGURE ESCIC I/Os ----------------- */
                                                                /* Set PORTG pins 12 & 13 as SCI pins.                      */
    PIM_CONFIG12_G = DEF_BIT_07;
    PIM_CONFIG13_G = DEF_BIT_07;

                                                                /* --------------------- SETUP ESCIC ---------------------- */
    ESCIC_CR1      = 0x00;                                      /* 1 stop bit, 8 data bits, 1 start bit, no parity.         */
    ESCIC_CR2      = 0x00;                                      /* Disable interrupts; disable receiver and transmitter.    */
    ESCIC_CR3      = 0x00;                                      /* Enable ESCIC.                                            */
    ESCIC_CR4      = 0x00;                                      /* Disable interrupts.                                      */
    ESCIC_LINCTRL1 = 0x00;                                      /* Disable line control mode.                               */
    ESCIC_LINCTRL2 = 0x00;
    ESCIC_BD       = div;                                       /* Set baud rate.                                           */
    ESCIC_CR2      = DEF_BIT_02 | DEF_BIT_03;                   /* Receiver and transmitter enabled.                        */

                                                                /* -------------- INITIALIZE INT. FOR ESCIC --------------- */
    BSP_IntInstall(INTC_ESCI_C, ProbeRS232_RxTxISRHandler, 5);  /* Assign ESCIC interrupt.                                  */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
                                                                /* ----------------- CONFIGURE ESCID I/Os ----------------- */
                                                                /* Set PORTG pins 14 & 15 as SCI pins.                      */
    PIM_CONFIG14_G = DEF_BIT_07;
    PIM_CONFIG15_G = DEF_BIT_07;

                                                                /* --------------------- SETUP ESCID ---------------------- */
    ESCID_CR1      = 0x00;                                      /* 1 stop bit, 8 data bits, 1 start bit, no parity.         */
    ESCID_CR2      = 0x00;                                      /* Disable interrupts; disable receiver and transmitter.    */
    ESCID_CR3      = 0x00;                                      /* Enable ESCID.                                            */
    ESCID_CR4      = 0x00;                                      /* Disable interrupts.                                      */
    ESCID_LINCTRL1 = 0x00;                                      /* Disable line control mode.                               */
    ESCID_LINCTRL2 = 0x00;
    ESCID_BD       = div;                                       /* Set baud rate.                                           */
    ESCID_CR2      = DEF_BIT_02 | DEF_BIT_03;                   /* Receiver and transmitter enabled.                        */

                                                                /* -------------- INITIALIZE INT. FOR ESCID --------------- */
    BSP_IntInstall(INTC_ESCI_D, ProbeRS232_RxTxISRHandler, 5);  /* Assign ESCID interrupt.                                  */
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
* Note(s)     : (1) The interrupt must be acknowledged by the generic interrupt handler which vectored
*                   to this specific IRQ handler after this function returns.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT08U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)                /* ------------------------- ESCIA ------------------------ */
                                                                /* Overrun error occurred.                                  */
    if ((ESCIA_SR1 & DEF_BIT_03) == DEF_BIT_03) {
        ESCIA_CR1 = DEF_BIT_03;
    }

                                                                /* Byte was received.                                       */
    while ((ESCIA_SR1 & DEF_BIT_05) == DEF_BIT_05) {
        ESCIA_SR1 = DEF_BIT_05;
        rx_data   = (CPU_INT08U)(ESCIA_DRL);
        ProbeRS232_RxHandler(rx_data);
    }

                                                                /* Byte was transmitted.                                    */
    if ((ESCIA_SR1 & DEF_BIT_06) == DEF_BIT_06) {
        ESCIA_SR1 = DEF_BIT_06;
        ProbeRS232_TxHandler();
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)                /* ------------------------- ESCIB ------------------------ */
                                                                /* Overrun error occurred.                                  */
    if ((ESCIB_SR1 & DEF_BIT_03) == DEF_BIT_03) {
        ESCIB_CR1 = DEF_BIT_03;
    }

                                                                /* Byte was received.                                       */
    while ((ESCIB_SR1 & DEF_BIT_05) == DEF_BIT_05) {
        ESCIB_SR1 = DEF_BIT_05;
        rx_data   = (CPU_INT08U)(ESCIB_DRL);
        ProbeRS232_RxHandler(rx_data);
    }

                                                                /* Byte was transmitted.                                    */
    if ((ESCIB_SR1 & DEF_BIT_06) == DEF_BIT_06) {
        ESCIB_SR1 = DEF_BIT_06;
        ProbeRS232_TxHandler();
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)                /* ------------------------- ESCIC ------------------------ */
                                                                /* Overrun error occurred.                                  */
    if ((ESCIC_SR1 & DEF_BIT_03) == DEF_BIT_03) {
        ESCIC_CR1 = DEF_BIT_03;
    }

                                                                /* Byte was received.                                       */
    while ((ESCIC_SR1 & DEF_BIT_05) == DEF_BIT_05) {
        ESCIC_SR1 = DEF_BIT_05;
        rx_data   = (CPU_INT08U)(ESCIC_DRL);
        ProbeRS232_RxHandler(rx_data);
    }

                                                                /* Byte was transmitted.                                    */
    if ((ESCIC_SR1 & DEF_BIT_06) == DEF_BIT_06) {
        ESCIC_SR1 = DEF_BIT_06;
        ProbeRS232_TxHandler();
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)                /* ------------------------- ESCID ------------------------ */
                                                                /* Overrun error occurred.                                  */
    if ((ESCID_SR1 & DEF_BIT_03) == DEF_BIT_03) {
        ESCID_CR1 = DEF_BIT_03;
    }

                                                                /* Byte was received.                                       */
    while ((ESCID_SR1 & DEF_BIT_05) == DEF_BIT_05) {
        ESCID_SR1 = DEF_BIT_05;
        rx_data   = (CPU_INT08U)(ESCID_DRL);
        ProbeRS232_RxHandler(rx_data);
    }

                                                                /* Byte was transmitted.                                    */
    if ((ESCID_SR1 & DEF_BIT_06) == DEF_BIT_06) {
        ESCID_SR1 = DEF_BIT_06;
        ProbeRS232_TxHandler();
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
    ESCIA_CR2 &= ~DEF_BIT_05;
    ESCIA_CR4 &= ~DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    ESCIB_CR2 &= ~DEF_BIT_05;
    ESCIB_CR4 &= ~DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    ESCIC_CR2 &= ~DEF_BIT_05;
    ESCIC_CR4 &= ~DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    ESCID_CR2 &= ~DEF_BIT_05;
    ESCID_CR4 &= ~DEF_BIT_03;
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
    ESCIA_CR2 |= DEF_BIT_05;
    ESCIA_CR4 |= DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    ESCIB_CR2 |= DEF_BIT_05;
    ESCIB_CR4 |= DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    ESCIC_CR2 |= DEF_BIT_05;
    ESCIC_CR4 |= DEF_BIT_03;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    ESCID_CR2 |= DEF_BIT_05;
    ESCID_CR4 |= DEF_BIT_03;
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
    ESCIA_DRL = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    ESCIB_DRL = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    ESCIC_DRL = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    ESCID_DRL = c;
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
    ESCIA_CR2 &= ~DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    ESCIB_CR2 &= ~DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    ESCIC_CR2 &= ~DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    ESCID_CR2 &= ~DEF_BIT_06;
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
    ESCIA_CR2 |= DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    ESCIB_CR2 |= DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    ESCIC_CR2 |= DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    ESCID_CR2 |= DEF_BIT_06;
#endif
}
