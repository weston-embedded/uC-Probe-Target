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
*                                   Port for the Renesas SH7201
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

#if      PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
#define  PROBE_RS232                        P_SCIF0
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
#define  PROBE_RS232                        P_SCIF1
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2
#define  PROBE_RS232                        P_SCIF2
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3
#define  PROBE_RS232                        P_SCIF3
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4
#define  PROBE_RS232                        P_SCIF4
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5
#define  PROBE_RS232                        P_SCIF5
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6
#define  PROBE_RS232                        P_SCIF6
#elif    PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7
#define  PROBE_RS232                        P_SCIF7
#endif


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

extern void ProbeRS232_BriISRHandler (void);
extern void ProbeRS232_EriISRHandler (void);
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
    INT8U  baud_clk;
    INT16U scbrr;
    INT32U vecttbl;


    ProbeRS232_EnSCIF();

    PROBE_RS232.SCSCR.BIT.TIE   =  0;               /* Disable transmit interrupts                      */
    PROBE_RS232.SCSCR.BIT.TE    =  0;               /* Disable transmitting                             */
    PROBE_RS232.SCSCR.BIT.RIE   =  0;               /* Disable receive interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  0;               /* Disable receiving                                */

    ProbeRS232_SetVect();

    PROBE_RS232.SCFCR.BIT.TFRST =  1;               /* Reset Transmit FIFO Data Register                */
    PROBE_RS232.SCFCR.BIT.RFRST =  1;               /* Reset Receive FIFO Data Register                 */
                                                    /* Read each flag before clearing                   */
    PROBE_RS232.SCFSR.BIT.ER;
    PROBE_RS232.SCFSR.BIT.ER    =  0;
    PROBE_RS232.SCFSR.BIT.DR;
    PROBE_RS232.SCFSR.BIT.DR    =  0;
    PROBE_RS232.SCFSR.BIT.BRK;
    PROBE_RS232.SCFSR.BIT.BRK   =  0;
    PROBE_RS232.SCLSR.BIT.ORER;
    PROBE_RS232.SCLSR.BIT.ORER  =  0;
    PROBE_RS232.SCSCR.BIT.CKE   =  0;               /* Use internal clock, set SCK pin as input pin     */
    PROBE_RS232.SCSMR.BIT.CA    =  0;               /* Set for asynchronous mode                        */
    PROBE_RS232.SCSMR.BIT.CHR   =  0;               /* 8-bit data                                       */
    PROBE_RS232.SCSMR.BIT.PE    =  0;               /* Parity mode disabled                             */
    PROBE_RS232.SCSMR.BIT.OE    =  0;               /* No parity                                        */
    PROBE_RS232.SCSMR.BIT.STOP  =  0;               /* 1 stop bit                                       */
    baud_clk                    =  0;               /* Algorithm for finding SCBRR value                */
    do {                                            /* N = SCBRR (0-255), B = bit rate, n = baud clock  */
        scbrr = SP_Scbrr(baud_clk, baud_rate);      /* N = Pclk / (64 * 2^(2 * n - 1) * B) - 1          */
        if (scbrr > 255) {
             baud_clk++;
        }
    } while (scbrr > 255);
    PROBE_RS232.SCSMR.BIT.CKS   =  baud_clk;
    PROBE_RS232.SCBRR           =  scbrr;
    PROBE_RS232.SCFCR.BIT.RTRG  =  3;               /* Set RDF  flag when 14 bytes are stored           */
    PROBE_RS232.SCFCR.BIT.TTGR  =  3;               /* Set TDFE flag when 0 bytes remaining in register */
    PROBE_RS232.SCFCR.BIT.TFRST =  0;               /* Disable reset of transmit FIFO data register     */
    PROBE_RS232.SCFCR.BIT.RFRST =  0;               /* Disable reset of receive FIFO data register      */
    ProbeRS232_Config();
    PROBE_RS232.SCSCR.BIT.TE    =  1;               /* Enable transmitting                              */
    PROBE_RS232.SCSCR.BIT.TIE   =  0;               /* Enable Transmit Interrupts                       */
    PROBE_RS232.SCSCR.BIT.RE    =  1;               /* Enable receiving                                 */
    PROBE_RS232.SCSCR.BIT.RIE   =  1;               /* Enable Receive Interrupts                        */
    PROBE_RS232.SCSCR.BIT.REIE  =  1;               /* Enable Receive Error Interrupts                  */
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


    vect = 180 + (PROBE_RS232_CFG_COMM_SEL - 1) * 4;
    gVectorTable[vect++] = ProbeRS232BriISR;
    gVectorTable[vect++] = ProbeRS232EriISR;
    gVectorTable[vect++] = ProbeRS232RxISR;
    gVectorTable[vect]   = ProbeRS232TxISR;
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
#if   PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0
    P_CPG.STBCR4.BIT.MSTP47 = 0;                    /* Enable SCIF0 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1
    P_CPG.STBCR4.BIT.MSTP46 = 0;                    /* Enable SCIF1 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2
    P_CPG.STBCR4.BIT.MSTP45 = 0;                    /* Enable SCIF2 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3
    P_CPG.STBCR4.BIT.MSTP44 = 0;                    /* Enable SCIF3 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4
    P_CPG.STBCR4.BIT.MSTP43 = 0;                    /* Enable SCIF4 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5
    P_CPG.STBCR4.BIT.MSTP42 = 0;                    /* Enable SCIF5 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6
    P_CPG.STBCR4.BIT.MSTP41 = 0;                    /* Enable SCIF6 module                              */
#elif PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7
    P_CPG.STBCR4.BIT.MSTP40 = 0;                    /* Enable SCIF7 module                              */
#endif
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
#if   PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_0
     P_PORT.D.CR3.BIT.MD9    =  1;
     P_PORT.D.CR3.BIT.MD8    =  1;                  /* Pin 113 as RXD0                                  */
     P_PORT.D.CR2.BIT.MD7    =  1;                  /* Pin 114 as TXD0                                  */
     P_INTC.IPR11.BIT._SCIF0 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_1
     P_PORT.D.CR4.BIT.MD12   =  1;
     P_PORT.D.CR3.BIT.MD11   =  1;                  /* Pin 110 as RXD1                                  */
     P_PORT.D.CR3.BIT.MD10   =  1;                  /* Pin 111 as TXD1                                  */
     P_INTC.IPR11.BIT._SCIF1 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_2
     P_PORT.B.CR6.BIT.MD22   =  4;
     P_PORT.B.CR6.BIT.MD21   =  4;                  /* Pin 162 as RXD2                                  */
     P_PORT.B.CR6.BIT.MD20   =  4;                  /* Pin 161 as TXD2                                  */
     P_INTC.IPR12.BIT._SCIF2 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_3
     P_PORT.B.CR8.BIT.MD30   =  3;
     P_PORT.B.CR8.BIT.MD29   =  3;                  /* Pin 172 as RXD3                                  */
     P_PORT.B.CR8.BIT.MD28   =  3;                  /* Pin 171 as TXD3                                  */
     P_INTC.IPR12.BIT._SCIF3 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_4
     P_PORT.D.CR2.BIT.MD6    =  2;
     P_PORT.D.CR2.BIT.MD5    =  2;                  /* Pin 116 as RXD4                                  */
     P_PORT.D.CR2.BIT.MD4    =  2;                  /* Pin 117 as TXD4                                  */
     P_INTC.IPR12.BIT._SCIF4 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_5
     P_PORT.C.CR2.BIT.MD6    =  2;
     P_PORT.C.CR2.BIT.MD5    =  2;                  /* Pin 78 as RXD5                                   */
     P_PORT.C.CR2.BIT.MD4    =  2;                  /* Pin 79 as TXD5                                   */
     P_INTC.IPR12.BIT._SCIF5 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_6
     P_PORT.B.CR7.BIT.MD26   =  4;
     P_PORT.B.CR7.BIT.MD25   =  4;                  /* Pin 166 as RXD6                                  */
     P_PORT.B.CR7.BIT.MD24   =  4;                  /* Pin 165 as TXD6                                  */
     P_INTC.IPR13.BIT._SCIF6 = 11;                  /* Set the interrupt priority                       */
#elif PROBE_RS232_CFG_COMM_SEL  ==  PROBE_RS232_UART_7
     P_PORT.F.CR1.BIT.MD2    =  2;
     P_PORT.F.CR1.BIT.MD1    =  2;                  /* Pin 96 as RXD7                                   */
     P_PORT.F.CR1.BIT.MD0    =  2;                  /* Pin 95 as TXD7                                   */
     P_INTC.IPR13.BIT._SCIF7 = 11;                  /* Set the interrupt priority                       */
#endif
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
    INT8U  i;
    INT8U  rx_data;


    while (!(PROBE_RS232.SCFSR.BIT.RDF
          || PROBE_RS232.SCFSR.BIT.DR)) ;           /* Wait until a read is requested                   */
    if (PROBE_RS232.SCFSR.BIT.RDF) {                /* Read then clear the correct read flag            */
        while (PROBE_RS232.SCFDR.BIT.R) {
            rx_data = PROBE_RS232.SCFRDR;
            ProbeRS232_RxHandler(rx_data);
        }
        PROBE_RS232.SCFSR.BIT.RDF;
        PROBE_RS232.SCFSR.BIT.RDF = 0;
    } else if (PROBE_RS232.SCFSR.BIT.DR) {
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
    PROBE_RS232.SCSCR.BIT.RIE    =  0;              /* Enable receiving                                 */
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
    PROBE_RS232.SCSCR.BIT.RIE   =  1;               /* Enable Transmit Interrupts                       */
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
    PROBE_RS232.SCFSR.BIT.TDFE;                     /* Read then clear the TDFE flag                    */
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
    while (!PROBE_RS232.SCFSR.BIT.TDFE) ;            /* Wait until FIFO has room                         */
    while (PROBE_RS232.SCFDR.BIT.T == 0x10) ;        /* Check if register is full                        */
    PROBE_RS232.SCFTDR         = c;                 /* Place character to be transmitted into register  */
    PROBE_RS232.SCFSR.BIT.TEND;
    PROBE_RS232.SCFSR.BIT.TEND = 0;                 /* Clear transmission end flag                      */
    PROBE_RS232.SCFSR.BIT.TDFE;                     /* Read then clear the TDFE flag                    */
    PROBE_RS232.SCFSR.BIT.TDFE = 0;
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
    PROBE_RS232.SCSCR.BIT.TIE   =  0;               /* Enable Transmit Interrupts                       */
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
    PROBE_RS232.SCSCR.BIT.TIE   =  1;               /* Enable Transmit Interrupts                       */
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
    if (PROBE_RS232.SCFSR.BIT.BRK) {
        PROBE_RS232.SCFSR.BIT.BRK;
        PROBE_RS232.SCFSR.BIT.BRK  = 0;
    }
    if (PROBE_RS232.SCLSR.BIT.ORER) {
        PROBE_RS232.SCLSR.BIT.ORER;
        PROBE_RS232.SCLSR.BIT.ORER = 0;
    }
}

#endif
