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
*                                   Port for the Renesas SH7216
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
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

extern  void*  INT_Vectors[];

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
*********************************************************************************************************
*                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

extern  void  ProbeRS232_EriISRHandler (void);
extern  void  ProbeRS232_TeiISRHandler (void);
extern  void  ProbeRS232_RxISRHandler  (void);
extern  void  ProbeRS232_TxISRHandler  (void);
extern  void  ProbeRS232TeiISR         (void);
extern  void  ProbeRS232EriISR         (void);
extern  void  ProbeRS232RxISR          (void);
extern  void  ProbeRS232TxISR          (void);


#if (PROBE_COM_CFG_RS232_EN > 0)

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
    CPU_INT32U  i;
    
    SCI1.SCSMR.BIT.CKS   = 0;                                           /* P/1 clock                                        */
    SCI1.SCSMR.BIT.STOP  = 0;                                           /* 1 stop bit                                       */
    SCI1.SCSMR.BIT.OE    = 0;                                           /* Even parity though parity not enabled            */
    SCI1.SCSMR.BIT._PE   = 0;                                           /* Parity disabled                                  */
    SCI1.SCSMR.BIT.CHR   = 0;                                           /* 8-bit data length                                */
    SCI1.SCSMR.BIT.CA    = 0;                                           /* Asynchronous                                     */
        
    SCI1.SCBRR = BAUDRATE( baud_rate );

    for (i = 0; i < 1000000; i++);                                      /* Wait for at least 1 bit time                     */

    SCI1.SCSCR.BIT.CKE   =  0;                                          /* N = 0                                            */
    SCI1.SCSCR.BIT.TE    =  1;                                          /* Tx enabled                                       */
    SCI1.SCSCR.BIT.RE    =  1;                                          /* Rx enabled                                       */
    SCI1.SCSCR.BIT.TIE   =  0;                                          /* Tx interrupt disabled                            */
    SCI1.SCSCR.BIT.RIE   =  1;                                          /* Rx interrupt enabled                             */
}

void  ProbeRS232_EnSCIF (void)
{
    STB.CR5.BIT._SCI1    = 0;                                           /* Enable SCI1 in module stop register              */
}

void  ProbeRS232_Config (void)
{
                                                                        /* Enable the SCIF channel 1 pins                   */
    PFC.PACRH1.BIT.PA19MD = 6;                                          /* RxD1                                             */
    PFC.PACRH2.BIT.PA20MD = 6;                                          /* TxD1                                             */
    INTC.IPR16.BIT._SCI1  = OS_INTERRUPT_PRIORITY;
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
    CPU_CHAR  rx_data;
    
    
    rx_data = SCI1.SCRDR;
    SCI1.SCSSR.BIT.RDRF = 0;
    ProbeRS232_RxHandler(rx_data);
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
    SCI1.SCSCR.BIT.RIE = 0;                                             /* Enable receiving                                 */
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
    SCI1.SCSCR.BIT.RIE = 1;                                             /* Enable Receive Interrupts                       */
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
    ProbeRS232_TxHandler();
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
    if (SCI1.SCSSR.BIT.TDRE == 1) {
        SCI1.SCTDR = c;
        SCI1.SCSSR.BIT.TDRE = 0;
    }
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
    SCI1.SCSCR.BIT.TIE = 0;                                             /* Enable Transmit Interrupts                       */
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
    SCI1.SCSCR.BIT.TIE = 1;                                             /* Enable Transmit Interrupts                       */
}

void  ProbeRS232_EriISRHandler (void)
{
                                                                /* clear error flags */
    SCI1.SCSSR.BIT.PER  = 0;                                    /* clear parity error */
    SCI1.SCSSR.BIT.FER  = 0;                                    /* clear framing error */
    SCI1.SCSSR.BIT.ORER = 0;                                    /* clear overrun error */
}

void  ProbeRS232_TeiISRHandler (void)
{

}

#else

void  ProbeRS232_EriISRHandler (void)
{
    
}

void  ProbeRS232_TeiISRHandler (void)
{
    
}

void  ProbeRS232_RxISRHandler  (void)
{
    
}

void  ProbeRS232_TxISRHandler  (void)
{
    
}

#endif
