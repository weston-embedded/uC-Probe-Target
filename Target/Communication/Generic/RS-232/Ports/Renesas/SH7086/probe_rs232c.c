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
*                                   Port for the Renesas SH7086
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

#define  PROBE_RS232                          SCI0

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

    void  ProbeRS232_Config   (void);
    void  ProbeRS232_EnSCIF   (void);
    void  ProbeRS232_SetVect  (void);

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


extern  void  ProbeRS232TeiISR (void);
extern  void  ProbeRS232EriISR (void);
extern  void  ProbeRS232RxISR  (void);
extern  void  ProbeRS232TxISR  (void);

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
    CPU_INT08U  baud_clk;
    CPU_INT16U  scbrr;

    ProbeRS232_EnSCIF();

    PROBE_RS232.SCSCR.BIT.TIE   =  0;                                   /* Disable transmit interrupts                      */
    PROBE_RS232.SCSCR.BIT.TE    =  0;                                   /* Disable transmitting                             */
    PROBE_RS232.SCSCR.BIT.RIE   =  0;                                   /* Disable receive interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  0;                                   /* Disable receiving                                */

    ProbeRS232_SetVect();

    PROBE_RS232.SCSSR.BIT.PER;
    PROBE_RS232.SCSSR.BIT.PER   =  0;
    PROBE_RS232.SCSSR.BIT.FER;
    PROBE_RS232.SCSSR.BIT.FER   =  0;
    PROBE_RS232.SCSSR.BIT.ORER;
    PROBE_RS232.SCSSR.BIT.ORER  =  0;
    PROBE_RS232.SCSCR.BIT.CKE   =  0;                                   /* Use internal clock, set SCK pin as input pin     */
    PROBE_RS232.SCSMR.BIT.CA    =  0;                                   /* Set for asynchronous mode                        */
    PROBE_RS232.SCSMR.BIT.CHR   =  0;                                   /* 8-bit data                                       */
    PROBE_RS232.SCSMR.BIT._PE   =  0;                                   /* Parity mode disabled                             */
    PROBE_RS232.SCSMR.BIT.OE    =  0;                                   /* No parity                                        */
    PROBE_RS232.SCSMR.BIT.STOP  =  0;                                   /* 1 stop bit                                       */
    baud_clk                    =  0;                                   /* Algorithm for finding SCBRR value                */

    do {                                                                /* N = SCBRR (0-255), B = bit rate, n = baud clock  */
        scbrr = SP_Scbrr(baud_clk, baud_rate);                                     /* N = Pclk / (64 * 2^(2 * n - 1) * B) - 1          */
        if (scbrr > 255) {
             baud_clk++;
        }
    } while (scbrr > 255);

    PROBE_RS232.SCSMR.BIT.CKS   =  baud_clk;
    PROBE_RS232.SCBRR           =  scbrr;

    ProbeRS232_Config();

    PROBE_RS232.SCSCR.BIT.TE    =  1;                                   /* Enable transmitting                              */
    PROBE_RS232.SCSCR.BIT.TIE   =  0;                                   /* Enable Transmit Interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  1;                                   /* Enable receiving                                 */
    PROBE_RS232.SCSCR.BIT.RIE   =  1;                                   /* Enable Receive Interrupts                        */
}


/*
*********************************************************************************************************
*                                        ProbeRS232_SetVect()
*
* Description : This function will set the interrupt vectors within the interrupts table.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_SetVect (void)
{
    CPU_INT08U vect;


    vect = 216 + ((PROBE_RS232_CFG_COMM_SEL - 1) * 4);

    INT_Vectors[vect++] = ProbeRS232EriISR;
    INT_Vectors[vect++] = ProbeRS232RxISR;
    INT_Vectors[vect++] = ProbeRS232TxISR;
    INT_Vectors[vect]   = ProbeRS232TeiISR;
}

/*
*********************************************************************************************************
*                                         ProbeRS232_EnSCIF()
*
* Description : Enables the SCIF module.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_EnSCIF (void)
{
    STB.CR3.BIT._SCI0 = 0;                                              /* Enable SCIF0 module                              */
}

/*
*********************************************************************************************************
*                                        ProbeRS232_Config()
*
* Description : Configures the necessary port pins.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  ProbeRS232_Config (void)
{
     PFC.PACRL1.BIT.PA2MD = 1;                                          /* Set pin as SCK0                                  */
     PFC.PACRL1.BIT.PA1MD = 1;                                          /* Set pin as TXD0                                  */
     PFC.PACRL1.BIT.PA0MD = 1;                                          /* Set pin as RXD0                                  */
     INTC.IPRL.BIT._SCI0  = 11;                                         /* Set the interrupt priority                       */
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
    CPU_INT08U  i;
    CPU_CHAR  rx_data;


    PROBE_RS232.SCSSR.BIT.ORER;
    PROBE_RS232.SCSSR.BIT.FER;
    PROBE_RS232.SCSSR.BIT.PER;

    while (PROBE_RS232.SCSSR.BIT.RDRF == 0) ;                           /* Wait until a read is requested                   */

    rx_data = PROBE_RS232.SCRDR;

    PROBE_RS232.SCSSR.BIT.RDRF;
    PROBE_RS232.SCSSR.BIT.RDRF = 0;

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
    PROBE_RS232.SCSCR.BIT.RIE    =  0;                                  /* Enable receiving                                 */
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
    PROBE_RS232.SCSCR.BIT.RIE   =  1;                                   /* Enable Transmit Interrupts                       */
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
    while (PROBE_RS232.SCSSR.BIT.TDRE == 0) ;                           /* Wait until FIFO has room                         */

    PROBE_RS232.SCTDR          = c;                                     /* Place character to be transmitted into register  */
    PROBE_RS232.SCSSR.BIT.TEND;
    PROBE_RS232.SCSSR.BIT.TEND = 0;                                     /* Clear transmission end flag                      */
    PROBE_RS232.SCSSR.BIT.TDRE;                                         /* Read then clear the TDFE flag                    */
    PROBE_RS232.SCSSR.BIT.TDRE = 0;
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
    PROBE_RS232.SCSCR.BIT.TIE   =  0;                                   /* Enable Transmit Interrupts                       */
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
    PROBE_RS232.SCSCR.BIT.TIE   =  1;                                   /* Enable Transmit Interrupts                       */
}

/*
*********************************************************************************************************
*                                      ProbeRS232_EriISRHandler()
*
* Description : Probe receive error handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void ProbeRS232_EriISRHandler (void)
{                                                                       /* Read each error flag and clear                   */
    PROBE_RS232.SCSSR.BIT.FER;
    PROBE_RS232.SCSSR.BIT.FER  = 0;
    PROBE_RS232.SCSSR.BIT.PER;
    PROBE_RS232.SCSSR.BIT.PER  = 0;
    PROBE_RS232.SCSSR.BIT.ORER;
    PROBE_RS232.SCSSR.BIT.ORER = 0;
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TeiISRHandler()
*
* Description : Probe transmit error handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void ProbeRS232_TeiISRHandler (void)
{
    PROBE_RS232.SCSSR.BIT.TDRE;                                         /* Read then clear the TDRE flag                    */
    PROBE_RS232.SCSSR.BIT.TDRE = 0;
}

#endif
