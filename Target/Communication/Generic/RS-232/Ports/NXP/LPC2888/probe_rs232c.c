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
*                                      PORT FOR THE NXP LPC288x
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
                                                                /* ------------- Register Base Specifications ------------- */
#define  LPC288X_UART_BASE                        0x80101000

                                                                /* ------------------- VIC Peripheral ID ------------------ */
#define  LPC288X_VIC_UART                                 12

                                                                /* ----------------- UART Register Defines ---------------- */
#define  LPC288X_REG_UART_RBR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0000))
#define  LPC288X_REG_UART_THR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0000))
#define  LPC288X_REG_UART_DLL       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0000))
#define  LPC288X_REG_UART_DLM       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0004))
#define  LPC288X_REG_UART_IER       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0004))
#define  LPC288X_REG_UART_IIR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0008))
#define  LPC288X_REG_UART_FCR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0008))
#define  LPC288X_REG_UART_LCR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x000C))
#define  LPC288X_REG_UART_LSR       (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0014))
#define  LPC288X_REG_UART_INTS      (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0FE0))
#define  LPC288X_REG_UART_INTE      (*(volatile CPU_INT08U *)(LPC288X_UART_BASE + 0x0FE4))


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
*
*               (2) 'BSP_IntEn()' enables the interrupt.
*
*               (3) 'BSP_IntPrioSet()' sets the interrupt priority.
*
*               (4) 'BSP_IntVectSet()' places the ISR handler into the vector table.
*********************************************************************************************************
*/

extern  CPU_INT32U  BSP_CPU_ClkFreq(void);

extern  void        BSP_IntEn      (CPU_INT08U     nbr);

extern  void        BSP_IntPrioSet (CPU_INT08U     nbr,
                                    CPU_INT08U     prio);

extern  void        BSP_IntVectSet (CPU_INT08U     nbr,
                                    CPU_FNCT_VOID  handler);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0]      "
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
    CPU_INT32U  clk_freq;


                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    clk_freq = BSP_CPU_ClkFreq();                               /* Get the CPU clk freq.                                */
    div      = (CPU_INT16U)(clk_freq / (16 * baud_rate));
    divlo    =  div & 0x00FF;                                   /* Split div into LOW and HIGH bytes.                   */
    divhi    = (div >> 8) & 0x00FF;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ------------------- SETUP UART0 -------------------- */
    LPC288X_REG_UART_LCR = DEF_BIT_07;                          /* Set div access bit.                                  */
    LPC288X_REG_UART_DLL = divlo;                               /* Load div.                                            */
    LPC288X_REG_UART_DLM = divhi;
    LPC288X_REG_UART_LCR = 0x03;
    LPC288X_REG_UART_IER = 0x00;                                /* Dis both Rx and Tx ints.                             */
    LPC288X_REG_UART_FCR = 0x05;                                /* Ena FIFO, flush Rx & Tx.                             */

                                                                /* ---------- INITIALIZE INTERRUPT FOR UART0 ---------- */
    BSP_IntVectSet(LPC288X_VIC_UART, ProbeRS232_RxTxISRHandler);
    BSP_IntPrioSet(LPC288X_VIC_UART, 5);
    BSP_IntEn(LPC288X_VIC_UART);
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
    iir = LPC288X_REG_UART_IIR & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                    */
                 lsr     = LPC288X_REG_UART_LSR;
                 rx_data = LPC288X_REG_UART_RBR;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                              */
                 break;

            default:
            case  0:                                            /* Modem interrupt?                                         */
            case  6:                                            /* Receive Line Status interrupt?                           */
            case 12:                                            /* CTI interrupt?                                           */
                 break;
        }
        iir = LPC288X_REG_UART_IIR & 0x0F;
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
    LPC288X_REG_UART_IER &= ~DEF_BIT_00;
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
    LPC288X_REG_UART_IER |=  DEF_BIT_00;
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
    LPC288X_REG_UART_THR = c;
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
    LPC288X_REG_UART_IER &= ~DEF_BIT_01;
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
    LPC288X_REG_UART_IER |=  DEF_BIT_01;
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
