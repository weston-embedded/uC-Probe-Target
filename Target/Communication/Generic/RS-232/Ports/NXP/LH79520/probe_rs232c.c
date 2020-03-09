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
*                                      PORT FOR THE NXP LH79520
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
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* ----------- REGISTER BASE SPECIFICATIONS ----------- */
#define  LH79520_REG_UART0_BASE          ((CPU_INT32U)0xFFFC0000)
#define  LH79520_REG_UART1_BASE          ((CPU_INT32U)0xFFFC1000)
#define  LH79520_REG_UART2_BASE          ((CPU_INT32U)0xFFFC2000)
#define  LH79520_REG_IOCON_BASE          ((CPU_INT32U)0xFFFE5000)
#define  LH79520_REG_RCPC_BASE           ((CPU_INT32U)0xFFFE2000)
#define  LH79520_REG_VIC_BASE            ((CPU_INT32U)0xFFFFF000)

                                                                /* ----------- INTERRUPT CONTROLLER SOURCES ----------- */
#define  LH79520_VIC_UART0                                23
#define  LH79520_VIC_UART1                                24
#define  LH79520_VIC_UART2                                25

                                                                /* -------------- UART0 REGISTER DEFINES -------------- */
#define  LH79520_REG_UART0_DR       (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0000))
#define  LH79520_REG_UART0_RSR      (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0004))
#define  LH79520_REG_UART0_ECR      (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0004))
#define  LH79520_REG_UART0_FR       (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0018))
#define  LH79520_REG_UART0_ILPR     (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0020))
#define  LH79520_REG_UART0_IBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0024))
#define  LH79520_REG_UART0_FBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0028))
#define  LH79520_REG_UART0_LCR_H    (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x002C))
#define  LH79520_REG_UART0_CR       (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0030))
#define  LH79520_REG_UART0_IFLS     (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0034))
#define  LH79520_REG_UART0_IMSC     (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0038))
#define  LH79520_REG_UART0_RIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x003C))
#define  LH79520_REG_UART0_MIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0040))
#define  LH79520_REG_UART0_ICR      (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0044))
#define  LH79520_REG_UART0_DMACTRL  (*(volatile CPU_INT32U *)(LH79520_REG_UART0_BASE + 0x0048))

                                                                /* -------------- UART1 REGISTER DEFINES -------------- */
#define  LH79520_REG_UART1_DR       (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0000))
#define  LH79520_REG_UART1_RSR      (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0004))
#define  LH79520_REG_UART1_ECR      (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0004))
#define  LH79520_REG_UART1_FR       (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0018))
#define  LH79520_REG_UART1_ILPR     (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0020))
#define  LH79520_REG_UART1_IBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0024))
#define  LH79520_REG_UART1_FBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0028))
#define  LH79520_REG_UART1_LCR_H    (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x002C))
#define  LH79520_REG_UART1_CR       (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0030))
#define  LH79520_REG_UART1_IFLS     (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0034))
#define  LH79520_REG_UART1_IMSC     (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0038))
#define  LH79520_REG_UART1_RIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x003C))
#define  LH79520_REG_UART1_MIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0040))
#define  LH79520_REG_UART1_ICR      (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0044))
#define  LH79520_REG_UART1_DMACTRL  (*(volatile CPU_INT32U *)(LH79520_REG_UART1_BASE + 0x0048))

                                                                /* -------------- UART2 REGISTER DEFINES -------------- */
#define  LH79520_REG_UART2_DR       (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0000))
#define  LH79520_REG_UART2_RSR      (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0004))
#define  LH79520_REG_UART2_ECR      (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0004))
#define  LH79520_REG_UART2_FR       (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0018))
#define  LH79520_REG_UART2_ILPR     (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0020))
#define  LH79520_REG_UART2_IBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0024))
#define  LH79520_REG_UART2_FBRD     (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0028))
#define  LH79520_REG_UART2_LCR_H    (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x002C))
#define  LH79520_REG_UART2_CR       (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0030))
#define  LH79520_REG_UART2_IFLS     (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0034))
#define  LH79520_REG_UART2_IMSC     (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0038))
#define  LH79520_REG_UART2_RIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x003C))
#define  LH79520_REG_UART2_MIS      (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0040))
#define  LH79520_REG_UART2_ICR      (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0044))
#define  LH79520_REG_UART2_DMACTRL  (*(volatile CPU_INT32U *)(LH79520_REG_UART2_BASE + 0x0048))

                                                                /* --------------- VIC REGISTER DEFINES --------------- */
#define  LH79520_REG_VIC_INTSEL     (*(volatile CPU_INT32U *)(LH79520_REG_VIC_BASE   + 0x000C))
#define  LH79520_REG_VIC_INTEN      (*(volatile CPU_INT32U *)(LH79520_REG_VIC_BASE   + 0x0010))
#define  LH79520_REG_VIC_INTENCLR   (*(volatile CPU_INT32U *)(LH79520_REG_VIC_BASE   + 0x0014))
#define  LH79520_REG_VIC_VECTADDR0  (*(volatile CPU_INT32U *)(LH79520_REG_VIC_BASE   + 0x0100))
#define  LH79520_REG_VIC_VECTCNTL0  (*(volatile CPU_INT32U *)(LH79520_REG_VIC_BASE   + 0x0200))

                                                                /* --------------- RCPC REGISTER DEFINES -------------- */
#define  LH79520_REG_RCPC_PCLKCTRL1 (*(volatile CPU_INT32U *)(LH79520_REG_RCPC_BASE  + 0x0024))
#define  LH79520_REG_RCPC_PCLKSEL1  (*(volatile CPU_INT32U *)(LH79520_REG_RCPC_BASE  + 0x0030))

                                                                /* --------------- IOCON REGISTER DEFINES ------------- */
#define  LH79520_REG_IOCON_UARTMUX  (*(volatile CPU_INT32U *)(LH79520_REG_IOCON_BASE + 0x0010))


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
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
    CPU_INT32U  idiv;
    CPU_INT32U  fdiv;
    CPU_INT32U  clk_freq;


                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    clk_freq = 14745600;
    idiv     = clk_freq / (16 * baud_rate);
    fdiv     = clk_freq % (16 * baud_rate);
    fdiv     = ((((8 * fdiv) / baud_rate) + 1) / 2);

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* --------------- SETUP VIC FOR UART0 ---------------- */
    LH79520_REG_VIC_INTENCLR    =   1 << LH79520_VIC_UART1;     /* Disable interrupt.                                   */
    LH79520_REG_VIC_INTSEL     &= ~(1 << LH79520_VIC_UART1);    /* Select  interrupt.                                   */
                                                                /* Set the vector address.                              */
    LH79520_REG_VIC_VECTADDR0   = (CPU_INT32U)ProbeRS232_RxTxISRHandler;
    LH79520_REG_VIC_VECTCNTL0   = 0x20 | LH79520_VIC_UART1;     /* Enable vectored interrupts.                          */

                                                                /* ----------------- IOCON FOR UART0 ------------------ */
    LH79520_REG_IOCON_UARTMUX  |= DEF_BIT_02 | DEF_BIT_03;

                                                                /* ------------------- SETUP UART0 -------------------- */
    LH79520_REG_RCPC_PCLKCTRL1 &= ~DEF_BIT_01;                  /* Enable clock.                                        */
    LH79520_REG_RCPC_PCLKSEL1  &= ~DEF_BIT_01;                  /* Select oscillator clock.                             */

    LH79520_REG_UART1_IBRD      =  idiv;
    LH79520_REG_UART1_FBRD      =  fdiv;

    LH79520_REG_UART1_LCR_H     = DEF_BIT_05 | DEF_BIT_06;      /* 8 data bits.                                         */
    LH79520_REG_UART1_IFLS      = DEF_BIT_NONE;
    LH79520_REG_UART1_FR        = DEF_BIT_NONE;
    LH79520_REG_UART1_CR        = DEF_BIT_00
                            | DEF_BIT_08
                            | DEF_BIT_09;
    LH79520_REG_UART1_DMACTRL   = DEF_BIT_NONE;
    LH79520_REG_UART1_ECR       = DEF_BIT_NONE;
    LH79520_REG_UART1_IMSC      = DEF_BIT_NONE;                 /* Disable all interrupts.                              */
    LH79520_REG_UART1_ICR       = DEF_BIT_04                    /* Clear   all interrupts.                              */
                            | DEF_BIT_05
                            | DEF_BIT_06
                            | DEF_BIT_07
                            | DEF_BIT_08
                            | DEF_BIT_09
                            | DEF_BIT_10;

                                                                /* -------------------- ENABLE INT -------------------- */
    LH79520_REG_VIC_INTEN       =  (1 << LH79520_VIC_UART1);    /* Enable interrupt.                                    */
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
    CPU_INT08U  rx_data;
    CPU_INT32U  status;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    status = LH79520_REG_UART1_MIS;

    if ((status & DEF_BIT_05) == DEF_BIT_05) {                  /* Transmitted character?                               */
        while (((LH79520_REG_UART1_IMSC & DEF_BIT_05) == DEF_BIT_05) &&
               ((LH79520_REG_UART1_FR   & DEF_BIT_05) == 0)) {
            ProbeRS232_TxHandler();
        }
    }

    if (((status & DEF_BIT_04) == DEF_BIT_04) ||                /* Received a character?                                */
        ((status & DEF_BIT_06) == DEF_BIT_06)) {

        while ((LH79520_REG_UART1_FR & DEF_BIT_04) == 0) {
            rx_data = (CPU_INT08U)(LH79520_REG_UART1_DR & 0xFF);
            ProbeRS232_RxHandler(rx_data);
        }
    }

    LH79520_REG_UART1_ICR = status;
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
    LH79520_REG_UART1_IMSC &= ~(DEF_BIT_04 | DEF_BIT_06);
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
    LH79520_REG_UART1_IMSC |= DEF_BIT_04 | DEF_BIT_06;
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

void  ProbeRS232_Tx1 (CPU_INT08U tx_data)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    LH79520_REG_UART1_DR = tx_data;
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
    LH79520_REG_UART1_IMSC &= ~DEF_BIT_05;
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
    LH79520_REG_UART1_IMSC |= DEF_BIT_05;
#endif
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
