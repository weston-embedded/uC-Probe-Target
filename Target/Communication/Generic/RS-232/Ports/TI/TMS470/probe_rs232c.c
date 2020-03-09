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
*                                          PORT FOR TI TMS470
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
                                                                /* ----------- REGISTER BASE SPECIFICATIONS ----------- */
#define  SCI1_BASE              ((CPU_INT32U)0xFFF7F400)
#define  SCI2_BASE              ((CPU_INT32U)0xFFF7F500)


                                                                /* ---------------- CIM PERIPHERAL IDs ---------------- */
#define  CIM_SCI1RX                    10
#define  CIM_SCI1TX                    20

#define  CIM_SCI2RX                    17
#define  CIM_SCI2TX                    26


                                                                /* --------------- CLK REGISTER DEFINES --------------- */
#define  PCR                (*(volatile CPU_INT32U *)(0xFFFFFD30))

                                                                /* --------------- SMC REGISTER DEFINES --------------- */
#define  REQMASK            (*(volatile CPU_INT32U *)(0xFFFFFF34))

                                                                /* --------------- SCI1 REGISTER DEFINES -------------- */
#define  SCI1CCR            (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0003))
#define  SCI1CTL1           (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0007))
#define  SCI1CTL2           (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x000B))
#define  SCI1CTL3           (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x000F))
#define  SCI1RXST           (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0013))
#define  SCI1HBAUD          (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0017))
#define  SCI1MBAUD          (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x001B))
#define  SCI1LBAUD          (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x001F))
#define  SCI1RXBUF          (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0027))
#define  SCI1TXBUF          (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x002B))
#define  SCI1PC1            (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x002F))
#define  SCI1PC2            (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0033))
#define  SCI1PC3            (*(volatile CPU_INT08U *)(SCI1_BASE   + 0x0037))

                                                                /* --------------- SCI2 REGISTER DEFINES -------------- */
#define  SCI2CCR            (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0003))
#define  SCI2CTL1           (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0007))
#define  SCI2CTL2           (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x000B))
#define  SCI2CTL3           (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x000F))
#define  SCI2RXST           (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0013))
#define  SCI2HBAUD          (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0017))
#define  SCI2MBAUD          (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x001B))
#define  SCI2LBAUD          (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x001F))
#define  SCI2RXBUF          (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0027))
#define  SCI2TXBUF          (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x002B))
#define  SCI2PC1            (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x002F))
#define  SCI2PC2            (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0033))
#define  SCI2PC3            (*(volatile CPU_INT08U *)(SCI2_BASE   + 0x0037))


                                                                /* ----------- SCICCR REGISTER BIT DEFINES ------------ */
#define   SCICCR_CHAR_8                    (CPU_INT32U)0x07
#define   SCICCR_TIMING_MODE_ASYNC               DEF_BIT_04

                                                                /* ----------- SCICTL1 REGISTER BIT DEFINES ----------- */
#define   SCICTL1_RXENA                          DEF_BIT_00

                                                                /* ----------- SCICTL2 REGISTER BIT DEFINES ----------- */
#define   SCICTL2_TXENA                          DEF_BIT_00

                                                                /* ----------- SCICTL3 REGISTER BIT DEFINES ----------- */
#define   SCICTL3_TX_ACTION_ENA                  DEF_BIT_03
#define   SCICTL3_RX_ACTION_ENA                  DEF_BIT_04
#define   SCICTL3_CLOCK                          DEF_BIT_05
#define   SCICTL3_SW_NRESET                      DEF_BIT_07

                                                                /* ------------ SCIPC2 REGISTER BIT DEFINES ----------- */
#define   SCIPC2_RX_FUNC                         DEF_BIT_01

                                                                /* ------------ SCIPC3 REGISTER BIT DEFINES ----------- */
#define   SCIPC3_TX_FUNC                         DEF_BIT_01


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

extern  CPU_INT32U  BSP_CPU_ClkFreq(void);

extern  void        BSP_IntVectSet (CPU_INT32U     int_nbr,
                                    CPU_FNCT_VOID  pisr);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_1]      "
  #error  "                                  [     || PROBE_RS232_UART_2]      "

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

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    CPU_INT08U  pcr_div;
    CPU_INT16U  div;
    CPU_INT08U  divlo;
    CPU_INT08U  divmid;
    CPU_INT08U  divhi;
    CPU_INT32U  clk_freq;


                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    clk_freq  = BSP_CPU_ClkFreq();                              /* Get the CPU clock frequency.                         */
    pcr_div   = ((PCR >> 1) + 1) & 0x0F;
    clk_freq /= pcr_div;

    div       = ((clk_freq / 8) / baud_rate) - 1;
    divlo     = div % 256;
    div      /= 256;
    divmid    = div % 256;
    div      /= 256;
    divhi     = div % 256;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
                                                                /* -------------- INITIALIZE CIM FOR SCI1 ------------- */
    BSP_IntVectSet(CIM_SCI1RX, ProbeRS232_RxISRHandler);        /* Set SCI1RX int vect.                                 */
    BSP_IntVectSet(CIM_SCI1TX, ProbeRS232_TxISRHandler);        /* Set SCI1TX int vect.                                 */
    REQMASK  &= ~(1 << CIM_SCI1RX);                             /* Dis SCI1RX ch int.                                   */
    REQMASK  &= ~(1 << CIM_SCI1TX);                             /* Dis SCI1TX ch int.                                   */

                                                                /* -------------------- SETUP SCI1 -------------------- */
    SCI1CTL3 &= ~SW_NRESET;                                     /* Reset SCI state machine.                             */
    SCI1CCR   = TIMING_MODE_ASYNC | CHAR_8;                     /* Asynch, 8 data bits, 1 stop bit, no parity.          */
    SCI1CTL1  = RXENA;                                          /* En RX.                                               */
    SCI1CTL2  = TXENA;                                          /* En TX.                                               */
    SCI1CTL3 |= CLOCK                                           /* Use internal clock.                                  */
              | RX_ACTION_ENA;                                  /* Enable rx int.                                       */

    SCI1LBAUD = divlo;                                          /* Set low    byte of baud rate div.                    */
    SCI1MBAUD = divmid;                                         /* Set middle byte of baud rate div.                    */
    SCI1HBAUD = divhi;                                          /* Set high   byte of baud rate div.                    */

    SCI1PC2  |= RX_FUNC;                                        /* SCIRX is the SCI rx pin.                             */
    SCI1PC3  |= TX_FUNC;                                        /* SCITX is the SCI tx pin.                             */
    SCI1CTL3 |= SW_NRESET;                                      /* Cfg SCI2 state machine.                              */

    REQMASK  |= (1 << CIM_SCI1RX);                              /* En SCI1RX ch int.                                    */
    REQMASK  |= (1 << CIM_SCI1TX);                              /* En SCI1TX ch int.                                    */
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
                                                                /* -------------- INITIALIZE CIM FOR SCI2 ------------- */
    BSP_IntVectSet(CIM_SCI2RX, ProbeRS232_RxISRHandler);        /* Set SCI2RX int vect.                                 */
    BSP_IntVectSet(CIM_SCI2TX, ProbeRS232_TxISRHandler);        /* Set SCI2TX int vect.                                 */
    REQMASK  &= ~(1 << CIM_SCI2RX);                             /* Dis SCI2RX ch int.                                   */
    REQMASK  &= ~(1 << CIM_SCI2TX);                             /* Dis SCI2TX ch int.                                   */

                                                                /* -------------------- SETUP SCI2 -------------------- */
    SCI2CTL3 &= ~SCICTL3_SW_NRESET;                             /* Reset SCI state machine.                             */
    SCI2CCR   = SCICCR_TIMING_MODE_ASYNC | SCICCR_CHAR_8;       /* Asynch, 8 data bits, 1 stop bit, no parity.          */
    SCI2CTL1  = SCICTL1_RXENA;                                  /* En RX.                                               */
    SCI2CTL2  = SCICTL2_TXENA;                                  /* En TX.                                               */
    SCI2CTL3 |= SCICTL3_CLOCK                                   /* Use internal clock.                                  */
              | SCICTL3_RX_ACTION_ENA;                          /* En rx int.                                           */

    SCI2LBAUD = divlo;                                          /* Set low    byte of baud rate div.                    */
    SCI2MBAUD = divmid;                                         /* Set middle byte of baud rate div.                    */
    SCI2HBAUD = divhi;                                          /* Set high   byte of baud rate div.                    */

    SCI2PC2  |= SCIPC2_RX_FUNC;                                 /* SCIRX is the SCI rx pin.                             */
    SCI2PC3  |= SCIPC3_TX_FUNC;                                 /* SCITX is the SCI tx pin.                             */
    SCI2CTL3 |= SCICTL3_SW_NRESET;                              /* Cfg SCI2 state machine.                              */

    REQMASK  |= (1 << CIM_SCI2RX);                              /* En SCI2RX ch int.                                    */
    REQMASK  |= (1 << CIM_SCI2TX);                              /* En SCI2TX ch int.                                    */
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
* Note(s)     : (1) This function is empty because receive and transmit interrupts are handled in the
*                   separate interrupt handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{

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
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    CPU_INT08U   rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    rx_data = SCI1RXBUF;
    ProbeRS232_RxHandler(rx_data);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    rx_data = SCI2RXBUF;
    ProbeRS232_RxHandler(rx_data);
#endif
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
    SCI1CTL3 &= ~SCICTL3_RX_ACTION_ENA;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2CTL3 &= ~SCICTL3_RX_ACTION_ENA;
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
    SCI1CTL3 |=  SCICTL3_RX_ACTION_ENA;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2CTL3 |=  SCICTL3_RX_ACTION_ENA;
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
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
#if ((PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1) || \
     (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2))
    ProbeRS232_TxHandler();
#endif
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
    SCI1TXBUF = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2TXBUF = c;
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
    SCI1CTL3 &= ~SCICTL3_TX_ACTION_ENA;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2CTL3 &= ~SCICTL3_TX_ACTION_ENA;
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
    SCI1CTL3 |=  SCICTL3_TX_ACTION_ENA;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    SCI2CTL3 |=  SCICTL3_TX_ACTION_ENA;
#endif
}
