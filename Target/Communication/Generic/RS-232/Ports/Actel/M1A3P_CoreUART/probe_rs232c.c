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
*                                PORT FOR THE ACTEL CORTEX-M1 CoreUART
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

#include  <iocm1.h>
#include  <iom1a3p.h>
#include  <bsp.h>
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
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       ProbeRS232_InitTarget()
*
* Description : Initialize the UART for Probe communication.
*
* Argument(s) : baud_rate       Intended baud rate of the RS-232.
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
    clk_freq =  24000000;                                       /* Get the CPU clock frequency.                         */
    div      = (CPU_INT16U)(clk_freq / (16 * (baud_rate + 1)));



#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ------------------- SETUP UART0 -------------------- */
    M1A3P_REG_COREUART0_CTRL1 = div;                            /* Load divisor.                                        */
    M1A3P_REG_COREUART0_CTRL2 = DEF_BIT_00;                     /* 8-bit data, parity disabled.                         */



                                                                /* ---------- INITIALIZE INTERRUPT FOR UART0 ---------- */
    BSP_IntVectSet(M1A3P_INT_RXRDY0, ProbeRS232_RxISRHandler);
    BSP_IntVectSet(M1A3P_INT_TXRDY0, ProbeRS232_TxISRHandler);
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
    CPU_INT32U  status;
    CPU_INT08U  rx_data;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    status = M1A3P_REG_COREUART0_STATUS;
    if (DEF_BIT_IS_SET(status, DEF_BIT_02 | DEF_BIT_03) == DEF_YES) {
        rx_data = M1A3P_REG_COREUART0_RXDATA;
        (void)&rx_data;
    } else if (DEF_BIT_IS_SET(status, DEF_BIT_01) == DEF_YES) {
        rx_data = M1A3P_REG_COREUART0_RXDATA;
        ProbeRS232_RxHandler(rx_data);
    }
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    BSP_IntDis(M1A3P_INT_RXRDY0);
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
    BSP_IntEn(M1A3P_INT_RXRDY0);
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    M1A3P_REG_COREUART0_TXDATA = c;
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
    BSP_IntDis(M1A3P_INT_TXRDY0);
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
    BSP_IntEn(M1A3P_INT_TXRDY0);
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
