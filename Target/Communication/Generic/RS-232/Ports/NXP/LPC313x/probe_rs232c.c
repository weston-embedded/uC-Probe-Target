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
*                                            PORT FOR LPC313x
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
 
                                                                /* ------ INTERRUPT ENABLE REGISTER BIT DEFINES ----- */
#define  LPC313x_IER_ABTO                  DEF_BIT_09          /* Auto-baud Timeout Interrupt enable                 */
#define  LPC313x_IER_ABEO                  DEF_BIT_08          /* End of Auto-Baud Interrupt enable                  */
#define  LPC313x_IER_CTS                   DEF_BIT_07          /* Modem Status int in a cts signal tansition enable  */
#define  LPC313x_IER_MS                    DEF_BIT_03          /* Modem Status interrupt enable                      */
#define  LPC313x_IER_RLS                   DEF_BIT_02          /* Receiver Line Status Interrupt enable              */
#define  LPC313x_IER_THR                   DEF_BIT_01          /* Transmitter Holdinig Reg Empty interrupt enable    */
#define  LPC313x_IER_RDA                   DEF_BIT_00          /* Receive Data Available interrupt enable            */

                                                                /* -------- LATCH DIVISOR REGISTER BIT DEFINES ------ */
#define  LPC313x_DLL_MASK                  DEF_BIT_FIELD(8, 0)  /* DLL Mask                                           */
#define  LPC313x_DLM_MASK                  DEF_BIT_FIELD(8, 0)  /* DLM Mask                                           */


                                                                /* -  INTERRUPT IDENTIFICATION REGISTER BIT DEFINES - */
#define  LPC313x_IIR_ABTO                  DEF_BIT_09           /* Auto-baud Timeout Interrupt                        */
#define  LPC313x_IIR_ABEO                  DEF_BIT_08           /* End of Auto-Baud Interrupt                         */
#define  LPC313x_IIR_FIFO                  DEF_BIT_06           /* Copies of FCR [0]                                  */
#define  LPC313x_IIR_INT_ID_MASK           DEF_BIT_FIELD(3, 1)  /* Interrupt Identification Mask                      */
#define  LPC313x_IIR_INT_STATUS            DEF_BIT_00           /* Interrupt Sttus                                    */

#define  LPC313x_IIR_INT_ID_MODM                0x00            /* Modem Status                                       */
#define  LPC313x_IIR_INT_ID_THR                 0x01            /* Trasmitter Holding register Empty                  */
#define  LPC313x_IIR_INT_ID_RX_DATA             0x02            /* Receive Data Available                             */
#define  LPC313x_IIR_INT_ID_RX_LINE_STAT        0x03            /* Receiver Line status                               */
#define  LPC313x_IIR_INT_ID_CH_TO               0x06            /* Character timeout                                  */

                                                                /* -------- FIFO CONTROL REGISTER BIT DEFINES ------- */
#define  LPC313x_FCR_RX_TRIG_LEVEL_MASK    DEF_BIT_FIELD(2, 6)  /* Receiver Trigger Level Selection                   */
#define  LPC313x_FCR_RX_TRIG_LEVEL_00      DEF_BIT_NONE         /* Trigger point at character 1                       */
#define  LPC313x_FCR_RX_TRIG_LEVEL_16      DEF_BIT_MASK(1, 6)   /* Trigger point at character 16                      */
#define  LPC313x_FCR_RX_TRIG_LEVEL_24      DEF_BIT_MASK(2, 6)   /* Trigger point at character 24                      */
#define  LPC313x_FCR_RX_TRIG_LEVEL_28      DEF_BIT_MASK(3, 6)   /* Trigger point at character 28                      */
#define  LPC313x_FCR_DMA_MODE              DEF_BIT_03           /* DMA mode                                           */
#define  LPC313x_FCR_TX_FIFO_RST           DEF_BIT_02           /* TX Fifo Reset                                      */
#define  LPC313x_FCR_RX_FIFO_RST           DEF_BIT_01           /* RX Fifo Reset                                      */
#define  LPC313x_FCR_INT_NO_PEND           DEF_BIT_00           /* Interrupt Status                                   */

                                                                /* -------- LINE CONTROL REGISTER BIT DEFINES ------- */
#define  LPC313x_LCR_DLAB                  DEF_BIT_07           /* Divisor Latch Access bit                           */
#define  LPC313x_LCR_BRK_CTRL              DEF_BIT_06           /* Break Control bit                                  */
#define  LPC313x_LCR_PAR_STICK             DEF_BIT_05           /* Stick parity mode                                  */
#define  LPC313x_LCR_PAR_EVEN              DEF_BIT_04           /* Parity even                                        */
#define  LPC313x_LCR_PAR_EN                DEF_BIT_03           /* Parity enable                                      */
#define  LPC313x_LCR_STOP_BIT_NBR          DEF_BIT_02           /* Stop bit number                                    */
#define  LPC313x_LCR_WD_LEN_SEL_MASK       DEF_BIT_FIELD(2, 1)  /* Word Length selector mask                          */
#define  LPC313x_LCR_WD_LEN_SEL_5          DEF_BIT_NONE         /* Word Length selector  5 bits                       */
#define  LPC313x_LCR_WD_LEN_SEL_6          DEF_BIT_MASK(1, 0)   /* Word Length selector  6 bits                       */
#define  LPC313x_LCR_WD_LEN_SEL_7          DEF_BIT_MASK(2, 0)   /* Word Length selector  7 bits                       */
#define  LPC313x_LCR_WD_LEN_SEL_8          DEF_BIT_MASK(3, 0)   /* Word Length selector  8 bits                       */

                                                                /* ------- MODEM CONTROL REGISTER BIT DEFINES ------- */
#define  LPC313x_MCR_AUTO_CTS              DEF_BIT_07           /* Auto-cts flow control enable                       */
#define  LPC313x_MCR_AUTO_RTS              DEF_BIT_06           /* Auto-rts flow control enable                       */
#define  LPC313x_MCR_LOOP                  DEF_BIT_04           /* Loop-back mode enable                              */
#define  LPC313x_MCR_OUT2                  DEF_BIT_03           /* Inverse control for out2_n output                  */
#define  LPC313x_MCR_OUT1                  DEF_BIT_02           /* Inverse control for out1_n output                  */
#define  LPC313x_MCR_RTS                   DEF_BIT_01           /* Request to send                                    */
#define  LPC313x_MCR_DTR                   DEF_BIT_00           /* Inverse control for the data terminal ready        */

                                                                /* -------- LINE STATUS REGISTER BIT DEFINES -------- */
#define  LPC313x_LSR_RX_ERR                DEF_BIT_07           /* Error in receiver FIFO                             */
#define  LPC313x_LSR_TEMT                  DEF_BIT_06           /* Transmitter empty                                  */
#define  LPC313x_LSR_THRE                  DEF_BIT_05           /* Transmiter Holding Register empty                  */
#define  LPC313x_LSR_BI                    DEF_BIT_04           /* Break indication                                   */
#define  LPC313x_LSR_FE                    DEF_BIT_03           /* Framing error logic                                */
#define  LPC313x_LSR_PE                    DEF_BIT_02           /* Parity Error                                       */
#define  LPC313x_LSR_OE                    DEF_BIT_01           /* Overrun  error                                     */
#define  LPC313x_LSR_DTR                   DEF_BIT_00           /* Data readly.                                       */

#define  LPC313x_LSR_ERR_FLAGS            (LPC313x_LSR_FE  | \
                                           LPC313x_LSR_OE  | \
                                           LPC313x_LSR_PE)
                                               
#define  LPC313x_LSR_BI                    DEF_BIT_04           /* Break indication                                   */
#define  LPC313x_LSR_FE                    DEF_BIT_03           /* Framing error logic                                */
#define  LPC313x_LSR_PE                    DEF_BIT_02           /* Parity Error                                       */

                                                                /* -------- MODEM STATUS REGISTER BIT DEFINES ------- */
#define  LPC313x_MSR_DCD                   DEF_BIT_07           /* Data carrier detect                                */
#define  LPC313x_MSR_RI                    DEF_BIT_06           /* Ring indicator                                     */
#define  LPC313x_MSR_DSR                   DEF_BIT_05           /* Data set ready                                     */
#define  LPC313x_MSR_CTS                   DEF_BIT_04           /* Clear To Send CTS                                  */
#define  LPC313x_MSR_DDCD                  DEF_BIT_03           /* Delta Data Carrier Detect                          */
#define  LPC313x_MSR_TERI                  DEF_BIT_02           /* Parity Error                                       */
#define  LPC313x_MSR_DDSR                  DEF_BIT_01           /* Delta Data Set ready                               */
#define  LPC313x_MSR_DCTS                  DEF_BIT_01           /* Delta clear To Send                                */

                                                                /* -------- MODEM STATUS REGISTER BIT DEFINES ------- */
#define  LPC313x_ACR_ABTO                  DEF_BIT_07           /* Auto-baud timeoout interrupt clear                 */
#define  LPC313x_ACR_ABEO                  DEF_BIT_06           /* End of Auto-baud interrupt clear                   */

                                                                /* ----------- NHP POP REGISTER BIT DEFINES --------- */
#define  LPC313x_NHP_POOP_RBR              DEF_BIT_00           /* Pop the firts item forn the Rx buffer              */

                                                                /* ------- MODE SELECTION REGISTER BIT DEFINES ------ */
#define  LPC313x_MODE_NHP                  DEF_BIT_00           /* NHP mode Enable                                    */

                                                                /* ------- CONFIGURATION REGISTER BIT DEFINES ------- */
#define  LPC313x_CFG_HAS_IRDA              DEF_BIT_12           /* IRDA module included                               */
#define  LPC313x_CFG_HAS_LEVEL             DEF_BIT_09           /* Fifo Level interface  included                     */
#define  LPC313x_CFG_HAS_DMA               DEF_BIT_08           /* ARM DMA interface  included                        */

#define  LPC313x_CFG_MODEM_MASK            DEF_BIT_FIELD(2, 4)  /* Modem Interface mask                               */
#define  LPC313x_CFG_MODEM_NONE            DEF_BIT_NONE         /* Modem Interface not included                       */
#define  LPC313x_CFG_MODEM_CTS_RTS         DEF_BIT_04           /* CTS and RTS modem interface included               */
#define  LPC313x_CFG_MODEM_FULL            DEF_BIT_05           /* All modem interface signals included               */
#define  LPC313x_CFG_TYPE_MASK             DEF_BIT_FIELD(2, 0)  /* UART type Mask                                     */
#define  LPC313x_CFG_TYPE_450              DEF_BIT_NONE         /* UART type '450                                     */
#define  LPC313x_CFG_TYPE_550              DEF_BIT_MASK(1, 0)   /* UART type '550                                     */
#define  LPC313x_CFG_TYPE_650              DEF_BIT_MASK(2, 0)   /* UART type '650                                     */
#define  LPC313x_CFG_TYPE_750              DEF_BIT_MASK(3, 0)   /* UART type '750                                     */

#define  LPC313x_CFG_CTS                   DEF_BIT_04           /* Clear To Send CTS                                  */
#define  LPC313x_CFG_DDCD                  DEF_BIT_03           /* Delta Data Carrier Detect                          */
#define  LPC313x_CFG_TERI                  DEF_BIT_02           /* Parity Error                                       */
#define  LPC313x_CFG_DDSR                  DEF_BIT_01           /* Delta Data Set ready                               */
#define  LPC313x_CFG_DCTS                  DEF_BIT_01           /* Delta clear To Send                                */

                                                                /* ------------ UART INTERRUPT BIT DEFINES ---------- */
#define  LPC313x_INT_OE                    DEF_BIT_15           /* Overrun interrupt                                  */
#define  LPC313x_INT_PE                    DEF_BIT_14           /* Parity error interrupt                             */
#define  LPC313x_INT_FE                    DEF_BIT_13           /* Frame  error interrupt                             */
#define  LPC313x_INT_BI                    DEF_BIT_12           /* Break indication interupt                          */
#define  LPC313x_INT_ABTO                  DEF_BIT_09           /* Auto-Baud timeout                                  */
#define  LPC313x_INT_ABEO                  DEF_BIT_08           /* End of Auto-Baud                                   */
#define  LPC313x_INT_RXDA                  DEF_BIT_06           /* Receiver Data Available                            */
#define  LPC313x_INT_THRE                  DEF_BIT_04           /* Transmitter Holding Register interrupt             */
#define  LPC313x_INT_DDCD                  DEF_BIT_03           /* Delta Data Carrier Detect interrupt                */
#define  LPC313x_INT_TER                   DEF_BIT_02           /* Trailing Edge ring indicator interrupt             */
#define  LPC313x_INT_DDSR                  DEF_BIT_01           /* Delta Data set ready                               */
#define  LPC313x_INT_DDTS                  DEF_BIT_00           /* Delta clear to send interrupt enable               */

                                                                /* ----------- UART BASE ADDRESS DEFINES ------------ */
#define  LPC313x_REG_BASE_ADDR             0x15001000
#define  LPC313x_REG_BASE                 ((LPC313x_UART_REG_PTR)(LPC313x_REG_BASE_ADDR))

/*
*********************************************************************************************************
*                                            LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef  struct lpc313x_uart_reg {
    CPU_REG32  DLL;                                             /* R/W 0x000 Divisor Latch LSB                        */
    CPU_REG32  DLM;                                             /* R/W 0x004 Divisor Latch MSB                        */
    CPU_REG32  IIR;                                             /* R 0x008 Interrupt Identification Register          */
    CPU_REG32  LCR;                                             /* R/W 0x00C Line Control Register                    */
    CPU_REG32  MCR;                                             /* R/W 0x010 Modem Control Register                   */
    CPU_REG32  LSR;                                             /* R 0x014 Line Status Register                       */
    CPU_REG32  MSR;                                             /* R 0x018 Modem status Register                      */
    CPU_REG32  SCR;                                             /* R/W 0x01C Scratch Register                         */
    CPU_REG32  ACR;                                             /* R/W 0x020 Auto-baud Control Register               */
    CPU_REG32  ICR;                                             /* R/W 0x024 IrDA Control Register                    */
    CPU_REG32  FDR;                                             /* R/W 0x028 Fractional Divider Register              */
    CPU_REG32  RESERVED0;                                       /* - 0x02C Reserved                                   */
    CPU_REG32  POP;                                             /* W 0x030 NHP Pop Register                           */
    CPU_REG32  MODE;                                            /* R/W 0x034 NHP Mode Selection Register              */
    CPU_REG32  RESERVED1[999];                                  /*- 0x038-0xFD0 Reserved:                             */
    CPU_REG32  CFG;                                             /* R 0xFD4 Configuration Register                     */
    CPU_REG32  INTCE;                                           /* W 0xFD8 Interrupt Clear Enable Register            */
    CPU_REG32  INTSE;                                           /* W 0xFDC Interrupt Set Enable Register              */
    CPU_REG32  INTS;                                            /* R 0xFE0 Interrupt Status Register                  */
    CPU_REG32  INTE;                                            /* R 0xFE4 Interrupt Enable Register                  */
    CPU_REG32  INTCS;                                           /* W 0xFE8 Interrupt Clear Status Register            */
    CPU_REG32  INTSS;                                           /* W 0xFEC Interrupt Set Status Register              */
}  LPC313x_UART_REG, *LPC313x_UART_REG_PTR;


/*
*********************************************************************************************************
*                                           LOCAL MACROS
*********************************************************************************************************
*/

#define    LPC313x_DLAB_SET()                   {    DEF_BIT_SET(LPC313x_REG_BASE->LCR, LPC313x_LCR_DLAB);          }
#define    LPC313x_DLAB_CLR()                   {    DEF_BIT_CLR(LPC313x_REG_BASE->LCR, LPC313x_LCR_DLAB);          }

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

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
    BSP_CGU_CLK_SPREAD_CFG   uart_clk_cfg;
    CPU_INT32U               uart_clk_freq;
    CPU_INT16U               uart_clk_div;
    BSP_ERR                  err;
    
    
    BSP_CGU_RstSignalSet(BSP_CGU_RST_SIGNAL_NBR_UART_SYS_RST_AN_SOFT,
                         &err);
    
    BSP_CGU_RstSignalClr(BSP_CGU_RST_SIGNAL_NBR_UART_SYS_RST_AN_SOFT,
                         &err);
  
                                                                /* -------------- ENABLE UART CLOCKS ---------------- */
                                                                /* Enable UART_APB_CLK in the CGU module              */
    uart_clk_cfg.Nbr    = BSP_CGU_CLK_SPREAD_NBR_AHB_APB2_UART_APB_CLK;
    uart_clk_cfg.PwrOpt = BSP_CGU_CLK_SPREAD_PWR_OPT_RUN_EN
                        | BSP_CGU_CLK_SPREAD_PWR_OPT_AUTO_EN;       
    uart_clk_cfg.Opt    = BSP_CGU_CLK_SPREAD_CFG_OPT_PWR_SET;

    BSP_CGU_ClkSpreadCfgSet(&uart_clk_cfg,
                            &err);
     
    if (err != BSP_ERR_NONE) {
        return;
    }
                                                              /* Enable UART_U_CLK in the CGU module                 */
    uart_clk_cfg.Nbr    = BSP_CGU_CLK_SPREAD_NBR_UART_UART_U_CLK;

    BSP_CGU_ClkSpreadCfgSet(&uart_clk_cfg,
                            &err);

    if (err != BSP_ERR_NONE) {
        return;
    }
    
                                                                /* Get the UART_U_CLK frequency                       */    
    uart_clk_freq = BSP_CGU_ClkSpreadFreqGet(BSP_CGU_CLK_SPREAD_NBR_UART_UART_U_CLK,
                                             &err);
    if (err != BSP_ERR_NONE) {
        return;
    }
    
    if (uart_clk_freq < baud_rate) {
       return;
    }
                                                                /* Calculate the baud reate divisor                   */
    uart_clk_div = (CPU_INT16U)(CPU_INT32U)((uart_clk_freq / (16 * baud_rate)));

                                                                /* --------------- UART CONFIGURATION  -------------- */    
    LPC313x_DLAB_CLR();                                         
    LPC313x_REG_BASE->DLM  = DEF_BIT_NONE;                      /* Diable all interrupts                              */
    LPC313x_REG_BASE->MCR  = DEF_BIT_NONE;                      /* Reset the modem control register                   */
    LPC313x_REG_BASE->MODE = DEF_BIT_NONE;                      /* Uart traditional mode                              */
    
    LPC313x_DLAB_SET();                                         
                                                                /* Set the divisor Latch registers                    */
    LPC313x_REG_BASE->FDR  = DEF_BIT_NONE;                      /* Disable the fractional dividers                    */
    LPC313x_REG_BASE->DLL  = (uart_clk_div >> 0) & LPC313x_DLL_MASK;  
    LPC313x_REG_BASE->DLM  = (uart_clk_div >> 8) & LPC313x_DLM_MASK;                     
        
    LPC313x_REG_BASE->LCR  = LPC313x_LCR_WD_LEN_SEL_8;          /* No parity, 8 bits, 1 stop bit                      */
                               
    LPC313x_REG_BASE->IIR  = LPC313x_FCR_TX_FIFO_RST
                           | LPC313x_FCR_RX_FIFO_RST
                           | LPC313x_FCR_RX_TRIG_LEVEL_00
                           | LPC313x_FCR_INT_NO_PEND;
    
    
    BSP_IntVectSet(BSP_INT_TYPE_NBR_IRQ,
                   BSP_INT_SRC_NBR_UART_INTREQ,
                   BSP_INT_PRIO_LOWEST,
                   BSP_INT_POL_HIGH,
                   ProbeRS232_RxTxISRHandler,
                   &err);
    if (err != BSP_ERR_NONE) {
        return;
    }
 
    BSP_IntEn(BSP_INT_SRC_NBR_UART_INTREQ,
              &err);
 
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
    CPU_INT32U  rx_data;
    CPU_INT32U  int_status;
    CPU_INT08U  int_id;
    
    
    
    int_status = (LPC313x_REG_BASE->IIR);
    int_id     = ((int_status >> 1) & 0x03);
        
    switch (int_id) {
        case LPC313x_IIR_INT_ID_THR:
             ProbeRS232_TxHandler();
             break;
             
        case LPC313x_IIR_INT_ID_RX_DATA:
             rx_data = LPC313x_REG_BASE->DLL;
             ProbeRS232_RxHandler(rx_data);
             break;             
    }        
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
    DEF_BIT_CLR(LPC313x_REG_BASE->DLM, LPC313x_IER_RDA);
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
    DEF_BIT_SET(LPC313x_REG_BASE->DLM, LPC313x_IER_RDA);
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
    LPC313x_REG_BASE->DLL = c;
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
    DEF_BIT_CLR(LPC313x_REG_BASE->DLM, LPC313x_IER_THR);
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
    DEF_BIT_SET(LPC313x_REG_BASE->DLM, LPC313x_IER_THR);
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
