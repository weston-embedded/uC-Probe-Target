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
*                                   PORT FOR THE ATMEL AT91SAM3U
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

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                     */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

                                                                /* -------- AT91SAM USART BASE ADDRESS DEFINES ----- */
#define  AT91SAM_REG_US0_BASE_ADDR               ((CPU_INT32U)0x40090000)
#define  AT91SAM_REG_US1_BASE_ADDR               ((CPU_INT32U)0x40094000)
#define  AT91SAM_REG_US2_BASE_ADDR               ((CPU_INT32U)0x40098000)
#define  AT91SAM_REG_US3_BASE_ADDR               ((CPU_INT32U)0x4009C000)
#define  AT91SAM_REG_DBG_BASE_ADDR               ((CPU_INT32U)0x400E0600)

                                                                /* ---------- US_IER REGISTER BIT DEFINES ---------- */
#define  AT91SAM_REG_US_RXRDY                     DEF_BIT_00
#define  AT91SAM_REG_US_TXRDY                     DEF_BIT_01
#define  AT91SAM_REG_US_ENDRX                     DEF_BIT_03
#define  AT91SAM_REG_US_ENDTX                     DEF_BIT_04
#define  AT91SAM_REG_US_OVRE                      DEF_BIT_05
#define  AT91SAM_REG_US_FRAME                     DEF_BIT_06
#define  AT91SAM_REG_US_PARE                      DEF_BIT_07
#define  AT91SAM_REG_US_TXEMPTY                   DEF_BIT_09
#define  AT91SAM_REG_US_TXBUFE                    DEF_BIT_11
#define  AT91SAM_REG_US_RXBUFF                    DEF_BIT_12
#define  AT91SAM_REG_US_COMM_TX                   DEF_BIT_30
#define  AT91SAM_REG_US_COMM_RX                   DEF_BIT_31

                                                                /* ---------- US_CR REGISTER BIT DEFINES ----------- */
#define  AT91SAM_REG_US_RSTRX                     DEF_BIT_02
#define  AT91SAM_REG_US_RSTTX                     DEF_BIT_03
#define  AT91SAM_REG_US_RXEN                      DEF_BIT_04
#define  AT91SAM_REG_US_RXDIS                     DEF_BIT_05
#define  AT91SAM_REG_US_TXEN                      DEF_BIT_06
#define  AT91SAM_REG_US_TXDIS                     DEF_BIT_07
#define  AT91SAM_REG_US_RSTSTA                    DEF_BIT_08

                                                                /* -------------- US_MR REGISTER BIT DEFINES ------- */
#define  AT91SAM_REG_US_USMODE_NORMAL             (0x00 <<  0)
#define  AT91SAM_REG_US_CLKS_CLOCK                (0x00 <<  4)
#define  AT91SAM_REG_US_CHRL_8_BITS               (0x03 <<  6)
#define  AT91SAM_REG_US_PAR_NONE                  (0x04 <<  9)
#define  AT91SAM_REG_US_NBSTOP_1_BIT              (0x00 << 12)

#define  AT91SAM_GPIOA_US0_PINS                   (DEF_BIT_18 | DEF_BIT_19)
#define  AT91SAM_GPIOA_US1_PINS                   (DEF_BIT_20 | DEF_BIT_21)
#define  AT91SAM_GPIOA_US2_PINS                   (DEF_BIT_22 | DEF_BIT_23)
#define  AT91SAM_GPIOC_US3_PINS                   (DEF_BIT_12 | DEF_BIT_13)
#define  AT91SAM_GPIOA_DBG_PINS                   (DEF_BIT_11 | DEF_BIT_12)


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct at91sam_usart_reg {
    CPU_REG32	 US_CR; 	                                /* Control Register                                   */
    CPU_REG32	 US_MR;                                         /* Mode Register                                      */
    CPU_REG32	 US_IER;                                        /* Interrupt Enable Register                          */
    CPU_REG32	 US_IDR;                                        /* Interrupt Disable Register                         */
    CPU_REG32	 US_IMR;                                        /* Output Disable Registerr                           */
    CPU_REG32	 US_CSR;                                        /* Output Status Register                             */
    CPU_REG32	 US_RHR;                                        /* Receiver Holding Register                          */
    CPU_REG32	 US_THR;                                        /* Transmit Holding Register                          */
    CPU_REG32	 US_BRGR;                                       /* Baud Rate Generator Register                       */
}  AT91SAM_USART_REG;

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_CFG_COMM_SEL

  #error  "PROBE_RS232_CFG_COMM_SEL          not #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0   ]   "
  #error  "                                  [     || PROBE_RS232_UART_1   ]   "
  #error  "                                  [     || PROBE_RS232_UART_2   ]   "
  #error  "                                  [     || PROBE_RS232_UART_3   ]   "
  #error  "                                  [     || PROBE_RS232_UART_DBG ]   "

#elif     (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_0  ) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_1  ) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_2  ) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_3  ) && \
          (PROBE_RS232_CFG_COMM_SEL != PROBE_RS232_UART_DBG)

  #error  "PROBE_RS232_CFG_COMM_SEL    illegally #define'd in 'probe_com_cfg.h'"
  #error  "                                  [MUST be PROBE_RS232_UART_0   ]   "
  #error  "                                  [     || PROBE_RS232_UART_1   ]   "
  #error  "                                  [     || PROBE_RS232_UART_2   ]   "
  #error  "                                  [     || PROBE_RS232_UART_3   ]   "
  #error  "                                  [     || PROBE_RS232_UART_DBG ]   "
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
* Note(s)     : (1) The DBGU shares the system interrupt with several other peripherals (including the
*                   Periodic Interval Timer (PIT)).  Consequently, the interrupt for the DBGU is not
*                   initialized here; rather, it is assumed to be initialized in user code.  Note also
*                   that the interrupt is not cleared for the DBGU in OSView_RxTxISRHandler().  This should
*                   be done in user code as well.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT32U          mclk_freq;
    AT91SAM_USART_REG  *p_usart_reg;
    CPU_INT08U          per_id;
    

    mclk_freq = BSP_PMC_MclkFreqGet();                          /* Calculate MCLK frequency                           */

                                                                /* Initialize the GPIO, Clks and Interrupts           */
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
     per_id      =  BSP_PER_ID_UART;

     BSP_GPIO_Cfg(BSP_GPIO_PORT_A,
                  AT91SAM_GPIOA_DBG_PINS,            
                  BSP_GPIO_OPT_PER_SEL_A); 
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
     per_id      =  BSP_PER_ID_US0;

     BSP_GPIO_Cfg(BSP_GPIO_PORT_A,
                  AT91SAM_GPIOA_US0_PINS,            
                  BSP_GPIO_OPT_PER_SEL_A); 
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
     per_id      =  BSP_PER_ID_US1;

     BSP_GPIO_Cfg(BSP_GPIO_PORT_A,
                  AT91SAM_GPIOA_US1_PINS,            
                  BSP_GPIO_OPT_PER_SEL_A); 
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
     per_id      =  BSP_PER_ID_US2;

     BSP_GPIO_Cfg(BSP_GPIO_PORT_A,
                  AT91SAM_GPIOA_US2_PINS,            
                  BSP_GPIO_OPT_PER_SEL_A); 
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
     per_id      =  BSP_PER_ID_US3;

     BSP_GPIO_Cfg(BSP_GPIO_PORT_C,
                  AT91SAM_GPIOC_US3_PINS,            
                  BSP_GPIO_OPT_PER_SEL_B); 
#endif
     
                                                                /* ------------------ SETUP USART ------------------- */
    p_usart_reg->US_IDR  = AT91SAM_REG_US_RXRDY                 /* Disable Rx & Tx interrupts                         */
                         | AT91SAM_REG_US_TXRDY;    
                                                                
    p_usart_reg->US_CR   = AT91SAM_REG_US_RXEN                  /* Enable the receiver & transmitter                  */
                         | AT91SAM_REG_US_TXEN;    

    p_usart_reg->US_MR   = AT91SAM_REG_US_USMODE_NORMAL         /* RS232C mode selected                               */
                         | AT91SAM_REG_US_CLKS_CLOCK            /* USART input CLK is MCK                             */
                         | AT91SAM_REG_US_CHRL_8_BITS           /* 8 bit data to be sent                              */
                         | AT91SAM_REG_US_PAR_NONE              /* No parity bit selected                             */
                         | AT91SAM_REG_US_NBSTOP_1_BIT;         /* 1 stop bit selected                                */

                                                                /* Set the USART baud rate                            */
    p_usart_reg->US_BRGR = (CPU_INT16U)((mclk_freq) / baud_rate / 16);
    
    BSP_IntEn(per_id);
    BSP_IntVectSet(per_id,  ProbeRS232_RxTxISRHandler);

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
* Note(s)     : (1) If DBGU is used, then this function is expected to be called by an external
*                   function which handles interrupts on the system (SYS) vector.  It is expected as well
*                   that that function will clear the SYS interrupt.
*
*               (2) The AIC End Of Interrupt Command Register (AIC_EOICR) will need to be written by the
*                   generic interrupt handler which vectored to this specific IRQ handler after this
*                   function returns.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    AT91SAM_USART_REG  *p_usart_reg;
    CPU_INT08U          rx_data;
    CPU_INT32U          reg_val;

    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    
     
    reg_val = p_usart_reg->US_CSR;             

                                                                 /* If we received a byte                             */
    if (DEF_BIT_IS_SET(reg_val, AT91SAM_REG_US_RXRDY)) {
        rx_data = (CPU_INT08U)(p_usart_reg->US_RHR & 0x00FF);   /* Remove the data from the holding register          */
        ProbeRS232_RxHandler(rx_data);                          /* Call the generic Rx handler                        */
    }

                                                                /* If we completed transmitting a byte                */
    if (DEF_BIT_IS_SET(reg_val, AT91SAM_REG_US_TXRDY)) {
        ProbeRS232_TxHandler();                                 /* Call the generic Tx handler                        */
    }

    if (DEF_BIT_IS_SET(reg_val, AT91SAM_REG_US_OVRE)) {
        p_usart_reg->US_CSR = AT91SAM_REG_US_RSTSTA;            /* If an overrun occurs, reset the OR flag            */
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
    AT91SAM_USART_REG  *p_usart_reg;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    
    
    p_usart_reg->US_IDR  = AT91SAM_REG_US_RXRDY;
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
    AT91SAM_USART_REG  *p_usart_reg;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    

     p_usart_reg->US_IER = AT91SAM_REG_US_RXRDY;
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
    AT91SAM_USART_REG  *p_usart_reg;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    

    p_usart_reg ->US_THR = c;
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
    AT91SAM_USART_REG  *p_usart_reg;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    

    p_usart_reg ->US_IDR = AT91SAM_REG_US_TXRDY;
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
    AT91SAM_USART_REG  *p_usart_reg;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_DBG)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_DBG_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US0_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US1_BASE_ADDR);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US2_BASE_ADDR);
#endif
     
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
     p_usart_reg = (AT91SAM_USART_REG *)(AT91SAM_REG_US3_BASE_ADDR);
#endif    

    p_usart_reg ->US_IER = AT91SAM_REG_US_TXRDY;
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
