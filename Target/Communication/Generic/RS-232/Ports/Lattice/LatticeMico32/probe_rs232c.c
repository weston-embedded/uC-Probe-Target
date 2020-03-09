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
*                                        PORT FOR LatticeMico32
*
* Filename : probe_rs232c.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file expects that the UART instance to be used with uC/Probe is named "uart".
*                If a UART with a different name will be used, please modify 'RS232_INSTANCE_NAME'.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include  <MicoUART.h>


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
*                                          LOCAL CONSTANTS
*********************************************************************************************************
*/

const  CPU_CHAR  *RS232_INSTANCE_NAME = "uart";


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

MicoUartCtx_t         *ProbeRS232C_UartCtx;
volatile  MicoUart_t  *ProbeRS232C_Uart;


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

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be  PROBE_RS232_UART_0]     "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be  PROBE_RS232_UART_0]     "
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
    MicoUartCtx_t         *ctx;
    volatile  MicoUart_t  *dev;


    ctx = (MicoUartCtx_t *)MicoGetDevice(RS232_INSTANCE_NAME);
    if(ctx == (MicoUartCtx_t *)0){
        printf("ERROR: failed to find UART instance named %s\n", RS232_INSTANCE_NAME);
        ProbeRS232C_UartCtx = (MicoUartCtx_t *)0;
        ProbeRS232C_Uart    = (volatile MicoUart_t *)0;
        return;
    }

    dev = (volatile MicoUart_t *)(ctx->base);

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* --------------------- SETUP UART0 ---------------------- */
    MicoUart_setRate(ctx, baud_rate);                           /* Set baud rate.                                           */
    MicoUart_dataConfig(ctx, 8, 0, 0, 1);                       /* 8 data bits, 0 parity bits, 1 stop bit.                  */

                                                                /* ------------ INITIALIZE INTERRUPT FOR UART0 ------------ */
    MicoDisableInterrupt(ctx->intrLevel);                       /* Disable UART0 interrupts.                                */
    ctx->ier = 0;                                               /* Disable all UART0 interrupt sources.                     */
    dev->ier = ctx->ier;
    MicoRegisterISR(ctx->intrLevel, ctx, ProbeRS232_RxTxISRHandler);   /* Register ISR handler.                             */
    MicoEnableInterrupt(ctx->intrLevel);                        /* Enable UART0 interrupts.                                 */

    ProbeRS232C_UartCtx = ctx;
    ProbeRS232C_Uart    = dev;
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
    volatile  MicoUart_t  *dev;
    CPU_INT32U             iir;
    CPU_INT08U             rx_data;
    CPU_INT32U             lsr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    dev = ProbeRS232C_Uart;

    iir = (dev->iir) & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case 2:                                             /* Transmitted character?                                   */
                 ProbeRS232_TxHandler();
                 break;

            case 4:                                             /* Received a character?                                    */
                 rx_data = (CPU_INT08U)dev->rxtx;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler.                             */
                 break;

            case  6:                                            /* Receive Line Status interrupt?                           */
                 lsr     = (CPU_INT32U)dev->lsr;                /* Line status interrupt?                                   */
                 break;

            default:
                 break;
        }
        iir = (dev->iir) & 0x0F;
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
    MicoDisableInterrupt(ProbeRS232C_UartCtx->intrLevel);
    ProbeRS232C_Uart->ier &= ~DEF_BIT_00;
    MicoEnableInterrupt(ProbeRS232C_UartCtx->intrLevel);
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
    MicoDisableInterrupt(ProbeRS232C_UartCtx->intrLevel);
    ProbeRS232C_Uart->ier |= DEF_BIT_00;
    MicoEnableInterrupt(ProbeRS232C_UartCtx->intrLevel);
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
    ProbeRS232C_Uart->rxtx = c;
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
    MicoDisableInterrupt(ProbeRS232C_UartCtx->intrLevel);
    ProbeRS232C_Uart->ier &= ~DEF_BIT_01;
    MicoEnableInterrupt(ProbeRS232C_UartCtx->intrLevel);
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
    MicoDisableInterrupt(ProbeRS232C_UartCtx->intrLevel);
    ProbeRS232C_Uart->ier |= DEF_BIT_01;
    MicoEnableInterrupt(ProbeRS232C_UartCtx->intrLevel);
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
