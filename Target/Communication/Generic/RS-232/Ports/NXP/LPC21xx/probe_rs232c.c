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
*                                      PORT FOR THE NXP LPC21xx
*
* Filename : probe_rs232c.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) The UARTs on all NXP LPC21xx and LPC22xx controllers are supported by this port.
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
                                                                /* ------------- Register Base Specifications ------------- */
#define  U0_BASE                ((CPU_INT32U)0xE000C000)
#define  U1_BASE                ((CPU_INT32U)0xE0010000)
#define  VIC_BASE               ((CPU_INT32U)0xFFFFF000)
#define  PINSEL_BASE            ((CPU_INT32U)0xE002C000)

                                                                /* ------------------ VIC Peripheral IDs ------------------ */
#define  VIC_UART0                      6
#define  VIC_UART1                      7

                                                                /* ---------------- UART0 Register Defines ---------------- */
#define  U0RBR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0THR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0DLL              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0DLM              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0004))
#define  U0IER              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0004))
#define  U0IIR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0008))
#define  U0FCR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0008))
#define  U0LCR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x000C))
#define  U0LSR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0014))

                                                                /* ---------------- UART1 Register Defines ---------------- */
#define  U1RBR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1THR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1DLL              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1DLM              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0004))
#define  U1IER              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0004))
#define  U1IIR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0008))
#define  U1FCR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0008))
#define  U1LCR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x000C))
#define  U1LSR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0014))

                                                                /* ---------- Pin Connect Block Register Defines ---------- */
#define  PINSEL0            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x0000))
#define  PINSEL1            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x0004))
#define  PINSEL2            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x0014))

                                                                /* ----------------- VIC Register Defines ----------------- */
#define  VICIntSelect       (*(volatile CPU_INT32U *)(VIC_BASE    + 0x000C))
#define  VICIntEnable       (*(volatile CPU_INT32U *)(VIC_BASE    + 0x0010))
#define  VICVectAddr        (*(volatile CPU_INT32U *)(VIC_BASE    + 0x0030))
#define  VICVectAddr15      (*(volatile CPU_INT32U *)(VIC_BASE    + 0x013C))
#define  VICVectCntl15      (*(volatile CPU_INT32U *)(VIC_BASE    + 0x023C))


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    EXTERNAL FUNCTION PROTOTYPES
*
* Note(s)     : (1) 'BSP_PclkFreq()' should return the peripheral frequency in Hz.
*********************************************************************************************************
*/

extern  CPU_INT32U  BSP_CPU_PclkFreq   (void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0  ) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1  )

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
    CPU_INT16U  div;                                            /* Baud rate divisor                                        */
    CPU_INT08U  divlo;
    CPU_INT08U  divhi;
    CPU_INT32U  pinsel;
    CPU_INT32U  pclk_freq;


                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    pclk_freq = BSP_CPU_PclkFreq();                             /* Get the CPU clock frequency                              */
    div       = (CPU_INT16U)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
    divlo     =  div & 0x00FF;                                  /* Split divisor into LOW and HIGH bytes                    */
    divhi     = (div >> 8) & 0x00FF;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ------------------- ENABLE UART0 I/Os ------------------ */
                                                                /* (1) P0[0]  = Function 01b                                */
                                                                /* (2) P0[1]  = Function 01b                                */
    pinsel   = PINSEL0;
    pinsel  &= 0xFFFFFFF0;
    pinsel  |= 0x00000005;
    PINSEL0  = pinsel;

                                                                /* --------------------- SETUP UART0 ---------------------- */
    U0LCR    = DEF_BIT_07;                                      /* Set divisor access bit                                   */
    U0DLL    = divlo;                                           /* Load divisor                                             */
    U0DLM    = divhi;
    U0LCR    = 0x03;                                            /* 8 Bits, 1 Stop, No Parity                                */
    U0IER    = 0x00;                                            /* Disable both Rx and Tx interrupts                        */
    U0FCR    = DEF_BIT_00;                                      /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART0 --------------- */
    VICIntSelect  &= ~(1 << VIC_UART0);                         /* Enable interrupts                                        */
    VICVectAddr15  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;     /* Set the vector address                                   */
    VICVectCntl15  = 0x20 | VIC_UART0;                          /* Enable vectored interrupts                               */
    VICIntEnable   =  (1 << VIC_UART0);                         /* Enable Interrupts                                        */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)

                                                                /* ------------------- ENABLE UART1 I/Os ------------------ */
                                                                /* (1) P0[8]  = Function 01b                                */
                                                                /* (2) P0[9]  = Function 01b                                */
    pinsel   = PINSEL0;
    pinsel  &= 0xFFF0FFFF;
    pinsel  |= 0x00050000;
    PINSEL0  = pinsel;

                                                                /* --------------------- SETUP UART1 ---------------------- */
    U1LCR    = DEF_BIT_07;                                      /* Set divisor access bit                                   */
    U1DLL    = divlo;                                           /* Load divisor                                             */
    U1DLM    = divhi;
    U1LCR    = 0x03;                                            /* 8 Bits, 1 Stop, No Parity                                */
    U1IER    = 0x00;                                            /* Disable both Rx and Tx interrupts                        */
    U1FCR    = DEF_BIT_00;                                      /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART1 --------------- */
    VICIntSelect  &= ~(1 << VIC_UART1);                         /* Enable interrupts                                        */
    VICVectAddr15  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;     /* Set the vector address                                   */
    VICVectCntl15  = 0x20 | VIC_UART1;                          /* Enable vectored interrupts                               */
    VICIntEnable   =  (1 << VIC_UART1);                         /* Enable Interrupts                                        */
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
    volatile  CPU_INT08U  rx_data;
    volatile  CPU_INT08U  lsr;
    volatile  CPU_INT08U  iir;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    iir = U0IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 lsr     = U0LSR;
                 rx_data = U0RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                              */
                 break;

            default:
                 break;
        }
        iir = U0IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    iir = U1IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 lsr     = U1LSR;
                 rx_data = U1RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                              */
                 break;

            default:
                 break;
        }
        iir = U1IIR & 0x0F;
    }
#endif

    VICVectAddr = 0x00000000L;                                  /* Clear the vector address register                        */
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
    U0IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1IER &= ~DEF_BIT_00;
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
    U0IER |= DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1IER |= DEF_BIT_00;
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
    U0THR = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1THR = c;
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
    U0IER &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1IER &= ~DEF_BIT_01;
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
    U0IER |= DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1IER |= DEF_BIT_01;
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
