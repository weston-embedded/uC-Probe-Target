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
*                                      PORT FOR THE NXP LPC3xxx
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
#include  <bsp.h>


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

/*
*********************************************************************************************************
*                                      LPC3xxx REGISTER DEFINES
*********************************************************************************************************
*/
                                                                /* ----------- REGISTER BASE SPECIFICATIONS ----------- */
#define  LPC3xxx_REG_HSU1_BASE              ((CPU_INT32U)0x40014000)
#define  LPC3xxx_REG_HSU2_BASE              ((CPU_INT32U)0x40018000)
#define  LPC3xxx_REG_U3_BASE                ((CPU_INT32U)0x40080000)
#define  LPC3xxx_REG_U4_BASE                ((CPU_INT32U)0x40088000)
#define  LPC3xxx_REG_U5_BASE                ((CPU_INT32U)0x40090000)
#define  LPC3xxx_REG_U6_BASE                ((CPU_INT32U)0x40098000)
#define  LPC3xxx_REG_HSU7_BASE              ((CPU_INT32U)0x4001C000)

                                                                /* -------- HIGH-SPEED UART1 REGISTER DEFINES --------- */
#define  LPC3xxx_REG_HSU1_RX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU1_BASE + 0x0000))
#define  LPC3xxx_REG_HSU1_TX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU1_BASE + 0x0000))
#define  LPC3xxx_REG_HSU1_LEVEL         (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU1_BASE + 0x0004))
#define  LPC3xxx_REG_HSU1_IIR           (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU1_BASE + 0x0008))
#define  LPC3xxx_REG_HSU1_CTRL          (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU1_BASE + 0x000C))
#define  LPC3xxx_REG_HSU1_RATE          (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU1_BASE + 0x0010))

                                                                /* -------- HIGH-SPEED UART2 REGISTER DEFINES --------- */
#define  LPC3xxx_REG_HSU2_RX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU2_BASE + 0x0000))
#define  LPC3xxx_REG_HSU2_TX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU2_BASE + 0x0000))
#define  LPC3xxx_REG_HSU2_LEVEL         (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU2_BASE + 0x0004))
#define  LPC3xxx_REG_HSU2_IIR           (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU2_BASE + 0x0008))
#define  LPC3xxx_REG_HSU2_CTRL          (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU2_BASE + 0x000C))
#define  LPC3xxx_REG_HSU2_RATE          (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU2_BASE + 0x0010))

                                                                /* -------------- UART3 REGISTER DEFINES -------------- */
#define  LPC3xxx_REG_U3RBR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0000))
#define  LPC3xxx_REG_U3THR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0000))
#define  LPC3xxx_REG_U3DLL              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0000))
#define  LPC3xxx_REG_U3DLM              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0004))
#define  LPC3xxx_REG_U3IER              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0004))
#define  LPC3xxx_REG_U3IIR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0008))
#define  LPC3xxx_REG_U3FCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0008))
#define  LPC3xxx_REG_U3LCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x000C))
#define  LPC3xxx_REG_U3LSR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U3_BASE   + 0x0014))

                                                                /* -------------- UART4 REGISTER DEFINES -------------- */
#define  LPC3xxx_REG_U4RBR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0000))
#define  LPC3xxx_REG_U4THR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0000))
#define  LPC3xxx_REG_U4DLL              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0000))
#define  LPC3xxx_REG_U4DLM              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0004))
#define  LPC3xxx_REG_U4IER              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0004))
#define  LPC3xxx_REG_U4IIR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0008))
#define  LPC3xxx_REG_U4FCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0008))
#define  LPC3xxx_REG_U4LCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x000C))
#define  LPC3xxx_REG_U4LSR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U4_BASE   + 0x0014))

                                                                /* -------------- UART5 REGISTER DEFINES -------------- */
#define  LPC3xxx_REG_U5RBR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0000))
#define  LPC3xxx_REG_U5THR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0000))
#define  LPC3xxx_REG_U5DLL              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0000))
#define  LPC3xxx_REG_U5DLM              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0004))
#define  LPC3xxx_REG_U5IER              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0004))
#define  LPC3xxx_REG_U5IIR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0008))
#define  LPC3xxx_REG_U5FCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0008))
#define  LPC3xxx_REG_U5LCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x000C))
#define  LPC3xxx_REG_U5LSR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U5_BASE   + 0x0014))

                                                                /* -------------- UART6 REGISTER DEFINES -------------- */
#define  LPC3xxx_REG_U6RBR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0000))
#define  LPC3xxx_REG_U6THR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0000))
#define  LPC3xxx_REG_U6DLL              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0000))
#define  LPC3xxx_REG_U6DLM              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0004))
#define  LPC3xxx_REG_U6IER              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0004))
#define  LPC3xxx_REG_U6IIR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0008))
#define  LPC3xxx_REG_U6FCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0008))
#define  LPC3xxx_REG_U6LCR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x000C))
#define  LPC3xxx_REG_U6LSR              (*(volatile CPU_INT08U *)(LPC3xxx_REG_U6_BASE   + 0x0014))


                                                                /* -------- HIGH-SPEED UART7 REGISTER DEFINES --------- */
#define  LPC3xxx_REG_HSU7_RX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU7_BASE + 0x0000))
#define  LPC3xxx_REG_HSU7_TX            (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU7_BASE + 0x0000))
#define  LPC3xxx_REG_HSU7_LEVEL         (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU7_BASE + 0x0004))
#define  LPC3xxx_REG_HSU7_IIR           (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU7_BASE + 0x0008))
#define  LPC3xxx_REG_HSU7_CTRL          (*(volatile CPU_INT32U *)(LPC3xxx_REG_HSU7_BASE + 0x000C))
#define  LPC3xxx_REG_HSU7_RATE          (*(volatile CPU_INT08U *)(LPC3xxx_REG_HSU7_BASE + 0x0010))


                                                                /* ----- ADDITIONAL UART CONTROL REGISTER DEFINES ----- */
#define  LPC3xxx_REG_U3CLK              (*(volatile CPU_INT32U *)(0x400040D0))
#define  LPC3xxx_REG_U4CLK              (*(volatile CPU_INT32U *)(0x400040D4))
#define  LPC3xxx_REG_U5CLK              (*(volatile CPU_INT32U *)(0x400040D8))
#define  LPC3xxx_REG_U6CLK              (*(volatile CPU_INT32U *)(0x400040DC))

                                                                /* ----- COMMON UART CONTROL REGISTER DEFINES --------- */
#define  LPC3xxx_REG_UART_CTRL          (*(volatile CPU_INT32U *)(0x40054000))
#define  LPC3xxx_REG_UART_CLKMODE       (*(volatile CPU_INT32U *)(0x40054004))
#define  LPC3xxx_REG_UART_LOOP          (*(volatile CPU_INT32U *)(0x40054008))
#define  LPC3xxx_REG_UARTCLK_CTRL       (*(volatile CPU_INT32U *)(0x400040E4))

/*
*********************************************************************************************************
*                                        REGISTER BITS DEFINES
*********************************************************************************************************
*/
                                                                             /* ----------- COMMON UART CONTROL BIT DEFINES ------------ */
#define  LCP3xxx_REG_UART_CLKMODE_UART3_CLK_ON          DEF_BIT_04
#define  LPC3xxx_REG_UART_CLKMODE_UART3_CLK_AUTO        DEF_BIT_05
#define  LCP3xxx_REG_UART_CLKMODE_UART3_CLK_MASK        DEF_BIT_04 | \
                                                        DEF_BIT_05

#define  LCP3xxx_REG_UART_CLKMODE_UART4_CLK_ON          DEF_BIT_06
#define  LPC3xxx_REG_UART_CLKMODE_UART4_CLK_AUTO        DEF_BIT_07
#define  LCP3xxx_REG_UART_CLKMODE_UART4_CLK_MASK        DEF_BIT_07 | \
                                                        DEF_BIT_06

#define  LCP3xxx_REG_UART_CLKMODE_UART5_CLK_ON          DEF_BIT_08
#define  LPC3xxx_REG_UART_CLKMODE_UART5_CLK_AUTO        DEF_BIT_09
#define  LCP3xxx_REG_UART_CLKMODE_UART5_CLK_MASK        DEF_BIT_09 | \
                                                        DEF_BIT_08

#define  LCP3xxx_REG_UART_CLKMODE_UART6_CLK_ON          DEF_BIT_10
#define  LPC3xxx_REG_UART_CLKMODE_UART6_CLK_AUTO        DEF_BIT_11
#define  LCP3xxx_REG_UART_CLKMODE_UART6_CLK_MASK        DEF_BIT_11 | \
                                                        DEF_BIT_10

#define  BSP_REG_UxLCR_DLAB                             DEF_BIT_07           /* Divisor Latch Acces Bit                                  */
#define  BSP_REG_UxLCR_WLS_8                                     3           /* 8 bits Word Length Select                                */

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "
  #error  "                                  [     || PROBE_RS232_UART_3]      "
  #error  "                                  [     || PROBE_RS232_UART_4]      "
  #error  "                                  [     || PROBE_RS232_UART_5]      "
  #error  "                                  [     || PROBE_RS232_UART_6]      "
  #error  "                                  [     || PROBE_RS232_UART_7]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_4) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_5) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_6) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_7)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "
  #error  "                                  [     || PROBE_RS232_UART_3]      "
  #error  "                                  [     || PROBE_RS232_UART_4]      "
  #error  "                                  [     || PROBE_RS232_UART_5]      "
  #error  "                                  [     || PROBE_RS232_UART_6]      "
  #error  "                                  [     || PROBE_RS232_UART_7]      "
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
#if ((PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6))
    CPU_INT08U  divlo;
    CPU_INT08U  divhi;
#endif
    CPU_INT32U  uart_div;
    CPU_INT32U  pclk_freq;


    pclk_freq = BSP_PwrCtrl_PCLK_FreqGet();                     /* Get the CPU clock frequency                          */

#if ((PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6))
                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    uart_div  = (pclk_freq + (8 * baud_rate)) / (16 * baud_rate); 
    divlo     =  uart_div       & 0x00FF;                       /* Split divisor into LOW and HIGH bytes                */
    divhi     = (uart_div >> 8) & 0x00FF;
#endif

#if ((PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2) ||    \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7))
                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    uart_div = (pclk_freq - (7 * baud_rate)) / (14 * baud_rate);
    
#endif



#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)

                                                                /* ------------------- SETUP UART1 -------------------- */
    LPC3xxx_REG_HSU1_RATE     = (CPU_INT08U)uart_div;           /* Set clock divider                                    */
    LPC3xxx_REG_HSU1_CTRL     =   0x00002800;                   /* Ensure control register starts with default value    */
    LPC3xxx_REG_HSU1_CTRL    |=  (0x14 <<  9);                  /* Ensure a sampling offset of 20 (default)             */
    LPC3xxx_REG_HSU1_CTRL    |=  (3 << 16);                     /* Set the character timeout to 16 characters           */
    LPC3xxx_REG_HSU1_CTRL    &=  ~0x1F;                         /* Set the Rx and Tx minimum FIFO levels to 0           */
    LPC3xxx_REG_HSU1_CTRL    &= ~(3 <<  5);                     /* Disable Receive and Tx Interrupts                    */
    LPC3xxx_REG_HSU1_IIR      =   0x3F;                         /* Clear all interrupt flags                            */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART1---------- */
    
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR1,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR1);

#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)

                                                                /* ------------------- SETUP UART2 -------------------- */
    LPC3xxx_REG_HSU2_RATE     = (CPU_INT08U)uart_div;           /* Set clock divider                                    */
    LPC3xxx_REG_HSU2_CTRL     =   0x00002800;                   /* Ensure control register starts with default value    */
    LPC3xxx_REG_HSU2_CTRL    |=  (0x14 <<  9);                  /* Ensure a sampling offset of 20 (default)             */
    LPC3xxx_REG_HSU2_CTRL    |=  (3 << 16);                     /* Set the character timeout to 16 characters           */
    LPC3xxx_REG_HSU2_CTRL    &=  ~0x1F;                         /* Set the Rx and Tx minimum FIFO levels to 0           */
    LPC3xxx_REG_HSU2_CTRL    &= ~(3 <<  5);                     /* Disable Receive and Tx Interrupts                    */
    LPC3xxx_REG_HSU2_IIR      =   0x3F;                         /* Clear all interrupt flags                            */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART2 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR2,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR2);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    
    DEF_BIT_SET(LPC3xxx_REG_UARTCLK_CTRL, DEF_BIT_00);          /* UART 3 HCLK enabled                                  */
                                                                /* ------------------- SETUP CLOCK -------------------- */
                                                                /* Set UART3 clock to be on                             */
    LPC3xxx_REG_UART_CLKMODE &= ~LCP3xxx_REG_UART_CLKMODE_UART3_CLK_MASK;
    LPC3xxx_REG_UART_CLKMODE |=  LCP3xxx_REG_UART_CLKMODE_UART3_CLK_ON;

    LPC3xxx_REG_U3CLK         = 0;                              /* Reset the clock select register to the default value */
    LPC3xxx_REG_U3CLK        |= DEF_BIT_08 | DEF_BIT_00;        /* Use peripheral clock, with xdiv = 1, ydiv = 1        */


                                                                /* ------------------- SETUP UART4 -------------------- */
    LPC3xxx_REG_U3LCR         = BSP_REG_UxLCR_DLAB;             /* Set divisor access bit                               */
    LPC3xxx_REG_U3DLL         = divlo;                          /* Load divisor                                         */
    LPC3xxx_REG_U3DLM         = divhi;
    LPC3xxx_REG_U3LCR         = BSP_REG_UxLCR_WLS_8;            /* Set line control register (Bit 8 is 0)               */
    LPC3xxx_REG_U3IER         = 0x00;                           /* Disable both Rx and Tx interrupts                    */
    LPC3xxx_REG_U3FCR         = 0x00;                           /* Disable FIFO, flush Rx & Tx                          */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART3 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR3,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR3);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    DEF_BIT_SET(LPC3xxx_REG_UARTCLK_CTRL, DEF_BIT_01);          /* UART 4 HCLK enabled                                  */

                                                                /* ------------------- SETUP CLOCK -------------------- */
                                                                /* Set UART4 clock to be on                             */
    LPC3xxx_REG_UART_CLKMODE &= ~LCP3xxx_REG_UART_CLKMODE_UART4_CLK_MASK;
    LPC3xxx_REG_UART_CLKMODE |=  LCP3xxx_REG_UART_CLKMODE_UART4_CLK_ON;

    LPC3xxx_REG_U4CLK         = 0;                              /* Reset the clock select register to the default value */
    LPC3xxx_REG_U4CLK        |= DEF_BIT_08 | DEF_BIT_00;        /* Use peripheral clock, with xdiv = 1, ydiv = 1        */


                                                                /* ------------------- SETUP UART4 -------------------- */
    LPC3xxx_REG_U4LCR         = BSP_REG_UxLCR_DLAB;             /* Set divisor access bit                               */
    LPC3xxx_REG_U4DLL         = divlo;                          /* Load divisor                                         */
    LPC3xxx_REG_U4DLM         = divhi;
    LPC3xxx_REG_U4LCR         = BSP_REG_UxLCR_WLS_8;            /* Set line control register (Bit 8 is 0)               */
    LPC3xxx_REG_U4IER         = 0x00;                           /* Disable both Rx and Tx interrupts                    */
    LPC3xxx_REG_U4FCR         = 0x00;                           /* Disable FIFO, flush Rx & Tx                          */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART4 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR4,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR4);

#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    DEF_BIT_SET(LPC3xxx_REG_UARTCLK_CTRL, DEF_BIT_02);          /* UART 5 HCLK enabled                                  */

                                                                /* ------------------- SETUP CLOCK -------------------- */
                                                                /* Set UART5 clock to be on                             */
    LPC3xxx_REG_UART_CLKMODE &= ~LCP3xxx_REG_UART_CLKMODE_UART5_CLK_MASK;
    LPC3xxx_REG_UART_CLKMODE |=  LCP3xxx_REG_UART_CLKMODE_UART5_CLK_ON;

    LPC3xxx_REG_U5CLK         = 0;                              /* Reset the clock select register to the default value */
    LPC3xxx_REG_U5CLK        |= DEF_BIT_08 | DEF_BIT_00;        /* Use peripheral clock, with xdiv = 1, ydiv = 1        */


                                                                /* ------------------- SETUP UART5 -------------------- */
    LPC3xxx_REG_U5LCR         = BSP_REG_UxLCR_DLAB;             /* Set divisor access bit                               */
    LPC3xxx_REG_U5DLL         = divlo;                          /* Load divisor                                         */
    LPC3xxx_REG_U5DLM         = divhi;
    LPC3xxx_REG_U5LCR         = BSP_REG_UxLCR_WLS_8;            /* Set line control register (Bit 8 is 0)               */
    LPC3xxx_REG_U5IER         = 0x00;                           /* Disable both Rx and Tx interrupts                    */
    LPC3xxx_REG_U5FCR         = 0x00;                           /* Disable FIFO, flush Rx & Tx                          */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART5 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR5,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR5);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    DEF_BIT_SET(LPC3xxx_REG_UARTCLK_CTRL, DEF_BIT_03);          /* UART 6 HCLK enabled                                  */
    
                                                                /* ------------------- SETUP CLOCK -------------------- */
                                                                /* Set UART6 clock to be on                             */
    LPC3xxx_REG_UART_CLKMODE &= ~LCP3xxx_REG_UART_CLKMODE_UART6_CLK_MASK;
    LPC3xxx_REG_UART_CLKMODE |=  LCP3xxx_REG_UART_CLKMODE_UART6_CLK_ON;

    LPC3xxx_REG_U6CLK         = 0;                              /* Reset the clock select register to the default value */
    LPC3xxx_REG_U6CLK        |= DEF_BIT_08 | DEF_BIT_00;        /* Use peripheral clock, with xdiv = 1, ydiv = 1        */


                                                                /* ------------------- SETUP UART6 -------------------- */
    LPC3xxx_REG_U6LCR         = BSP_REG_UxLCR_DLAB;             /* Set divisor access bit                               */
    LPC3xxx_REG_U6DLL         = divlo;                          /* Load divisor                                         */
    LPC3xxx_REG_U6DLM         = divhi;
    LPC3xxx_REG_U6LCR         = BSP_REG_UxLCR_WLS_8;            /* Set line control register (Bit 8 is 0)               */
    LPC3xxx_REG_U6IER         = 0x00;                           /* Disable both Rx and Tx interrupts                    */
    LPC3xxx_REG_U6FCR         = 0x01;                           /* Disable FIFO, flush Rx & Tx                          */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART6 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR6,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR6);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)

                                                                /* ------------------- SETUP UART7 -------------------- */
    LPC3xxx_REG_HSU7_RATE  = (CPU_INT08U)uart_div;              /* Set clock divider                                    */
    LPC3xxx_REG_HSU7_CTRL  =   0x00002800;                      /* Ensure control register starts with default value    */
    LPC3xxx_REG_HSU7_CTRL |=  (0x14 <<  9);                     /* Ensure a sampling offset of 20 (default)             */
    LPC3xxx_REG_HSU7_CTRL |=  (3 << 16);                        /* Set the character timeout to 16 characters           */
    LPC3xxx_REG_HSU7_CTRL &=  ~0x1F;                            /* Set the Rx and Tx minimum FIFO levels to 0           */
    LPC3xxx_REG_HSU7_CTRL &= ~(3 <<  5);                        /* Disable Receive and Tx Interrupts                    */
    LPC3xxx_REG_HSU7_IIR   =   0x3F;                            /* Clear all interrupt flags                            */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART7 ---------- */
    BSP_IntVectSet((CPU_INT08U   )BSP_INT_CTRL_MAIN,           
                   (CPU_INT08U   )BSP_INT_CTRL_SRC_MAIN_IIR7,
                   (CPU_INT08U   )BSP_INT_CTRL_POL_HIGH,
                   (CPU_INT08U   )BSP_INT_CTRL_ACT_TYPE_LEVEL,
                   (CPU_INT08U   )BSP_INT_CTRL_TYPE_IRQ,
                   (CPU_FNCT_VOID)ProbeRS232_RxTxISRHandler);    

    BSP_IntEn(BSP_INT_CTRL_MAIN,           
              BSP_INT_CTRL_SRC_MAIN_IIR7);
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



#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    iir = (LPC3xxx_REG_HSU1_IIR & 0x3F);                        /* Read the interrupt status register.                  */

    if ((iir & DEF_BIT_00) == DEF_BIT_00) {                     /* Tx complete interrupt.                               */
        LPC3xxx_REG_HSU1_IIR |= DEF_BIT_00;                     /* Clear the Interrupt source.                          */
        ProbeRS232_TxHandler();                                 /* Call the Tx handler, Int. cleared by read of U1IIR.  */
    }

    if ((iir & DEF_BIT_01) == DEF_BIT_01) {                     /* If FIFO is below threshhold.                         */
        while ((LPC3xxx_REG_HSU1_LEVEL & 0xFF) > 0) {           /* Read bytes from FIFO while there is data present.    */
            rx_data = (LPC3xxx_REG_HSU1_RX & 0xFF);             /* Read the data. Int. cleared by read of U1RBR.        */
            ProbeRS232_RxHandler(rx_data);                      /* Call the generic Rx handler.                         */
        }
    }

    if ((iir & DEF_BIT_03) == DEF_BIT_03) {                     /* Framing Error.                                       */
        LPC3xxx_REG_HSU1_IIR |= DEF_BIT_03;
    }

    if ((iir & DEF_BIT_04) == DEF_BIT_04) {                     /* Break Interrupt.                                     */
        LPC3xxx_REG_HSU1_IIR |= DEF_BIT_04;
    }

    if ((iir & DEF_BIT_05) == DEF_BIT_05) {                     /* Overrun Interrupt.                                   */
        LPC3xxx_REG_HSU1_IIR |= DEF_BIT_05;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    iir = (LPC3xxx_REG_HSU2_IIR & 0x3F);                        /* Read the interrupt status register.                  */

    if ((iir & DEF_BIT_00) == DEF_BIT_00) {                     /* Tx complete interrupt.                               */
        LPC3xxx_REG_HSU2_IIR |= DEF_BIT_00;                     /* Clear the Interrupt source.                          */
        ProbeRS232_TxHandler();                                 /* Call the Tx handler, Int. cleared by read of U2IIR.  */
    }

    if ((iir & DEF_BIT_01) == DEF_BIT_01) {                     /* If FIFO is below threshhold.                         */
        while ((LPC3xxx_REG_HSU2_LEVEL & 0xFF) > 0) {           /* Read bytes from FIFO while there is data present.    */
            rx_data = (LPC3xxx_REG_HSU2_RX & 0xFF);             /* Read the data. Int. cleared by read of U2RBR.        */
            ProbeRS232_RxHandler(rx_data);                      /* Call the generic Rx handler.                         */
        }
    }

    if ((iir & DEF_BIT_03) == DEF_BIT_03) {                     /* Framing Error.                                       */
        LPC3xxx_REG_HSU2_IIR |= DEF_BIT_03;
    }

    if ((iir & DEF_BIT_04) == DEF_BIT_04) {                     /* Break Interrupt.                                     */
        LPC3xxx_REG_HSU2_IIR |= DEF_BIT_04;
    }

    if ((iir & DEF_BIT_05) == DEF_BIT_05) {                     /* Overrun Interrupt.                                   */
        LPC3xxx_REG_HSU2_IIR |= DEF_BIT_05;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    iir = LPC3xxx_REG_U3IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 rx_data = LPC3xxx_REG_U3RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler.                             */
                 break;

            case  6:                                            /* Receive Line Status interrupt?                           */
                 lsr     = LPC3xxx_REG_U3LSR;
                 break;

            case 12:                                            /* CTI interrupt?                                           */
                 rx_data = LPC3xxx_REG_U3RBR;
                 break;

            default:
                 break;
        }
        iir = LPC3xxx_REG_U3IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    iir = LPC3xxx_REG_U4IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 rx_data = LPC3xxx_REG_U4RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler.                             */
                 break;

            case  6:                                            /* Receive Line Status interrupt?                           */
                 lsr     = LPC3xxx_REG_U4LSR;
                 break;

            case 12:                                            /* CTI interrupt?                                           */
                 rx_data = LPC3xxx_REG_U4RBR;
                 break;

            default:
                 break;
        }
        iir = LPC3xxx_REG_U4IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    iir = LPC3xxx_REG_U5IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 rx_data = LPC3xxx_REG_U5RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler.                             */
                 break;

            case  6:                                            /* Receive Line Status interrupt?                           */
                 lsr     = LPC3xxx_REG_U5LSR;
                 break;

            case 12:                                            /* CTI interrupt?                                           */
                 rx_data = LPC3xxx_REG_U5RBR;
                 break;

            default:
                 break;
        }
        iir = LPC3xxx_REG_U5IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    iir = LPC3xxx_REG_U6IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 rx_data = LPC3xxx_REG_U6RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler.                             */
                 break;

            case  6:                                            /* Receive Line Status interrupt?                           */
                 lsr     = LPC3xxx_REG_U6LSR;
                 break;

            case 12:                                            /* CTI interrupt?                                           */
                 rx_data = LPC3xxx_REG_U6RBR;
                 break;

            default:
                 break;
        }
        iir = LPC3xxx_REG_U6IIR & 0x0F;
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    iir = (LPC3xxx_REG_HSU7_IIR & 0x3F);                        /* Read the interrupt status register.                  */

    if ((iir & DEF_BIT_00) == DEF_BIT_00) {                     /* Tx complete interrupt.                               */
        LPC3xxx_REG_HSU7_IIR |= DEF_BIT_00;                     /* Clear the Interrupt source.                          */
        ProbeRS232_TxHandler();                                 /* Call the Tx handler, Int. cleared by read of U7IIR.  */
    }

    if ((iir & DEF_BIT_01) == DEF_BIT_01) {                     /* If FIFO is below threshhold.                         */
        while ((LPC3xxx_REG_HSU7_LEVEL & 0xFF) > 0) {           /* Read bytes from FIFO while there is data present.    */
            rx_data = (LPC3xxx_REG_HSU7_RX & 0xFF);             /* Read the data. Int. cleared by read of U7RBR.        */
            ProbeRS232_RxHandler(rx_data);                      /* Call the generic Rx handler.                         */
        }
    }

    if ((iir & DEF_BIT_03) == DEF_BIT_03) {                     /* Framing Error                                        */
        LPC3xxx_REG_HSU7_IIR |= DEF_BIT_03;
    }

    if ((iir & DEF_BIT_04) == DEF_BIT_04) {                     /* Break Interrupt                                      */
        LPC3xxx_REG_HSU7_IIR |= DEF_BIT_04;
    }

    if ((iir & DEF_BIT_05) == DEF_BIT_05) {                     /* Overrun Interrupt                                    */
        LPC3xxx_REG_HSU7_IIR |= DEF_BIT_05;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    LPC3xxx_REG_HSU1_CTRL &= ~DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LPC3xxx_REG_HSU2_CTRL &= ~DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    LPC3xxx_REG_U3IER     &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    LPC3xxx_REG_U4IER     &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    LPC3xxx_REG_U5IER     &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    LPC3xxx_REG_U6IER     &= ~DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    LPC3xxx_REG_HSU7_CTRL &= ~DEF_BIT_06;
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
    LPC3xxx_REG_HSU1_CTRL |=  DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LPC3xxx_REG_HSU2_CTRL |=  DEF_BIT_06;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    LPC3xxx_REG_U3IER     |=  DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    LPC3xxx_REG_U4IER     |=  DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    LPC3xxx_REG_U5IER     |=  DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    LPC3xxx_REG_U6IER     |=  DEF_BIT_00;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    LPC3xxx_REG_HSU7_CTRL |=  DEF_BIT_06;
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
    LPC3xxx_REG_HSU1_TX = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LPC3xxx_REG_HSU2_TX = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    LPC3xxx_REG_U3THR   = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    LPC3xxx_REG_U4THR   = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    LPC3xxx_REG_U5THR   = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    LPC3xxx_REG_U6THR   = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    LPC3xxx_REG_HSU7_TX = c;
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
    LPC3xxx_REG_HSU1_CTRL &= ~DEF_BIT_05;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LPC3xxx_REG_HSU2_CTRL &= ~DEF_BIT_05;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    LPC3xxx_REG_U3IER     &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    LPC3xxx_REG_U4IER     &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    LPC3xxx_REG_U5IER     &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    LPC3xxx_REG_U6IER     &= ~DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    LPC3xxx_REG_HSU7_CTRL &= ~DEF_BIT_05;
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
    LPC3xxx_REG_HSU1_CTRL |=  DEF_BIT_05;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    LPC3xxx_REG_HSU2_CTRL |=  DEF_BIT_05;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    LPC3xxx_REG_U3IER     |=  DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    LPC3xxx_REG_U4IER     |=  DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    LPC3xxx_REG_U5IER     |=  DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    LPC3xxx_REG_U6IER     |=  DEF_BIT_01;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    LPC3xxx_REG_HSU7_CTRL |=  DEF_BIT_05;
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
