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
*                                       PORT FOR TOSHIBA TMPM369
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
#include  <bsp.h>
#include  <math.h>



/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_rs232.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                       */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* -------- SERIAL CHANNEL (SIO) BASE ADDRESS --------- */
#define  TMPM369_SIO_CH0_BASE_ADDR              0x400E1000u
#define  TMPM369_SIO_CH1_BASE_ADDR              0x400E1100u
#define  TMPM369_SIO_CH2_BASE_ADDR              0x400E1200u
#define  TMPM369_SIO_CH3_BASE_ADDR              0x400E1300u

                                                                /* -------------- SERIAL ENABLE REGISTER -------------- */
#define  TMPM369_SCEN_BIT_SIO_EN                DEF_BIT_00

                                                                /* ----------------- CONTROL REGISTER ----------------- */
#define  TMPM369_SCCR_BIT_IOC                   DEF_BIT_00
#define  TMPM369_SCCR_BIT_SCLKS                 DEF_BIT_01
#define  TMPM369_SCCR_BIT_FERR                  DEF_BIT_02
#define  TMPM369_SCCR_BIT_PERR                  DEF_BIT_03
#define  TMPM369_SCCR_BIT_OERR                  DEF_BIT_04

#define  TMPM369_SCCR_BIT_ERR_FLAG_ALL         (TMPM369_SCCR_BIT_FERR | TMPM369_SCCR_BIT_PERR | TMPM369_SCCR_BIT_OERR)

#define  TMPM369_SCCR_BIT_PE                    DEF_BIT_05
#define  TMPM369_SCCR_BIT_EVEN                  DEF_BIT_06
#define  TMPM369_SCCR_BIT_RB8                   DEF_BIT_07

                                                                /* -------------- MODE CONTROL REGISTER 0 ------------- */
#define  TMPM369_SCMOD0_BIT_SC_TMR9             DEF_BIT_MASK(0u, 0u)
#define  TMPM369_SCMOD0_BIT_SC_BAUD             DEF_BIT_MASK(1u, 0u)
#define  TMPM369_SCMOD0_BIT_SC_INT_CLK          DEF_BIT_MASK(2u, 0u)
#define  TMPM369_SCMOD0_BIT_SC_EXT_CLK          DEF_BIT_MASK(3u, 0u)

#define  TMPM369_SCMOD0_BIT_SM_IO               DEF_BIT_MASK(0u, 2u)
#define  TMPM369_SCMOD0_BIT_SM_7_BIT            DEF_BIT_MASK(1u, 2u)
#define  TMPM369_SCMOD0_BIT_SM_8_BIT            DEF_BIT_MASK(2u, 2u)
#define  TMPM369_SCMOD0_BIT_SM_9_BIT            DEF_BIT_MASK(3u, 2u)

#define  TMPM369_SCMOD0_BIT_WU                  DEF_BIT_04
#define  TMPM369_SCMOD0_BIT_RXE                 DEF_BIT_05
#define  TMPM369_SCMOD0_BIT_CTSE                DEF_BIT_06
#define  TMPM369_SCMOD0_BIT_TB8                 DEF_BIT_07

                                                                /* -------------- MODE CONTROL REGISTER 1 ------------- */
#define  TMPM369_SCMOD1_BIT_SINT_1SCLK          DEF_BIT_MASK(1u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_2SCLK          DEF_BIT_MASK(2u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_4SCLK          DEF_BIT_MASK(3u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_8SCLK          DEF_BIT_MASK(4u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_16SCLK         DEF_BIT_MASK(5u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_32SCLK         DEF_BIT_MASK(6u, 1u)
#define  TMPM369_SCMOD1_BIT_SINT_64SCLK         DEF_BIT_MASK(7u, 1u)

#define  TMPM369_SCMOD1_BIT_TXE                 DEF_BIT_04

#define  TMPM369_SCMOD1_BIT_FDPX_NO_TRANS       DEF_BIT_MASK(0u, 5u)
#define  TMPM369_SCMOD1_BIT_FDPX_RX_HALF        DEF_BIT_MASK(1u, 5u)
#define  TMPM369_SCMOD1_BIT_FDPX_TX_HALF        DEF_BIT_MASK(2u, 5u)
#define  TMPM369_SCMOD1_BIT_FDPX_FULL_DUPLEX    DEF_BIT_MASK(3u, 5u)

#define  TMPM369_SCMOD1_BIT_I2SC                DEF_BIT_07

                                                                /* -------------- MODE CONTROL REGISTER 2 ------------- */
#define  TMPM369_SCMOD2_BIT_SWRST               DEF_BIT_MASK(1u, 0u)

#define  TMPM369_SCMOD2_BIT_WBUF                DEF_BIT_02
#define  TMPM369_SCMOD2_BIT_DRCHG               DEF_BIT_03
#define  TMPM369_SCMOD2_BIT_SBLEN               DEF_BIT_04
#define  TMPM369_SCMOD2_BIT_TXRUN               DEF_BIT_05
#define  TMPM369_SCMOD2_BIT_RBFLL               DEF_BIT_06
#define  TMPM369_SCMOD2_BIT_TBEMP               DEF_BIT_07

                                                                /* -------- BAUD RATE GENERATOR CONTROL REGISTER ------ */
#define  TMPM369_SCBRCR_BIT_BRCK_T1             DEF_BIT_MASK(0u, 4u)
#define  TMPM369_SCBRCR_BIT_BRCK_T4             DEF_BIT_MASK(1u, 4u)
#define  TMPM369_SCBRCR_BIT_BRCK_T16            DEF_BIT_MASK(2u, 4u)
#define  TMPM369_SCBRCR_BIT_BRCK_T64            DEF_BIT_MASK(3u, 4u)

#define  TMPM369_SCBRCR_BIT_BRADDE              DEF_BIT_06

                                                                /* ------------ FIFO CONFIGURATION REGISTER ----------- */
#define  TMPM369_SCFCNF_BIT_CNFG                DEF_BIT_00
#define  TMPM369_SCFCNF_BIT_RXTXCNT             DEF_BIT_01
#define  TMPM369_SCFCNF_BIT_RFIE                DEF_BIT_02
#define  TMPM369_SCFCNF_BIT_TFIE                DEF_BIT_03
#define  TMPM369_SCFCNF_BIT_RFST                DEF_BIT_04

                                                                /* ----------- RX FIFO CONFIGURATION REGISTER --------- */
#define  TMPM369_SCRFC_BIT_RFIS                 DEF_BIT_06
#define  TMPM369_SCRFC_BIT_RFCS                 DEF_BIT_07

                                                                /* ----------- TX FIFO CONFIGURATION REGISTER --------- */
#define  TMPM369_SCTFC_BIT_TFIS                 DEF_BIT_06
#define  TMPM369_SCTFC_BIT_TFCS                 DEF_BIT_07

                                                                /* ----------- RX FIFO CONFIGURATION REGISTER --------- */
#define  TMPM369_SCRST_BIT_RLVL_EMPTRY          DEF_BIT_MASK(0u, 0u)
#define  TMPM369_SCRST_BIT_RLVL_1_BYTE          DEF_BIT_MASK(1u, 0u)
#define  TMPM369_SCRST_BIT_RLVL_2_BYTE          DEF_BIT_MASK(2u, 0u)
#define  TMPM369_SCRST_BIT_RLVL_3_BYTE          DEF_BIT_MASK(3u, 0u)
#define  TMPM369_SCRST_BIT_RLVL_4_BYTE          DEF_BIT_MASK(4u, 0u)

#define  TMPM369_SCRST_BIT_ROR                  DEF_BIT_07

                                                                /* ----------- TX FIFO CONFIGURATION REGISTER --------- */
#define  TMPM369_SCTST_BIT_TLVL_EMPTRY          DEF_BIT_MASK(0u, 0u)
#define  TMPM369_SCTST_BIT_TLVL_1_BYTE          DEF_BIT_MASK(1u, 0u)
#define  TMPM369_SCTST_BIT_TLVL_2_BYTE          DEF_BIT_MASK(2u, 0u)
#define  TMPM369_SCTST_BIT_TLVL_3_BYTE          DEF_BIT_MASK(3u, 0u)
#define  TMPM369_SCTST_BIT_TLVL_4_BYTE          DEF_BIT_MASK(4u, 0u)

#define  TMPM369_SCTST_BIT_TUR                  DEF_BIT_07

                                                                /* ----------- PORT E UART COMMUNICATION PINS --------- */
#define  TMPM369_PORTE_PEFR1_PE1F1              DEF_BIT_01
#define  TMPM369_PORTE_PEFR1_PE2F1              DEF_BIT_02
#define  TMPM369_PORTE_PEFR1_PE5F1              DEF_BIT_05
#define  TMPM369_PORTE_PEFR1_PE6F1              DEF_BIT_06

#define  TMPM369_PORTE_PEFR4_PE3F4              DEF_BIT_03
#define  TMPM369_PORTE_PEFR4_PE4F4              DEF_BIT_04

#define  TMPM369_PORTE_PEIE_PE1IE               DEF_BIT_01
#define  TMPM369_PORTE_PEIE_PE3IE               DEF_BIT_03
#define  TMPM369_PORTE_PEIE_PE4IE               DEF_BIT_04
#define  TMPM369_PORTE_PEIE_PE6IE               DEF_BIT_06

#define  TMPM369_PORTE_PECR_PE2C                DEF_BIT_02
#define  TMPM369_PORTE_PECR_PE5C                DEF_BIT_05

                                                                /* ----------- PORT L UART COMMUNICATION PINS --------- */
#define  TMPM369_PORTL_PLFR5_PL1F5              DEF_BIT_01
#define  TMPM369_PORTL_PLFR5_PL2F5              DEF_BIT_02
#define  TMPM369_PORTL_PLFR6_PL3F6              DEF_BIT_03

#define  TMPM369_PORTL_PLIE_PL1IE               DEF_BIT_01
#define  TMPM369_PORTL_PLIE_PL3IE               DEF_BIT_03

#define  TMPM369_PORTL_PLCR_PL2C                DEF_BIT_02

                                                                /* ----------- PORT B UART COMMUNICATION PINS --------- */
#define  TMPM369_PORTB_PBFR3_PB0F3              DEF_BIT_00
#define  TMPM369_PORTB_PBFR3_PB1F3              DEF_BIT_01

#define  TMPM369_PORTB_PBIE_PB1IE               DEF_BIT_01
#define  TMPM369_PORTB_PBCR_PB0C                DEF_BIT_00

                                                                /* ----------- PORT A UART COMMUNICATION PINS --------- */
#define  TMPM369_PORTA_PAFR4_PA7F4              DEF_BIT_07
#define  TMPM369_PORTA_PAIE_PA7IE               DEF_BIT_07


/*
*********************************************************************************************************
*                                           EXTERNAL FUNCTIONS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/
                                                                /* ------------ SERIAL CHANNEL REGISTER MAP ----------- */
typedef struct tmpm369_ser_reg {
    volatile  CPU_INT32U  SCEN;                                 /* Enable Register                                      */
    volatile  CPU_INT32U  SCBUF;                                /* Buffer Register                                      */
    volatile  CPU_INT32U  SCCR;                                 /* Control Register                                     */
    volatile  CPU_INT32U  SCMOD0;                               /* Mode Control Register 0                              */
    volatile  CPU_INT32U  SCBRCR;                               /* Baud Rate Control Register                           */
    volatile  CPU_INT32U  SCBRADD;                              /* Baud Rate Control Register 2                         */
    volatile  CPU_INT32U  SCMOD1;                               /* Mode Control Register 1                              */
    volatile  CPU_INT32U  SCMOD2;                               /* Mode Control Register 2                              */
    volatile  CPU_INT32U  SCRFC;                                /* Rx FIFO Configuration Register                       */
    volatile  CPU_INT32U  SCTFC;                                /* Tx FIFO Configuration Register                       */
    volatile  CPU_INT32U  SCRST;                                /* Rx FIFO Status Register                              */
    volatile  CPU_INT32U  SCTST;                                /* Tx FIFO Status Register                              */
    volatile  CPU_INT32U  SCFCNF;                               /* FIFO Configuration Register                          */    
} TMPM369_SER_REG;


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL MACRO(S)
*********************************************************************************************************
*/

#define  TMPM369_SCBRCR_BIT_BRS(x)              DEF_BIT_MASK(((x) & 0x0Fu), 0u)

                                                                /* Fill Level(s) for Tx & Rx FIFO based on Duplex.      */
#define  TMPM369_SCxFC_BIT_xIL(x)               DEF_BIT_MASK(((x) & 0x03u), 0u)


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
*
*               (3) The baud rate is calculated as follows:
*
*                                             Baud rate generator input clock (1/2, 1/8, 1/32, 1/128)
*                               Baud Rate = -------------------------------------
*                                                        16 * N
*                   where N is the Baud rate divisor.
*            
*                         
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    TMPM369_SER_REG  *p_ser_reg;
    CPU_INT32U        periph_clk_freq;
    CPU_INT32U        baud_div;
    CPU_INT32U        n_val;
    CPU_FP32          k_val;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH0_BASE_ADDR;
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH1_BASE_ADDR;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH2_BASE_ADDR;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH3_BASE_ADDR;
#endif
    
    periph_clk_freq = CSP_PM_PerClkFreqGet(0u);                 /* Get clk freq.                                        */

    DEF_BIT_SET(p_ser_reg->SCEN, TMPM369_SCEN_BIT_SIO_EN);      /* En the serial channel blk.                           */
    DEF_BIT_CLR(p_ser_reg->SCCR, DEF_INT_08_MASK);              /* Normal operation; 8-bits, no parity                  */       
                                                                /* 8 bit data sent & serial tranfer clk: baud rate gen. */ 
    DEF_BIT_SET(p_ser_reg->SCMOD0, (TMPM369_SCMOD0_BIT_SM_8_BIT | TMPM369_SCMOD0_BIT_SC_BAUD));
    
                                                                /* 1 stop bit & LSB sent first.                         */
    DEF_BIT_CLR(p_ser_reg->SCMOD2, (TMPM369_SCMOD2_BIT_DRCHG | TMPM369_SCMOD2_BIT_SBLEN));
    
    DEF_BIT_SET(p_ser_reg->SCMOD2,  TMPM369_SCMOD2_BIT_WBUF);   /* En double buff.                                      */
    DEF_BIT_SET(p_ser_reg->SCMOD1,  TMPM369_SCMOD1_BIT_TXE);    /* En the transmitter.                                  */
    DEF_BIT_SET(p_ser_reg->SCMOD0,  TMPM369_SCMOD0_BIT_RXE);    /* En the receiver.                                     */
    DEF_BIT_SET(p_ser_reg->SCBRCR,  TMPM369_SCBRCR_BIT_BRCK_T4);
    
    baud_div        = (TMPM369_SCBRCR_BIT_BRCK_T4 >> 4u);       /* Configuration for Baud Rate Generator.               */
    periph_clk_freq = (int)((periph_clk_freq / (pow(4u, baud_div) * 2u)) / 16u);
    k_val           = (float)periph_clk_freq / (float)baud_rate;
    n_val           = (int)k_val;
    k_val           = k_val - n_val;
    k_val           = 16u - (k_val * 16u);
    
    if((k_val - (int)k_val) > .5) {
        k_val++;
    } 
    
    p_ser_reg->SCBRADD = (int)k_val;
    DEF_BIT_SET(p_ser_reg->SCBRCR, (TMPM369_SCBRCR_BIT_BRS(n_val) | TMPM369_SCBRCR_BIT_BRADDE));

                                                                /* Clear the Rx & Tx and specify the fill lvl(s)        */
    DEF_BIT_SET(p_ser_reg->SCRFC, (TMPM369_SCxFC_BIT_xIL(2u) | TMPM369_SCRFC_BIT_RFCS));
    DEF_BIT_SET(p_ser_reg->SCTFC, (TMPM369_SCxFC_BIT_xIL(2u) | TMPM369_SCTFC_BIT_TFCS));
    
                                                                /* En int.                                              */
    DEF_BIT_SET(p_ser_reg->SCFCNF, (TMPM369_SCFCNF_BIT_TFIE | \
                                    TMPM369_SCFCNF_BIT_RFIE | \
                                    TMPM369_SCFCNF_BIT_CNFG));
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)            /* ------ CONFIGURE PORTx for UART COMMUNICATION ------ */
                                                                /* Set PortE function pin(s) 1 & 2 to RXD0 & TXD0.      */
                                                                /* Set PortE function pin 3 to CTS0 (CLK).              */
                                                                /* Input: RXD0, CTS0. Output: TXD0.                     */
    DEF_BIT_SET(BSP_PORTx_REG_PxFR1(PORT_E), (TMPM369_PORTE_PEFR1_PE1F1 | TMPM369_PORTE_PEFR1_PE2F1));
    DEF_BIT_SET(BSP_PORTx_REG_PxFR4(PORT_E),  TMPM369_PORTE_PEFR4_PE3F4);
    DEF_BIT_SET(BSP_PORTx_REG_PxIE(PORT_E),  (TMPM369_PORTE_PEIE_PE1IE  | TMPM369_PORTE_PEIE_PE3IE));
    DEF_BIT_SET(BSP_PORTx_REG_PxCR(PORT_E),   TMPM369_PORTE_PECR_PE2C);
    
                                                                /* Set ISR handler for UART interrupt(s).               */
    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTRX0,
                    (CPU_FNCT_PTR)ProbeRS232_RxISRHandler,
                    (void       *)0u);

    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTTX0,
                    (CPU_FNCT_PTR)ProbeRS232_TxISRHandler,
                    (void       *)0u);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)            /* ------ CONFIGURE PORTx for UART COMMUNICATION ------ */
                                                                /* Set PortE function pin(s) 6 & 5 to RXD0 & TXD0.      */
                                                                /* Set PortE function pin 4 to CTS0 (CLK).              */
                                                                /* Input: RXD0, CTS0. Output: TXD0.                     */
    DEF_BIT_SET(BSP_PORTx_REG_PxFR1(PORT_E), (TMPM369_PORTE_PEFR1_PE5F1 | TMPM369_PORTE_PEFR1_PE6F1));
    DEF_BIT_SET(BSP_PORTx_REG_PxFR4(PORT_E),  TMPM369_PORTE_PEFR4_PE4F4);
    DEF_BIT_SET(BSP_PORTx_REG_PxIE(PORT_E),  (TMPM369_PORTE_PEIE_PE4IE  | TMPM369_PORTE_PEIE_PE6IE));
    DEF_BIT_SET(BSP_PORTx_REG_PxCR(PORT_E),   TMPM369_PORTE_PECR_PE5C);
                                                                /* Set ISR handler for UART interrupt(s).               */
    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTRX1,
                    (CPU_FNCT_PTR)ProbeRS232_RxISRHandler,
                    (void       *)0u);

    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTTX1,
                    (CPU_FNCT_PTR)ProbeRS232_TxISRHandler,
                    (void       *)0u);
    
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)            /* ------ CONFIGURE PORTx for UART COMMUNICATION ------ */
                                                                /* Set PortL function pin(s) 1 & 2 to RXD0 & TXD0.      */
                                                                /* Set PortL function pin 3 to CTS0 (CLK).              */
                                                                /* Input: RXD0, CTS0. Output: TXD0.                     */
    DEF_BIT_SET(BSP_PORTx_REG_PxFR1(PORT_L), (TMPM369_PORTL_PLFR5_PL1F5 | TMPM369_PORTL_PLFR5_PL2F5));
    DEF_BIT_SET(BSP_PORTx_REG_PxFR4(PORT_L),  TMPM369_PORTL_PLFR6_PL3F6);
    DEF_BIT_SET(BSP_PORTx_REG_PxIE(PORT_L),  (TMPM369_PORTL_PLIE_PL1IE  | TMPM369_PORTL_PLIE_PL3IE));
    DEF_BIT_SET(BSP_PORTx_REG_PxCR(PORT_L),   TMPM369_PORTL_PLCR_PL2C);
                                                                /* Set ISR handler for UART interrupt(s).               */
    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTRX2,
                    (CPU_FNCT_PTR)ProbeRS232_RxISRHandler,
                    (void       *)0u);

    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTTX2,
                    (CPU_FNCT_PTR)ProbeRS232_TxISRHandler,
                    (void       *)0u);

#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)            /* ------ CONFIGURE PORTx for UART COMMUNICATION ------ */
                                                                /* Set PortB function pin(s) 1 & 2 to RXD0 & TXD0.      */
                                                                /* Set PortA function pin 3 to CTS0 (CLK).              */
                                                                /* Input: RXD0, CTS0. Output: TXD0.                     */
    DEF_BIT_SET(BSP_PORTx_REG_PxFR1(PORT_B), (TMPM369_PORTB_PBFR3_PB0F3 | TMPM369_PORTB_PBFR3_PB1F3));
    DEF_BIT_SET(BSP_PORTx_REG_PxFR4(PORT_A),  TMPM369_PORTA_PAFR4_PA7F4);
    DEF_BIT_SET(BSP_PORTx_REG_PxIE(PORT_B),   TMPM369_PORTB_PBIE_PB1IE);
    DEF_BIT_SET(BSP_PORTx_REG_PxIE(PORT_A),   TMPM369_PORTA_PAIE_PA7IE);
    DEF_BIT_SET(BSP_PORTx_REG_PxCR(PORT_B),   TMPM369_PORTB_PBCR_PB0C);
                                                                /* Set ISR handler for UART interrupt(s).               */
    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTRX3,
                    (CPU_FNCT_PTR)ProbeRS232_RxISRHandler,
                    (void       *)0u);

    CSP_IntVectReg ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,
                    (CSP_DEV_NBR )CSP_INT_SRC_NBR_INTTX3,
                    (CPU_FNCT_PTR)ProbeRS232_TxISRHandler,
                    (void       *)0u);

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
* Note(s)     : (1) This function may be empty if receive and transmit interrupts are handled in the
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
* Note(s)     : (1) This function may be empty if receive interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    TMPM369_SER_REG  *p_ser_reg;
    CPU_INT08U        uart_chr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH0_BASE_ADDR;
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH1_BASE_ADDR;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH2_BASE_ADDR;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH3_BASE_ADDR;
#endif

    if (DEF_BIT_IS_CLR(p_ser_reg->SCCR, TMPM369_SCCR_BIT_ERR_FLAG_ALL)) {
        uart_chr  = p_ser_reg->SCBUF; 
        ProbeRS232_RxHandler(uart_chr);
    }
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    CSP_IntClr((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Clr Rx int.                                          */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX0);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CSP_IntClr((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Clr Rx int.                                          */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntClr((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Clr Rx int.                                          */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX2);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntClr((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Clr Rx int.                                          */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX3);
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
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Rx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Rx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Rx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX2);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Rx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX3);
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
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Rx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Rx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Rx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX2);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Rx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTRX3);
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
* Note(s)     : (1) This function may be empty if transmit interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
    ProbeRS232_TxHandler();
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
    TMPM369_SER_REG  *p_ser_reg;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH0_BASE_ADDR;
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH1_BASE_ADDR;
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH2_BASE_ADDR;
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    p_ser_reg = (TMPM369_SER_REG *)TMPM369_SIO_CH3_BASE_ADDR;
#endif
    
    DEF_BIT_SET(p_ser_reg->SCMOD1, TMPM369_SCMOD1_BIT_TXE);

    p_ser_reg->SCBUF = c;
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
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Tx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Tx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Tx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX2);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntDis ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,             /* Disable the UART Tx int.                             */
                (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX2);
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
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Tx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX0);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Tx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX1);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Tx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX2);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    CSP_IntEn ((CSP_DEV_NBR)CSP_INT_CTRL_NBR_MAIN,              /* Enable the UART Tx int.                              */
               (CSP_DEV_NBR)CSP_INT_SRC_NBR_INTTX3);
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
