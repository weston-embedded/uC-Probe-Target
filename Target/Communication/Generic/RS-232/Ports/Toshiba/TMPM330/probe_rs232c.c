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
*                                       PORT FOR TOSHIBA TMPM330
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

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                       */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* --------- SERIAL CHANNEL (SIO) BASE ADDRESS ------ */
#define  TMPM330_SIO_CH0_BASE_ADDR             0x40020080
#define  TMPM330_SIO_CH1_BASE_ADDR             0x400200C0
#define  TMPM330_SIO_CH2_BASE_ADDR             0x40020100

                                                                /* ------- SERIAL ENABLE REGISTER BIT DEFINES ------- */
#define  TMPM330_SCEN_SIO_EN                   DEF_BIT_00

                                                                /* ----------- CONTROL REGISTER BIT DEFINES --------- */ 
#define  TMPM330_SCCR_ERR_FLAG_OVER            DEF_BIT_04       /* Overrun error                                      */
#define  TMPM330_SCCR_ERR_FLAG_PARITY          DEF_BIT_03       /* Parity/Underrun error                              */
#define  TMPM330_SCCR_ERR_FLAG_FRAMING         DEF_BIT_02       /* Framing error                                      */

#define  TMPM330_SCCR_ERR_FLAG_ALL            (TMPM330_SCCR_ERR_FLAG_OVER    | \
                                               TMPM330_SCCR_ERR_FLAG_PARITY  | \
                                               TMPM330_SCCR_ERR_FLAG_FRAMING)

                                                                /* ------- MODE CONTROL REGISTERS BIT DEFINES ------- */
                                                                
#define  TMPM330_SCMOD2_RST_0                         0x2       /* Overwriting "01" in place place of "10" will ...   */
#define  TMPM330_SCMOD2_RST_1                         0x1       /* ... generate a software reset                      */
 
#define  TMPM330_SCMOD1_TX_EN                   DEF_BIT_04      /* Transmit Enable                                    */

#define  TMPM330_SCMOD1_TX_MODE_HALF_RX        (0x01 << 5)      /* Transfer mode Half-Duplex (RX)                     */
#define  TMPM330_SCMOD1_TX_MODE_HALF_TX        (0x02 << 5)      /* Transfer mode Half-Duplex (TX)                     */
#define  TMPM330_SCMOD1_TX_MODE_FULL           (0x03 << 5)      /* Transfer mode Full-Duplex                          */

#define  TMPM330_SCMOD0_MODE_IO                (0x00 << 2)      /* Serial Interface Mode                              */
#define  TMPM330_SCMOD0_MODE_7_BIT             (0x01 << 2)      /* 7-bit length mode                                  */
#define  TMPM330_SCMOD0_MODE_8_BIT             (0x02 << 2)      /* 8-bit length mode                                  */
#define  TMPM330_SCMOD0_MODE_9_BIT             (0x03 << 2)      /* 8-bit length mode                                  */

#define  TMPM330_SCMOD0_RXE                     DEF_BIT_05      /* Reception Enable                                   */

#define  TMPM330_SCMOD0_SC_TMR7                (0x00 << 0)
#define  TMPM330_SCMOD0_SC_BAUD_RATE           (0x01 << 0)
#define  TMPM330_SCMOD0_SC_INT_CLK             (0x02 << 0)
#define  TMPM330_SCMOD0_SC_EXT_CLK             (0x03 << 0)
                                                                /* -BAUD RATE GENERATOR CONTROL REGISTER BIT DEFINES- */
#define  TMP330_SCBRCR_T1                      (0x00 << 4)      /* Input Clock T1                                     */
#define  TMP330_SCBRCR_T4                      (0x00 << 4)      /* Input Clock T4                                     */
#define  TMP330_SCBRCR_T16                     (0x00 << 4)      /* Input Clock T16                                    */
#define  TMP330_SCBRCR_T64                     (0x00 << 4)      /* Input Clock T64                                    */

                                                                /* ------------------ UART I/O DEFINES -------------- */

#define  TMPM330_UART0_TX                      DEF_BIT_00
#define  TMPM330_UART0_RX                      DEF_BIT_01

#define  TMPM330_UART1_TX                      DEF_BIT_04
#define  TMPM330_UART1_RX                      DEF_BIT_05

#define  TMPM330_UART2_TX                      DEF_BIT_00
#define  TMPM330_UART2_RX                      DEF_BIT_01


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
                                                                /* --------- SERIAL CHANNELs REGISTER DEFINES ------- */
typedef struct tmpm330_ser_reg {
    volatile  CPU_INT32U  SCEN;                                 /* Enable register                                    */
    volatile  CPU_INT32U  SCBUF;                                /* Buffer register                                    */
    volatile  CPU_INT32U  SCCR;                                 /* Control register                                   */
    volatile  CPU_INT32U  SCMOD0;                               /* Mode control register 0                            */
    volatile  CPU_INT32U  SCBRCR;                               /* Baud Rate control register                         */
    volatile  CPU_INT32U  SCBRADD;                              /* Baud Rate control register 2                       */
    volatile  CPU_INT32U  SCMOD1;                               /* Mode control register 1                            */
    volatile  CPU_INT32U  SCMOD2;                               /* Mode control register 2                            */
    volatile  CPU_INT32U  SCRFC;                                /* Rx FIFO configuration register                     */
    volatile  CPU_INT32U  SCTFC;                                /* Tx FIFO configuration register                     */
    volatile  CPU_INT32U  SCRST;                                /* Rx FIFO status register                            */
    volatile  CPU_INT32U  SCTST;                                /* Tx FIFO status register                            */
    volatile  CPU_INT32U  SCFCNF;                               /* FIFO configuration register                        */    
} TMPM330_SER_REG;


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
    TMPM330_SER_REG  *p_ser_reg;
    CPU_INT32U        periph_clk_freq;
    CPU_INT08U        div_ratio;

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH0_BASE_ADDR;
#endif  
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  
       
    periph_clk_freq   = BSP_ClkPeriphFreqGet();
    div_ratio         = (CPU_INT08U)(CPU_INT32U)(((periph_clk_freq  + 16 * baud_rate) / (32 * baud_rate)))  ;
    
    p_ser_reg->SCEN   = TMPM330_SCEN_SIO_EN;                    /* Enable the serial channel block                    */
  
    p_ser_reg->SCMOD2 = TMPM330_SCMOD2_RST_0;                   /* Generate a software reset                          */
    p_ser_reg->SCMOD2 = TMPM330_SCMOD2_RST_1;

    p_ser_reg->SCCR   = DEF_BIT_NONE;                           /* Normal operation 8 bits, no parity                 */
    p_ser_reg->SCMOD1 = TMPM330_SCMOD1_TX_MODE_FULL;            /* Transfer mode Full-Duplex                          */
    p_ser_reg->SCMOD2 = DEF_BIT_NONE;
   
    p_ser_reg->SCBRCR = div_ratio                               /* Division ratio "N"                                 */
                      | TMP330_SCBRCR_T1;                       /* Input clock T1 (Fperiph / 2)                       */
   
    p_ser_reg->SCMOD0 = TMPM330_SCMOD0_MODE_8_BIT               /* 8-bit length mode                                  */
                      | TMPM330_SCMOD0_RXE                      /* Reception Enable                                   */
                      | TMPM330_SCMOD0_SC_BAUD_RATE;            /* Serial transfer clock : Baud Rate Generator        */

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)       
    BSP_GPIO_Cfg( BSP_GPIO_PORT_E,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_OUT_EN),
                  TMPM330_UART0_TX);
    
    BSP_GPIO_Cfg( BSP_GPIO_PORT_E,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_IN_EN),
                  TMPM330_UART0_RX);       

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_RX0, 
                   (CPU_FNCT_VOID)ProbeRS232_RxISRHandler);

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_TX0, 
                   (CPU_FNCT_VOID)ProbeRS232_TxISRHandler);
    
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
  BSP_GPIO_Cfg( BSP_GPIO_PORT_E,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_OUT_EN),
                  TMPM330_UART1_TX);
    
    BSP_GPIO_Cfg( BSP_GPIO_PORT_E,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_IN_EN),
                  TMPM330_UART1_RX);

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_RX1, 
                   (CPU_FNCT_VOID)ProbeRS232_RxISRHandler);

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_TX1, 
                   (CPU_FNCT_VOID)ProbeRS232_TxISRHandler);
    
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
    BSP_GPIO_Cfg( BSP_GPIO_PORT_F,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_OUT_EN),
                  TMPM330_UART2_TX);
    
    BSP_GPIO_Cfg( BSP_GPIO_PORT_F,
                 (BSP_GPIO_OPT_FNCT1_EN | BSP_GPIO_OPT_IN_EN),
                  TMPM330_UART2_RX);

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_RX2, 
                   (CPU_FNCT_VOID)ProbeRS232_RxISRHandler);

    BSP_IntVectSet((CPU_DATA     )BSP_INT_ID_TX2, 
                   (CPU_FNCT_VOID)ProbeRS232_TxISRHandler);

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
    TMPM330_SER_REG  *p_ser_reg;
    CPU_INT08U        uart_chr;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH0_BASE_ADDR;
#endif  
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  

    if (DEF_BIT_IS_CLR(p_ser_reg->SCCR, TMPM330_SCCR_ERR_FLAG_ALL)) {
        uart_chr  = p_ser_reg->SCBUF; 
        ProbeRS232_RxHandler(uart_chr);
    }

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)   
    BSP_IntClr(BSP_INT_ID_RX0);
#endif  
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)   
    BSP_IntClr(BSP_INT_ID_RX1);
#endif  

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)   
    BSP_IntClr(BSP_INT_ID_RX2);
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
    BSP_IntDis(BSP_INT_ID_RX0);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
    BSP_IntDis(BSP_INT_ID_RX1);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)       
    BSP_IntDis(BSP_INT_ID_RX2);
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
    BSP_IntEn(BSP_INT_ID_RX0);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
    BSP_IntEn(BSP_INT_ID_RX1);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)       
    BSP_IntEn(BSP_INT_ID_RX2);
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
    TMPM330_SER_REG  *p_ser_reg;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH0_BASE_ADDR;
#endif  
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)   
    p_ser_reg = (TMPM330_SER_REG *)TMPM330_SIO_CH2_BASE_ADDR;
#endif  
    
    DEF_BIT_SET(p_ser_reg->SCMOD1, TMPM330_SCMOD1_TX_EN);

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
    BSP_IntDis(BSP_INT_ID_TX0);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
    BSP_IntDis(BSP_INT_ID_TX1);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)       
    BSP_IntDis(BSP_INT_ID_TX2);
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
    BSP_IntEn(BSP_INT_ID_TX0);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)       
    BSP_IntEn(BSP_INT_ID_TX1);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)       
    BSP_IntEn(BSP_INT_ID_TX2);
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
