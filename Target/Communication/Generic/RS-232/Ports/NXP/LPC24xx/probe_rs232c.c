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
*                                      PORT FOR THE NXP LPC24xx
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
                                                                /* ------------- REGISTER BASE SPECIFICATIONS ------------- */
#define  U0_BASE                ((CPU_INT32U)0xE000C000)
#define  U1_BASE                ((CPU_INT32U)0xE0010000)
#define  U2_BASE                ((CPU_INT32U)0xE0078000)
#define  U3_BASE                ((CPU_INT32U)0xE007C000)
#define  VIC_BASE               ((CPU_INT32U)0xFFFFF000)
#define  PINSEL_BASE            ((CPU_INT32U)0xE002C000)

                                                                /* ------------------ VIC PERIPHERAL IDs ------------------ */
#define  VIC_UART0                                         6
#define  VIC_UART1                                         7
#define  VIC_UART2                                        28
#define  VIC_UART3                                        29

                                                                /* ----------------------- PCLK IDs ----------------------- */
#define  PCLK_UART0                                        3
#define  PCLK_UART1                                        4
#define  PCLK_UART2                                       24
#define  PCLK_UART3                                       25

                                                                /* ------------------- PCONP BIT DEFINES ------------------ */
#define  PCUART0                                  DEF_BIT_03
#define  PCUART1                                  DEF_BIT_04
#define  PCUART2                                  DEF_BIT_24
#define  PCUART3                                  DEF_BIT_25

                                                                /* -------- CLOCKING AND POWER CONTROL REGISTER DEFINES --- */
#define  PCONP              (*(volatile CPU_INT32U *)(0xE01FC0C4))


                                                                /* ---------------- UART0 REGISTER DEFINES ---------------- */
#define  U0RBR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0THR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0DLL              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0000))
#define  U0DLM              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0004))
#define  U0IER              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0004))
#define  U0IIR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0008))
#define  U0FCR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0008))
#define  U0LCR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x000C))
#define  U0LSR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0014))
#define  U0FDR              (*(volatile CPU_INT08U *)(U0_BASE     + 0x0028))

                                                                /* ---------------- UART1 REGISTER DEFINES ---------------- */
#define  U1RBR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1THR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1DLL              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0000))
#define  U1DLM              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0004))
#define  U1IER              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0004))
#define  U1IIR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0008))
#define  U1FCR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0008))
#define  U1LCR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x000C))
#define  U1LSR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0014))
#define  U1FDR              (*(volatile CPU_INT08U *)(U1_BASE     + 0x0028))

                                                                /* ---------------- UART2 REGISTER DEFINES ---------------- */
#define  U2RBR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0000))
#define  U2THR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0000))
#define  U2DLL              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0000))
#define  U2DLM              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0004))
#define  U2IER              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0004))
#define  U2IIR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0008))
#define  U2FCR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0008))
#define  U2LCR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x000C))
#define  U2LSR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0014))
#define  U2FDR              (*(volatile CPU_INT08U *)(U2_BASE     + 0x0028))

                                                                /* ---------------- UART3 REGISTER DEFINES ---------------- */
#define  U3RBR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0000))
#define  U3THR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0000))
#define  U3DLL              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0000))
#define  U3DLM              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0004))
#define  U3IER              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0004))
#define  U3IIR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0008))
#define  U3FCR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0008))
#define  U3LCR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x000C))
#define  U3LSR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0014))
#define  U3FDR              (*(volatile CPU_INT08U *)(U3_BASE     + 0x0028))

                                                                /* ---------- PIN CONNECT BLOCK REGISTER DEFINES ---------- */
#define  PINSEL0            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x0000))
#define  PINSEL1            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x0004))
#define  PINSEL7            (*(volatile CPU_INT32U *)(PINSEL_BASE + 0x001C))

                                                                /* ----------------- VIC REGISTER DEFINES ----------------- */
#define  VICIntSelect       (*(volatile CPU_INT32U *)(VIC_BASE    + 0x000C))
#define  VICIntEnable       (*(volatile CPU_INT32U *)(VIC_BASE    + 0x0010))
#define  VICVectAddr(n)     (*(volatile CPU_INT32U *)(VIC_BASE    + 0x0100 + (4 * (n))))
#define  VICVectCntl(n)     (*(volatile CPU_INT32U *)(VIC_BASE    + 0x0200 + (4 * (n))))


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    EXTERNAL FUNCTION PROTOTYPES
*
* Note(s)     : (1) 'BSP_CPU_PclkFreq()' should return the peripheral clock frequency in Hz of the
*                   peripheral corresponding to id 'pclk'.
*********************************************************************************************************
*/

extern  CPU_INT32U  BSP_CPU_PclkFreq  (CPU_INT08U  pclk);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
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
    CPU_INT08U  divlo;
    CPU_INT08U  divhi;
    CPU_INT32U  pinsel;
    CPU_INT32U  pclk_freq;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    PCONP    |= PCUART0;                                        /* Enable UART0 peripheral.                                 */

                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    pclk_freq =  BSP_CPU_PclkFreq(PCLK_UART0);                  /* Get the CPU clock frequency                              */
    div       = (CPU_INT16U)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
    divlo     =  div & 0x00FF;                                  /* Split divisor into LOW and HIGH bytes                    */
    divhi     = (div >> 8) & 0x00FF;

                                                                /* ------------------- ENABLE UART0 I/Os ------------------ */
                                                                /* (1) P0[2]  = Function 01b                                */
                                                                /* (2) P0[3]  = Function 01b                                */
    pinsel    = PINSEL0;
    pinsel   &= 0xFFFFFF0F;
    pinsel   |= 0x00000050;
    PINSEL0   = pinsel;

                                                                /* --------------------- SETUP UART0 ---------------------- */
    U0LCR     = DEF_BIT_07;                                     /* Set divisor access bit                                   */
    U0DLL     = divlo;                                          /* Load divisor                                             */
    U0DLM     = divhi;
    U0LCR     = 0x03;                                           /* Set line control register (Bit 8 is 0)                   */
    U0IER     = 0x00;                                           /* Disable both Rx and Tx interrupts                        */
    U0FDR     = 0x00;                                           /* Disable fractional dividers                              */
    U0FCR     = 0x01;                                           /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART0 --------------- */
    VICIntSelect           &= ~(1 << VIC_UART0);                        /* Enable interrupts                                */
    VICVectAddr(VIC_UART0)  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;    /* Set the vector address                           */
    VICVectCntl(VIC_UART0)  =  14;
    VICIntEnable            =  (1 << VIC_UART0);                        /* Enable Interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    PCONP    |= PCUART1;                                        /* Enable UART1 peripheral.                                 */

                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    pclk_freq =  BSP_CPU_PclkFreq(PCLK_UART1);                  /* Get the CPU clock frequency                              */
    div       = (CPU_INT16U)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
    divlo     =  div & 0x00FF;                                  /* Split divisor into LOW and HIGH bytes                    */
    divhi     = (div >> 8) & 0x00FF;

                                                                /* ------------------- ENABLE UART1 I/Os ------------------ */
#ifdef  PROBE_RS232_LPC24XX_UART1_P3_16_17
                                                                /* (1) P3[16] = Function 11b                                */
                                                                /* (2) P3[17] = Function 11b                                */
    pinsel    = PINSEL7;
    pinsel   &= 0xFFFFFFF0;
    pinsel   |= 0x0000000F;
    PINSEL7   = pinsel;

#else
                                                                /* (1) P0[15] = Function 01b                                */
                                                                /* (2) P0[16] = Function 01b                                */
    pinsel    = PINSEL0;
    pinsel   &= 0x3FFFFFFF;
    pinsel   |= 0x40000000;
    PINSEL0   = pinsel;

    pinsel    = PINSEL1;
    pinsel   &= 0xFFFFFFFC;
    pinsel   |= 0x00000001;
    PINSEL1   = pinsel;
#endif


                                                                /* --------------------- SETUP UART1 ---------------------- */
    U1LCR     = DEF_BIT_07;                                     /* Set divisor access bit                                   */
    U1DLL     = divlo;                                          /* Load divisor                                             */
    U1DLM     = divhi;
    U1LCR     = 0x03;                                           /* 8 Bits, 1 Stop, No Parity                                */
    U1IER     = 0x00;                                           /* Disable both Rx and Tx interrupts                        */
    U1FDR     = 0x00;                                           /* Disable fractional dividers                              */    
    U1FCR     = 0x01;                                           /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART1 --------------- */
    VICIntSelect           &= ~(1 << VIC_UART1);                        /* Enable interrupts                                */
    VICVectAddr(VIC_UART1)  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;    /* Set the vector address                           */
    VICVectCntl(VIC_UART1)  =  14;
    VICIntEnable            =  (1 << VIC_UART1);                        /* Enable Interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    PCONP    |= PCUART2;                                        /* Enable UART2 peripheral.                                 */

                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    pclk_freq =  BSP_CPU_PclkFreq(PCLK_UART2);                  /* Get the CPU clock frequency                              */
    div       = (CPU_INT16U)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
    divlo     =  div & 0x00FF;                                  /* Split divisor into LOW and HIGH bytes                    */
    divhi     = (div >> 8) & 0x00FF;

                                                                /* ------------------- ENABLE UART2 I/Os ------------------ */
                                                                /* (1) P0[10] = Function 01b                                */
                                                                /* (2) P0[11] = Function 01b                                */
    pinsel    = PINSEL0;
    pinsel   &= 0xFF0FFFFF;
    pinsel   |= 0x00500000;
    PINSEL0   = pinsel;

                                                                /* --------------------- SETUP UART2 ---------------------- */
    U2LCR     = DEF_BIT_07;                                     /* Set divisor access bit                                   */
    U2DLL     = divlo;                                          /* Load divisor                                             */
    U2DLM     = divhi;
    U2LCR     = 0x03;                                           /* 8 Bits, 1 Stop, No Parity                                */
    U2IER     = 0x00;                                           /* Disable both Rx and Tx interrupts                        */
    U2FDR     = 0x00;                                           /* Disable fractional dividers                              */    
    U2FCR     = 0x01;                                           /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART2 --------------- */
    VICIntSelect           &= ~(1 << VIC_UART2);                        /* Enable interrupts                                */
    VICVectAddr(VIC_UART2)  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;    /* Set the vector address                           */
    VICVectCntl(VIC_UART2)  =  14;
    VICIntEnable            =  (1 << VIC_UART2);                        /* Enable Interrupts                                */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    PCONP    |= PCUART3;                                        /* Enable UART3 peripheral.                                 */

                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    pclk_freq =  BSP_CPU_PclkFreq(PCLK_UART3);                  /* Get the CPU clock frequency                              */
    div       = (CPU_INT16U)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
    divlo     =  div & 0x00FF;                                  /* Split divisor into LOW and HIGH bytes                    */
    divhi     = (div >> 8) & 0x00FF;

                                                                /* ------------------- ENABLE UART3 I/Os ------------------ */
                                                                /* (1) P0[0]  = Function 11b                                */
                                                                /* (2) P0[1]  = Function 11b                                */
    pinsel    = PINSEL0;
    pinsel   &= 0xFFFFFFF0;
    pinsel   |= 0x0000000F;
    PINSEL0   = pinsel;

                                                                /* --------------------- SETUP UART3 ---------------------- */
    U3LCR     = DEF_BIT_07;                                     /* Set divisor access bit                                   */
    U3DLL     = divlo;                                          /* Load divisor                                             */
    U3DLM     = divhi;
    U3LCR     = 0x03;                                           /* 8 Bits, 1 Stop, No Parity                                */
    U3IER     = 0x00;                                           /* Disable both Rx and Tx interrupts                        */
    U3FCR     = 0x01;                                           /* Enable FIFO, flush Rx & Tx                               */

                                                                /* --------------- INITIALIZE VIC FOR UART3 --------------- */
    VICIntSelect           &= ~(1 << VIC_UART3);                        /* Enable interrupts                                */
    VICVectAddr(VIC_UART3)  = (CPU_INT32U)ProbeRS232_RxTxISRHandler;    /* Set the vector address                           */
    VICVectCntl(VIC_UART3)  =  14;
    VICIntEnable            =  (1 << VIC_UART3);                        /* Enable Interrupts                                */
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



#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)                /* ------------------------- UART0 ------------------------ */
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)                /* ------------------------- UART1 ------------------------ */
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)                /* ------------------------- UART2 ------------------------ */
    iir = U2IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 lsr     = U2LSR;
                 rx_data = U2RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                              */
                 break;

            default:
                 break;
        }
        iir = U2IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)                /* ------------------------- UART3 ------------------------ */
    iir = U3IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 lsr     = U3LSR;
                 rx_data = U3RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                              */
                 break;

            default:
                 break;
        }
        iir = U3IIR & 0x0F;
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
    U0IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    U1IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2IER &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    U3IER &= ~DEF_BIT_00;
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2IER |= DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    U3IER |= DEF_BIT_00;
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2THR = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    U3THR = c;
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2IER &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    U3IER &= ~DEF_BIT_01;
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

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    U2IER |= DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    U3IER |= DEF_BIT_01;
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
