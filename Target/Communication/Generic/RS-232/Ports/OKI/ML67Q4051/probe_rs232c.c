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
*                                     PORT FOR THE OKI ML67Q4051
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
    INT16U  baudDiv;
    INT8U   baudDivH;
    INT8U   baudDivL;
    INT32U  APB_BusClkFrq;


    OSProbe_TmrInit();                                                      /* Initialize the free running timer.                   */

    APB_BusClkFrq       =   BSP_APB_ClkFrq();                               /* Get the APB bus clock frequency                      */

    baudDiv             =  (INT16U)(APB_BusClkFrq / (16 * baud_rate));      /* Calculate the UART divider for the specified baud    */
    baudDivH            =  (baudDiv >> 8);                                  /* Calculate the high part of the divider value         */
    baudDivL            =  (baudDiv & 0xFF);                                /* Calculate the low part of the divider value          */

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    set_wbit(PECLKCNT, 0x00000008);                                         /* Enable UART0 clock                                   */

    set_bit(UARTLCR0, UARTLCR_DLAB);                                        /* DLAB =  1 before writing UARTDLL / UARTDLM           */
    put_value(UARTDLL0, baudDivL);                                          /* Write the high part of the divider value             */
    put_value(UARTDLM0, baudDivH);                                          /* Write the low part of the divider value              */
    clr_bit(UARTLCR0, UARTLCR_DLAB);                                        /* DLAB =  0 make Rx Tx data registers accessible       */

    put_value(UARTLCR0, UARTLCR_LEN8 | UARTLCR_STB1 | UARTLCR_PDIS);        /* data length: 8bit, stop bit: 1, parity: none         */

    set_wbit(PORTSEL1, 0x00050000);                                         /* Enable UART0 Rx and Tx pins                          */
    BSP_Set_IRQ_Vector(INT_UART0, ProbeRS232_RxTxISRHandler);               /* Configure the interrupt controller                   */
    set_wbit(ILC1, ILC1_ILR9 & ILC0_INT_LV1);                               /* Set the UART interrupt priority level to 1 (lowest)  */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    set_wbit(PECLKCNT, 0x00000010);                                         /* Enable UART1 clock                                   */

    set_bit(UARTLCR1, UARTLCR_DLAB);                                        /* DLAB =  1 before writing UARTDLL / UARTDLM           */
    put_value(UARTDLL1, baudDivL);                                          /* Write the high part of the divider value             */
    put_value(UARTDLM1, baudDivH);                                          /* Write the low part of the divider value              */
    clr_bit(UARTLCR1, UARTLCR_DLAB);                                        /* DLAB =  0 make Rx Tx data registers accessible       */

    put_value(UARTLCR1, UARTLCR_LEN8 | UARTLCR_STB1 | UARTLCR_PDIS);        /* data length: 8bit, stop bit: 1, parity: none         */

    set_wbit(PORTSEL1, 0x00500000);                                         /* Enable UART1 Rx and Tx pins                          */
    BSP_Set_IRQ_Vector(INT_UART1, ProbeRS232_RxTxISRHandler);               /* Configure the interrupt controller                   */
    set_wbit(ILC1, ILC1_ILR12 & ILC0_INT_LV1);                              /* Set the UART interrupt priority level to 1 (lowest)  */
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
    INT8U  intType;
    INT8U  status;
    INT8U  rx_data;


#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    intType     = get_value(UARTIIR0) & 0xFF;                               /* Get the interrupt type                               */

    switch (intType) {
        case UARTIIR_RCV:                                                   /* Receive interrupt                                    */
             rx_data = get_value(UARTRBR0) & 0xFF;                          /* Read the UART data register                          */
             ProbeRS232_RxHandler(rx_data);                                 /* Call the generic Rx handler                          */
             break;

        case UARTIIR_TRA:                                                   /* Transmit interrupt                                   */
             ProbeRS232_TxHandler();                                        /* Call the Tx handler                                  */
             break;

        case UARTIIR_LINE:                                                  /* Line status interrupt (overrun, framing error etc... */
             break;

        case UARTIIR_TO:                                                    /* Timeout interrupt                                    */
             break;

        default:
             break;
    }

    status      = get_value(UARTLSR0);                                      /* Read status register and clear the interrupt source  */
   (void)status;                                                            /* Avoid compiler warning 'status set but not used'     */

    set_wbit(CILCL,  (1 << INT_UART0));                                     /* Clear the interrupt controller interrupt             */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    intType     = get_value(UARTIIR1) & 0xFF;                               /* Get the interrupt type                               */

    switch (intType) {
        case UARTIIR_RCV:                                                   /* Receive interrupt                                    */
             rx_data = get_value(UARTRBR1) & 0xFF;                          /* Read the UART data register                          */
             ProbeRS232_RxHandler(rx_data);                                 /* Call the generic Rx handler                          */
             break;

        case UARTIIR_TRA:                                                   /* Transmit interrupt                                   */
             ProbeRS232_TxHandler();                                        /* Call the Tx handler                                  */
             break;

        case UARTIIR_LINE:                                                  /* Line status interrupt (overrun, framing error etc... */
             break;

        case UARTIIR_TO:                                                    /* Timeout interrupt                                    */
             break;

        default:
             break;
    }

    status      = get_value(UARTLSR1);                                      /* Read status register to clear line status Int. flags */
   (void)status;                                                            /* Avoid compiler warning 'status set but not used'     */

    set_wbit(CILCL,  (1 << INT_UART1));                                     /* Clear the interrupt controller interrupt             */
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
#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    clr_bit(UARTIER0, UARTIER_ERBF);                                        /* Disable Rx interrupts                                */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    clr_bit(UARTIER1, UARTIER_ERBF);                                        /* Disable Rx interrupts                                */
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
#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    set_bit(UARTIER0, UARTIER_ERBF);                                        /* Enable Rx interrupts                                 */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    set_bit(UARTIER1, UARTIER_ERBF);                                        /* Enable Rx interrupts                                 */
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
#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    put_value(UARTTHR0, c);                                                 /* Transmit 1 byte                                      */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    put_value(UARTTHR1, c);                                                 /* Transmit 1 byte                                      */
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
#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    clr_bit(UARTIER0, UARTIER_ETBEF);                                       /* Disable Tx interrupts                                */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    clr_bit(UARTIER1, UARTIER_ETBEF);                                       /* Disable Tx interrupts                                */
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
#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    set_bit(UARTIER0, UARTIER_ETBEF);                                       /* Enable Tx interrupts                                */
#endif

#if PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    set_bit(UARTIER1, UARTIER_ETBEF);                                       /* Enable Tx interrupts                                */
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
