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
*                                  PORT FOR LUMINARY MICRO LM3Sxxxx
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
#include  <includes.h>


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
                                                                /* ---------------- UART0 Register Defines ---------------- */
#define  UART0DR            (*(CPU_REG32 *)(UART0_BASE + 0x0000))
#define  UART0FR            (*(CPU_REG32 *)(UART0_BASE + 0x0018))
#define  UART0IBRD          (*(CPU_REG32 *)(UART0_BASE + 0x0024))
#define  UART0FBRD          (*(CPU_REG32 *)(UART0_BASE + 0x0028))
#define  UART0LCRH          (*(CPU_REG32 *)(UART0_BASE + 0x002C))
#define  UART0CTL           (*(CPU_REG32 *)(UART0_BASE + 0x0030))
#define  UART0IFLS          (*(CPU_REG32 *)(UART0_BASE + 0x0034))
#define  UART0IM            (*(CPU_REG32 *)(UART0_BASE + 0x0038))
#define  UART0MIS           (*(CPU_REG32 *)(UART0_BASE + 0x0040))
#define  UART0ICR           (*(CPU_REG32 *)(UART0_BASE + 0x0044))

                                                                /* ---------------- UART1 Register Defines ---------------- */
#define  UART1DR            (*(CPU_REG32 *)(UART1_BASE + 0x0000))
#define  UART1FR            (*(CPU_REG32 *)(UART1_BASE + 0x0018))
#define  UART1IBRD          (*(CPU_REG32 *)(UART1_BASE + 0x0024))
#define  UART1FBRD          (*(CPU_REG32 *)(UART1_BASE + 0x0028))
#define  UART1LCRH          (*(CPU_REG32 *)(UART1_BASE + 0x002C))
#define  UART1CTL           (*(CPU_REG32 *)(UART1_BASE + 0x0030))
#define  UART1IFLS          (*(CPU_REG32 *)(UART1_BASE + 0x0034))
#define  UART1IM            (*(CPU_REG32 *)(UART1_BASE + 0x0038))
#define  UART1MIS           (*(CPU_REG32 *)(UART1_BASE + 0x0040))
#define  UART1ICR           (*(CPU_REG32 *)(UART1_BASE + 0x0044))

                                                                /* ------------- UARTCTL Register Bit Defines ------------- */
#define  UARTCTL_UARTEN             DEF_BIT_00
#define  UARTCTL_TXE                DEF_BIT_08
#define  UARTCTL_RXE                DEF_BIT_09

                                                                /* ------------- UARTIM  Register Bit Defines ------------- */
                                                                /* ------------- UARTRIS Register Bit Defines ------------- */
                                                                /* ------------- UARTMIS Register Bit Defines ------------- */
                                                                /* ------------- UARTICR Register Bit Defines ------------- */
#define  UARTINT_RX                 DEF_BIT_04
#define  UARTINT_TX                 DEF_BIT_05
#define  UARTINT_RT                 DEF_BIT_06
#define  UARTINT_FE                 DEF_BIT_07
#define  UARTINT_PE                 DEF_BIT_08
#define  UARTINT_BE                 DEF_BIT_09
#define  UARTINT_OE                 DEF_BIT_10

                                                                /* ------------- UARTFLS Register Bit Defines ------------- */
#define  UARTFLS_RX_1_8             0x00000000
#define  UARTFLS_TX_7_8             0x00000004

                                                                /* ------------ UARTLCRH Register Bit Defines ------------- */
#define  UARTLCRH_WLEN_8            0x00000060
#define  UARTLCRH_FEN               DEF_BIT_04

                                                                /* ------------- UARTFR Register Bit Defines -------------- */
#define  UARTFR_BUSY                DEF_BIT_03
#define  UARTFR_RXFE                DEF_BIT_04
#define  UARTFR_TXFF                DEF_BIT_05
#define  UARTFR_RXFF                DEF_BIT_06
#define  UARTFR_TXFE                DEF_BIT_07

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


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
  #error  "                                  [     || PROBE_RS232_UART_1]     "

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
    CPU_INT32U  idiv;
    CPU_INT32U  fdiv;
    CPU_INT32U  clk_freq;


                                                                /* --------------- COMPUTE DIVISOR BAUD RATE -------------- */
    clk_freq = SysCtlClockGet();
    idiv     = clk_freq / (16 * baud_rate);
    fdiv     = clk_freq % (16 * baud_rate);
    fdiv     = ((((2 * fdiv * 4) / baud_rate) + 1) / 2);

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_UART0);

                                                                /* ------------------- ENABLE UART0 I/Os ------------------ */
                                                                /* PA[0] & PA[1]                                            */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

                                                                /* --------------------- SETUP UART0 ---------------------- */
    UART0CTL  &= ~UARTCTL_UARTEN;
    UART0LCRH &= ~UARTLCRH_FEN;
    UART0IBRD  =  idiv;
    UART0FBRD  =  fdiv;
    UART0LCRH  =  UARTLCRH_WLEN_8 /*|
                  UARTLCRH_FEN*/;
    UART0CTL   =  UARTCTL_RXE |
                  UARTCTL_TXE |
                  UARTCTL_UARTEN;
    UART0IFLS  =  0x00000000;
    UART0ICR   =  UARTINT_OE |
                  UARTINT_BE |
                  UARTINT_PE |
                  UARTINT_FE |
                  UARTINT_RT |
                  UARTINT_TX |
                  UARTINT_RX;

    BSP_IntVectSet(BSP_INT_ID_UART0, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_UART0);                                /* ------------ INITIALIZE INTERRUPT FOR UART0 ------------ */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralReset(SYSCTL_PERIPH_UART1);
                                                                /* ------------------- ENABLE UART0 I/Os ------------------ */
                                                                /* PD[3] & PD[4]                                            */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3);

                                                                /* --------------------- SETUP UART1 ---------------------- */
    UART1CTL  &= ~UARTCTL_UARTEN;
    UART1LCRH &= ~UARTLCRH_FEN;
    UART1IBRD  =  idiv;
    UART1FBRD  =  fdiv;
    UART1LCRH  =  UARTLCRH_WLEN_8 /*|
                  UARTLCRH_FEN*/;
    UART1CTL   =  UARTCTL_RXE |
                  UARTCTL_TXE |
                  UARTCTL_UARTEN;
    UART1IFLS  =  0x00000000;
    UART1ICR   =  UARTINT_OE |
                  UARTINT_BE |
                  UARTINT_PE |
                  UARTINT_FE |
                  UARTINT_RT |
                  UARTINT_TX |
                  UARTINT_RX;

    BSP_IntVectSet(BSP_INT_ID_UART1, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_UART1);                                /* ------------ INITIALIZE INTERRUPT FOR UART1 ------------ */
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
* Note(s)     : (1) This ISR handler handles the interrupt entrance/exit as expected by
*                   by uC/OS-II v2.8x.  If you are using a different RTOS (or no RTOS), then this
*                   procedure may need to be modified or eliminated.  However, the logic in the handler
*                   need not be changed.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT32U  status;
    CPU_INT08U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    status = UART0MIS;

    if ((status & UARTINT_TX) == UARTINT_TX) {
        while (((UART0IM & UARTINT_TX) == UARTINT_TX) && ((UART0FR & UARTFR_TXFF) == 0)) {
            ProbeRS232_TxHandler();                             /* Call the generic Tx handler                              */
        }
    }

    if (((status & UARTINT_RX) == UARTINT_RX) ||
        ((status & UARTINT_RT) == UARTINT_RT)) {
        while ((UART0FR & UARTFR_RXFE) == 0) {
            rx_data = (UART0DR & 0xFF);
            ProbeRS232_RxHandler(rx_data);                      /* Call the generic Rx handler                              */
        }
    }

    UART0ICR = status;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    status = UART1MIS;

    if ((status & UARTINT_TX) == UARTINT_TX) {
        while (((UART1IM & UARTINT_TX) == UARTINT_TX) && ((UART1FR & UARTFR_TXFF) == 0)) {
            ProbeRS232_TxHandler();                             /* Call the generic Tx handler                              */
        }
    }

    if ((status & UARTINT_RX) == UARTINT_RX) {
        while ((UART1FR & UARTFR_RXFE) == 0) {
            rx_data = (UART1DR & 0xFF);
            ProbeRS232_RxHandler(rx_data);                      /* Call the generic Rx handler                              */
        }
    }

    UART1ICR = status;
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
    UART0IM &= ~UARTINT_RX;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1IM &= ~UARTINT_RX;
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
    UART0IM |= UARTINT_RX | UARTINT_RT | UARTINT_BE;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1IM |= UARTINT_RX | UARTINT_RT | UARTINT_BE;
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
    UART0DR = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1DR = c;
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
    UART0IM &= ~UARTINT_TX;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1IM &= ~UARTINT_TX;
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
    UART0IM |=  UARTINT_TX;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UART1IM |=  UARTINT_TX;
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
