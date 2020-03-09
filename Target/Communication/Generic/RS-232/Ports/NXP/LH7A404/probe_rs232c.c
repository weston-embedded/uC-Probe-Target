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
*                                      PORT FOR THE NXP LH7A404
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
#define  LH7A404_REG_CSC_BASE            ((CPU_INT32U)0x80000400)
#define  LH7A404_REG_UART1_BASE          ((CPU_INT32U)0x80000600)
#define  LH7A404_REG_UART2_BASE          ((CPU_INT32U)0x80000700)
#define  LH7A404_REG_UART3_BASE          ((CPU_INT32U)0x80000800)
#define  LH7A404_REG_VIC2_BASE           ((CPU_INT32U)0x8000A000)

                                                                /* ----------- INTERRUPT CONTROLLER SOURCES ----------- */
#define  LH7A404_VIC2_UART1                                6
#define  LH7A404_VIC2_UART2                                8
#define  LH7A404_VIC2_UART3                               10

                                                                /* -------------- UART1 REGISTER DEFINES -------------- */
#define  LH7A404_REG_UART1_DATA     (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0000))
#define  LH7A404_REG_UART1_FCON     (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0004))
#define  LH7A404_REG_UART1_BRCON    (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0008))
#define  LH7A404_REG_UART1_CON      (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x000C))
#define  LH7A404_REG_UART1_STATUS   (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0010))
#define  LH7A404_REG_UART1_RAWISR   (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0014))
#define  LH7A404_REG_UART1_INTEN    (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x0018))
#define  LH7A404_REG_UART1_ISR      (*(volatile CPU_INT32U *)(LH7A404_REG_UART1_BASE + 0x001C))
#define  LH7A404_REG_UART1_RES      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0020))
#define  LH7A404_REG_UART1_EIC      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0024))
#define  LH7A404_REG_UART1_DMACR    (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0028))

                                                                /* -------------- UART2 REGISTER DEFINES -------------- */
#define  LH7A404_REG_UART2_DATA     (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0000))
#define  LH7A404_REG_UART2_FCON     (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0004))
#define  LH7A404_REG_UART2_BRCON    (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0008))
#define  LH7A404_REG_UART2_CON      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x000C))
#define  LH7A404_REG_UART2_STATUS   (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0010))
#define  LH7A404_REG_UART2_RAWISR   (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0014))
#define  LH7A404_REG_UART2_INTEN    (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0018))
#define  LH7A404_REG_UART2_ISR      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x001C))
#define  LH7A404_REG_UART2_RES      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0020))
#define  LH7A404_REG_UART2_EIC      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0024))
#define  LH7A404_REG_UART2_DMACR    (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0028))

                                                                /* -------------- UART3 REGISTER DEFINES -------------- */
#define  LH7A404_REG_UART3_DATA     (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0000))
#define  LH7A404_REG_UART3_FCON     (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0004))
#define  LH7A404_REG_UART3_BRCON    (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0008))
#define  LH7A404_REG_UART3_CON      (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x000C))
#define  LH7A404_REG_UART3_STATUS   (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0010))
#define  LH7A404_REG_UART3_RAWISR   (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0014))
#define  LH7A404_REG_UART3_INTEN    (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x0018))
#define  LH7A404_REG_UART3_ISR      (*(volatile CPU_INT32U *)(LH7A404_REG_UART3_BASE + 0x001C))
#define  LH7A404_REG_UART3_RES      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0020))
#define  LH7A404_REG_UART3_EIC      (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0024))
#define  LH7A404_REG_UART3_DMACR    (*(volatile CPU_INT32U *)(LH7A404_REG_UART2_BASE + 0x0028))

                                                                /* --------------- VIC REGISTER DEFINES --------------- */
#define  LH7A404_REG_VIC2_INTSEL    (*(volatile CPU_INT32U *)(LH7A404_REG_VIC2_BASE  + 0x000C))
#define  LH7A404_REG_VIC2_INTEN     (*(volatile CPU_INT32U *)(LH7A404_REG_VIC2_BASE  + 0x0010))
#define  LH7A404_REG_VIC2_VAD6      (*(volatile CPU_INT32U *)(LH7A404_REG_VIC2_BASE  + 0x0118))
#define  LH7A404_REG_VIC2_VECTCNTL6 (*(volatile CPU_INT32U *)(LH7A404_REG_VIC2_BASE  + 0x0218))

                                                                /* --------------- CSC REGISTER DEFINES --------------- */
#define  LH7A404_REG_CSC_PWRCNT     (*(volatile CPU_INT32U *)(LH7A404_REG_CSC_BASE   + 0x0004))


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
  #error  "                                  [MUST be PROBE_RS232_UART_2]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_2]      "
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
    CPU_INT16U  div;
    CPU_INT32U  clk_freq;

                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    LH7A404_REG_CSC_PWRCNT |= DEF_BIT_29;                       /* UART buad clock source = 14.7456.                    */
    clk_freq            = 14756000;                             /* Get the CPU clock frequency.                         */
    div                 = (CPU_INT16U)((clk_freq / (16 * baud_rate)) - 1);

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* --------------- SETUP VIC FOR UART2 ---------------- */
    LH7A404_REG_VIC2_INTSEL    &= ~(1 << LH7A404_VIC2_UART2);   /* Select IRQ.                                          */
                                                                /* Set the vector address.                              */
    LH7A404_REG_VIC2_VAD6       = (CPU_INT32U)ProbeRS232_RxTxISRHandler;
    LH7A404_REG_VIC2_VECTCNTL6  = 0x20 | LH7A404_VIC2_UART2;    /* Enable vectored interrupts.                          */

                                                                /* ------------------- SETUP UART2 -------------------- */
    LH7A404_REG_UART2_CON       = DEF_BIT_00;                   /* Enable UART.                                         */
    LH7A404_REG_UART2_BRCON     = div;                          /* Init baud rate.                                      */
    LH7A404_REG_UART2_FCON      = DEF_BIT_04                    /* Enable FIFO.                                         */
                                | DEF_BIT_05 | DEF_BIT_06;      /* 8 data bits, 1 stop bit, no parity.                  */
    LH7A404_REG_UART2_DMACR     = 0;                            /* Disable DMA.                                         */
    LH7A404_REG_UART2_EIC       = 0x0F;
    LH7A404_REG_UART2_INTEN     = 0;                            /* Disable all interrupts.                              */

                                                                /* -------------------- ENABLE INT -------------------- */
    LH7A404_REG_VIC2_INTEN      =  (1 << LH7A404_VIC2_UART2);   /* Enable interrupt.                                    */
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
    CPU_INT32U  isr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)

    isr = LH7A404_REG_UART2_ISR;

    while (isr != 0) {
        if (isr & DEF_BIT_01) {                                 /* Transmitted character?                               */
             ProbeRS232_TxHandler();
        }

        if (isr & (DEF_BIT_00 | DEF_BIT_03)) {                  /* Received a character?                                */
            rx_data = (CPU_INT08U)(LH7A404_REG_UART2_DATA & 0xFF);
            ProbeRS232_RxHandler(rx_data);
        }

        isr = LH7A404_REG_UART2_ISR;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LH7A404_REG_UART2_INTEN &= ~(DEF_BIT_00 | DEF_BIT_03);
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LH7A404_REG_UART2_INTEN |= DEF_BIT_00 | DEF_BIT_03;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LH7A404_REG_UART2_DATA = tx_data;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LH7A404_REG_UART2_INTEN &= ~DEF_BIT_01;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LH7A404_REG_UART2_INTEN |= DEF_BIT_01;
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
