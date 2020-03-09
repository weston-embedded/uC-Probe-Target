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
*                                              uC/Probe
*
*                                      Communication: RS-232
*                                    H8SX/1664  Specific code
*
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

#include  "includes.h"


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

#define  PROBE_RS232    SCIF


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

void  ProbeRS232_Config         (void);
void  ProbeRS232_EnSCIF         (void);
void  ProbeRS232_RxTxISRHandler (void);
void  ProbeRS232_SetBaud        (CPU_INT16U baud);

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      ProbeRS232_InitTarget()
*
* Description : Initialize the UART for uC/Probe communication.
*
* Argument(s) : baud_rate   Intended baud rate of the RS-232.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT08U baud_clk;
    CPU_INT16U brr;


    ProbeRS232_EnSCIF();
    ProbeRS232_Config ();

    PROBE_RS232.SCIFCR.BIT.SCIFOE  = 3;                                 /* Enable port output                               */
    PROBE_RS232.SCIFCR.BIT.CKSEL   = 1;                                 /* Select 33Mhz L clk / 18 as the clock source      */
    PROBE_RS232.FIER.BYTE          = 0;                                 /* Disable interrupts                               */
    PROBE_RS232.FLCR.BIT.DLAB      = 1;                                 /* Enable the baud rate to be set                   */
    ProbeRS232_SetBaud(baud_rate);                                      /* Set baud rate                                    */
    PROBE_RS232.FLCR.BIT.DLAB      = 0;                                 /* Disable baud rate setting                        */
    PROBE_RS232.FLCR.BIT.CLS       = 3;                                 /* Specify character length as 8 bits               */
    PROBE_RS232.FIER.BIT.ERBFI     = 1;                                 /* Enable receive interrupts                        */
    PROBE_RS232.FMCR.BIT.OUT2      = 1;                                 /* Enable interrupts                                */
}

void  ProbeRS232_EnSCIF (void)
{                                                                       /* Enable SCIF module                               */
    LPC.HICR5.BIT.SCIFE          =  0;
    SYSTEM.SUBMSTPBL.BIT.SMSTPB3 =  0;
    SYSTEM.SUBMSTPAL.BIT.SMSTPA3 =  0;
}

void  ProbeRS232_Config (void)
{
    P5.DDR.BIT.P51DDR  = 1;                                             /* Set the data direction register                  */
    P5.DDR.BIT.P50DDR  = 0;
    INT.ICRD.BIT.ICRD1 = 0;                                             /* Set the interrupt priority                       */
}

void  ProbeRS232_SetBaud (CPU_INT16U baud)
{
    CPU_INT32U  tmp1;
    CPU_INT32U  tmp2;


    tmp1  = 16 * 11;
    tmp1 *= baud;
    tmp1  = BSP_CPU_CLKFREQ * 10 / tmp1;
    tmp2  = tmp1 / 10;

    if (tmp1 > (tmp2 * 10 + 4)) {
        tmp2++;
    }

    PROBE_RS232.FRBR = (CPU_INT16U) tmp2;
}

/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description : Handle Rx and Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may be empty if Rx and Tx interrupts are handled in the separate
*                   interrupt handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_CHAR rx_data;


    if (PROBE_RS232.FLSR.BIT.DR == 1) {
        rx_data  = (CPU_CHAR)PROBE_RS232.FRBR;

        ProbeRS232_RxHandler(rx_data);
    }

    if (PROBE_RS232.FLSR.BIT.THRE == 1) {
        ProbeRS232_TxHandler();
    }
}


/*
*********************************************************************************************************
*                                    ProbeRS232_RxISRHandler()
*
* Description : Handle Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may empty if Rx interrupts are handled by the combined Rx/Tx interrupt
*                   handler, ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{

}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntDis()
*
* Description : Disable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
    PROBE_RS232.FIER.BIT.ERBFI = 0;                                     /* Disable receive interrupts                       */
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntEn()
*
* Description : Enable Rx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
    PROBE_RS232.FIER.BIT.ERBFI = 1;                                     /* Enable receive interrupts                        */
}


/*
*********************************************************************************************************
*                                    ProbeRS232_TxISRHandler()
*
* Description : Handle Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : (1) This function may empty if Tx interrupts are handled by the combined Rx/Tx interrupt
*                   handler, ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{

}


/*
*********************************************************************************************************
*                                      ProbeRS232_Tx1()
*
* Description : Transmit one byte.
*
* Argument(s) : c           The byte to transmit.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
    while (PROBE_RS232.FLSR.BIT.THRE == 0) ;

    PROBE_RS232.FRBR = c;
}

/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntDis()
*
* Description : Disable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
    PROBE_RS232.FIER.BIT.ETBEI = 0;                                        /* Disable Transmit Interrupts                      */
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntEn()
*
* Description : Enable Tx interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
    PROBE_RS232.FIER.BIT.ETBEI = 1;                                        /* Enable Transmit Interrupts                       */
}

#endif
