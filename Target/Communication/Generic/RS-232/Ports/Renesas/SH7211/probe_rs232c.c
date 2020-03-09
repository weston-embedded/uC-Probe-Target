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
*                                   Port for the Renesas SH7211
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

#define  PROBE_RS232                          SCIF2

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

void  ProbeRS232_Config  (void);
void  ProbeRS232_EnSCIF  (void);
void  ProbeRS232_SetVect (void);

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

extern void ProbeRS232_EriISRHandler (void);
extern void ProbeRS232_BriISRHandler (void);
extern void ProbeRS232_RxISRHandler  (void);
extern void ProbeRS232_TxISRHandler  (void);
extern void ProbeRS232BriISR         (void);
extern void ProbeRS232EriISR         (void);
extern void ProbeRS232RxISR          (void);
extern void ProbeRS232TxISR          (void);

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
    CPU_INT16U scbrr;
    CPU_INT32U vecttbl;


    ProbeRS232_EnSCIF();

    PROBE_RS232.SCSCR.BIT.TIE   =  0;                                   /* Disable transmit interrupts                      */
    PROBE_RS232.SCSCR.BIT.TE    =  0;                                   /* Disable transmitting                             */
    PROBE_RS232.SCSCR.BIT.RIE   =  0;                                   /* Disable receive interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  0;                                   /* Disable receiving                                */

    ProbeRS232_SetVect();

    PROBE_RS232.SCFCR.BIT.TFRST =  1;                                   /* Reset Transmit FIFO Data Register                */
    PROBE_RS232.SCFCR.BIT.RFRST =  1;                                   /* Reset Receive FIFO Data Register                 */
                                                                        /* Read each flag before clearing                   */
    PROBE_RS232.SCFSR.BIT.ER;
    PROBE_RS232.SCFSR.BIT.ER    =  0;
    PROBE_RS232.SCFSR.BIT.DR;
    PROBE_RS232.SCFSR.BIT.DR    =  0;
    PROBE_RS232.SCFSR.BIT.BRK;
    PROBE_RS232.SCFSR.BIT.BRK   =  0;
    PROBE_RS232.SCLSR.BIT.ORER;
    PROBE_RS232.SCLSR.BIT.ORER  =  0;
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
    PROBE_RS232.SCFCR.BIT.RTRG  =  3;                                   /* Set RDF  flag when 14 bytes are stored           */
    PROBE_RS232.SCFCR.BIT.TTRG  =  3;                                   /* Set TDFE flag when 0 bytes remaining in register */
    PROBE_RS232.SCFCR.BIT.TFRST =  0;                                   /* Disable reset of transmit FIFO data register     */
    PROBE_RS232.SCFCR.BIT.RFRST =  0;                                   /* Disable reset of receive FIFO data register      */

    ProbeRS232_Config();

    PROBE_RS232.SCSCR.BIT.TE    =  1;                                   /* Enable transmitting                              */
    PROBE_RS232.SCSCR.BIT.TIE   =  0;                                   /* Enable Transmit Interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  1;                                   /* Enable receiving                                 */
    PROBE_RS232.SCSCR.BIT.RIE   =  1;                                   /* Enable Receive Interrupts                        */
    PROBE_RS232.SCSCR.BIT.REIE  =  1;                                   /* Enable Receive Error Interrupts                  */
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
    INT8U vect;


    vect = 240 + (PROBE_RS232_CFG_COMM_SEL - 1) * 4;
    INT_Vectors[vect++] = ProbeRS232BriISR;
    INT_Vectors[vect++] = ProbeRS232EriISR;
    INT_Vectors[vect++] = ProbeRS232RxISR;
    INT_Vectors[vect]   = ProbeRS232TxISR;
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
    STB.CR4.BIT._SCIF2 = 0;                                             /* Enable SCIF2 module                              */

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
     PFC.PBCRH2.BIT.PB23MD = 5;                                         /* Set pin as TXD2                                  */
     PFC.PBCRH2.BIT.PB22MD = 5;                                         /* Set pin as RXD2                                  */
     INTC.IPR14.BIT._SCIF2 = 11;                                        /* Set the interrupt priority                       */
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


    while (PROBE_RS232.SCFSR.BIT.RDF == 0
        && PROBE_RS232.SCFSR.BIT.DR  == 0) ;                            /* Wait until a read is requested                   */

    if (PROBE_RS232.SCFSR.BIT.RDF == 1) {                               /* Read then clear the correct read flag            */
        while (PROBE_RS232.SCFDR.BIT.R > 0) {
            rx_data = PROBE_RS232.SCFRDR;
            ProbeRS232_RxHandler(rx_data);
        }

        PROBE_RS232.SCFSR.BIT.RDF;
        PROBE_RS232.SCFSR.BIT.RDF = 0;

    } else if (PROBE_RS232.SCFSR.BIT.DR == 1) {
        i = PROBE_RS232.SCFDR.BIT.R;

        for (i; i > 0; i--) {
            rx_data = PROBE_RS232.SCFRDR;
            ProbeRS232_RxHandler(rx_data);
        }

        PROBE_RS232.SCFSR.BIT.DR;
        PROBE_RS232.SCFSR.BIT.DR = 0;
    }
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

    PROBE_RS232.SCFSR.BIT.TDFE;                                         /* Read then clear the TDFE flag                    */
    PROBE_RS232.SCFSR.BIT.TDFE = 0;
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
    while (PROBE_RS232.SCFSR.BIT.TDFE ==    0) ;                        /* Wait until FIFO has room                         */
    while (PROBE_RS232.SCFDR.BIT.T    == 0x10) ;                        /* Check if register is full                        */

    PROBE_RS232.SCFTDR              = c;                                /* Place character to be transmitted into register  */
    PROBE_RS232.SCFSR.BIT.TEND;
    PROBE_RS232.SCFSR.BIT.TEND      = 0;                                /* Clear transmission end flag                      */
    PROBE_RS232.SCFSR.BIT.TDFE;                                         /* Read then clear the TDFE flag                    */
    PROBE_RS232.SCFSR.BIT.TDFE      = 0;
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
{
    PROBE_RS232.SCFSR.BIT.ER;
    PROBE_RS232.SCFSR.BIT.ER  = 0;
}

/*
*********************************************************************************************************
*                                      ProbeRS232_BriISRHandler()
*
* Description : Probe break error handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

void ProbeRS232_BriISRHandler (void)
{
    if (PROBE_RS232.SCFSR.BIT.BRK == 1) {
        PROBE_RS232.SCFSR.BIT.BRK;
        PROBE_RS232.SCFSR.BIT.BRK  = 0;
    }

    if (PROBE_RS232.SCLSR.BIT.ORER == 1) {
        PROBE_RS232.SCLSR.BIT.ORER;
        PROBE_RS232.SCLSR.BIT.ORER = 0;
    }
}

#endif
