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
*                                      PORT FOR FREESCALE i.MX21
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
#include  <iomc9328mx21.h>
#include  <bsp.h>


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

extern  CPU_INT32U  BSP_ClkFreq       (CPU_INT08U  clk);

extern  void        BSP_Set_IRQ_Vector(CPU_INT08U  irq,
                                            void (*isr)(void));

extern  void        BSP_IntEn         (CPU_INT08U  irq);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'  "
  #error  "                                  [MUST be  PROBE_RS232_UART_1]       "
  #error  "                                  [     ||  PROBE_RS232_UART_4]       "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_4)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'  "
  #error  "                                  [MUST be  PROBE_RS232_UART_1]       "
  #error  "                                  [     ||  PROBE_RS232_UART_4]       "
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
* Note(s)     : (1) The baud rate divider is computed by
*
*                         (Desired Baud Rate) * 16         NUM        UBIR + 1
*                        --------------------------   =  -------  =  ----------
*                           (reference frequency)         DENOM       UBMR + 1
*
*                    where
*                       reference frequency  = (PERCLK1 / RFDIV [2:0])
*
*              (2) The reference divider for the selected UART must be configured to divide by 1.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    CPU_INT32U  perclk1_freq;
    CPU_INT32U  ubir;
    CPU_INT32U  ubmr;


                                                                /* --------------- COMPUTE BAUD RATE DIVISOR -------------- */
    perclk1_freq = BSP_ClkFreq(BSP_PERCLK1);                    /* Obtain UART reference clock frequency in Hz              */

    ubir         = baud_rate * 16;                              /* Set initial baud numerator                               */
    ubmr         = perclk1_freq;                                /* Set initial baud divider                                 */

    while ((ubir > 65535) || (ubmr > 65535)) {                  /* Divide the numerator and denominator by 2 until the      */
        ubir >>= 1;                                             /* values are small enough to fit into the registers        */
        ubmr >>= 1;                                             /* Note: The RFDIV bits for the selected UART must be make  */
    }                                                           /* a divider of 1 (register bits value = 5                  */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    PCCR0    |= DEF_BIT_00;                                     /* Enable UART module clock                                 */

                                                                /* --------------------- RESET UART1 ---------------------- */
    UCR1_1    = 0;                                              /* DISABLE the UART to reprogram it                         */
    UCR2_1   &= DEF_BIT_00;                                     /* Soft reset UART1. No UART reg accesses for 4 ipg_clocks  */


                                                                /* ------------------- ENABLE UART1 I/Os ------------------ */
    PTE_GIUS &= ~(DEF_BIT_12 | DEF_BIT_13);
    PTE_GPR  |=   DEF_BIT_12 | DEF_BIT_13;


                                                                /* --------------------- SETUP UART1 ---------------------- */
    UCR3_1   |= DEF_BIT_02;                                     /* Configure the Rx pin multiplexor for use with the UART   */

    USR1_1    = 0xFFFFFFFF;                                     /* Clear pending interrupts                                 */
    USR2_1    = 0xFFFFFFFF;                                     /* Clear pending interrupts                                 */

    UFCR_1    = (1 <<  0)                                       /* Rx FIFO triggers an int. when >= 1 byte received         */
              | (1 << 10)                                       /* Tx FIFO triggers an int. when <  1 byte in the Tx FIFO   */
              | (5 <<  7);                                      /* Set div = 1                                              */

                                                                /* Update baud rate: UBIR must be written before UBMR       */
    UBIR_1    = (ubir - 1);
    UBMR_1    = (ubmr - 1);

    UCR2_1    = 0x00004027;                                     /* UART1, Do not reset                                      */
                                                                /* 8 Bits, 1 Stop, No Parity                                */
                                                                /* Enable the Receiver                                      */
                                                                /* Enable the Transmitter                                   */
                                                                /* Ignore the RTS pin                                       */

    UCR1_1   |= DEF_BIT_00;                                     /* Enable the UART                                          */

                                                                /* ------------ INITIALIZE INTERRUPT FOR UART1 ------------ */
    BSP_Set_IRQ_Vector(INT_UART1, ProbeRS232_RxTxISRHandler);   /* Register the interrupt routine                           */
    BSP_IntEn(INT_UART1);                                       /* Enable the UART global interrupt                         */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    PCCR0       |= DEF_BIT_03;                                  /* Enable UART module clock                                 */

                                                                /* --------------------- RESET UART4 ---------------------- */
    UCR1_4       = 0;                                           /* DISABLE the UART to reprogram it                         */
    UCR2_4      &= DEF_BIT_00;                                  /* Soft reset UART4. No UART reg accesses for 4 ipg_clocks  */

                                                                /* ------------------- ENABLE UART4 I/Os ------------------ */
    PTB_GIUS    &= ~DEF_BIT_28;
    PTB_GPR     |=  DEF_BIT_28;

    PTB_GIUS    |=  DEF_BIT_29;
    PTB_GIUS    &= ~DEF_BIT_29;

    PTB_ICONFA2 &= ~(DEF_BIT_26 | DEF_BIT_27);

    FMCR        &= DEF_BIT_25;                                  /* Set USBH1_TXDP (PB29) GPIO AOUT as Rx input to UART4     */

                                                                /* --------------------- SETUP UART4 ---------------------- */
    UCR3_4      |= DEF_BIT_02;                                  /* Configure the Rx pin multiplexor for use with the UART   */

    USR1_4      = 0xFFFFFFFF;                                   /* Clear pending interrupts                                 */
    USR2_4      = 0xFFFFFFFF;                                   /* Clear pending interrupts                                 */

    UFCR_4      = (1 <<  0)                                     /* Rx FIFO triggers an int. when >= 1 byte received         */
                | (1 << 10)                                     /* Tx FIFO triggers an int. when <  1 byte in the Tx FIFO   */
                | (5 <<  7);                                    /* Set div = 1                                              */

                                                                /* Update baud rate: UBIR must be written before UBMR       */
    UBIR_4      = (ubir - 1);
    UBMR_4      = (ubmr - 1);

    UCR2_4      = 0x00004027;                                   /* UART1, Do not reset                                      */
                                                                /* 8 Bits, 1 Stop, No Parity                                */
                                                                /* Enable the Receiver                                      */
                                                                /* Enable the Transmitter                                   */
                                                                /* Ignore the RTS pin                                       */

    UCR1_4     |= DEF_BIT_00;                                   /* Enable the UART                                          */

                                                                /* ------------ INITIALIZE INTERRUPT FOR UART4 ------------ */
    BSP_Set_IRQ_Vector(INT_UART4, ProbeRS232_RxTxISRHandler);   /* Register the interrupt routine                           */
    BSP_IntEn(INT_UART4);                                       /* Enable the UART global interrupt                         */
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
* Note(s)     : (1) The receive interrupt is cleared automatically when the FIFO has no bytes it in.
*                   When this occurs, the FIFO is set to cause an interrupt when one byte arrives;
*                   therefore a single read will clear the flag.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT08U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    if ((USR1_1 & DEF_BIT_09) == DEF_BIT_09) {                  /* Received a character?                                    */
        rx_data = (CPU_INT08U)(URXD_1 & 0x00FF);                /* Read data.                                               */
        ProbeRS232_RxHandler(rx_data);                          /* Call the generic Rx handler                              */
    }

    if ((USR1_1 & DEF_BIT_13) == DEF_BIT_13) {                  /* Transmitted a character?                                 */
        ProbeRS232_TxHandler();
    }
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    if ((USR1_4 & DEF_BIT_09) == DEF_BIT_09) {                  /* Received a character?                                    */
        rx_data = (CPU_INT08U)(URXD_4 & 0x00FF);                /* Read data.                                               */
        ProbeRS232_RxHandler(rx_data);                          /* Call the generic Rx handler                              */
    }

    if ((USR1_4 & DEF_BIT_13) == DEF_BIT_13) {                  /* Transmitted a character?                                 */
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    UCR1_1 &= ~DEF_BIT_09;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    UCR1_4 &= ~DEF_BIT_09;
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
    UCR1_1 |= DEF_BIT_09;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    UCR1_4 |= DEF_BIT_09;
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
    UTXD_1 = (CPU_INT32U)c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    UTXD_4 = (CPU_INT32U)c;
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
    UCR1_1 &= ~DEF_BIT_13;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    UCR1_4 &= ~DEF_BIT_13;
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
    UCR1_1     |=  DEF_BIT_13;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    UCR1_4     |=  DEF_BIT_13;
#endif
}


