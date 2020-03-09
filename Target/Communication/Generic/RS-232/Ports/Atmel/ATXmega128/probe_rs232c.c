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
*                                   Port for the Atmel Xmega128
*
* Filename : probe_rs232c.c
* Version  :  V2.30
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232.h>
#include  <bsp.h>

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* ----------------- REGISTER BASE ADDRESS ----------------- */
#define  PORTC_BASE_ADDR                       (CPU_INT16U)(0x0640)
#define  PORTD_BASE_ADDR                       (CPU_INT16U)(0x0660)
#define  PORTE_BASE_ADDR                       (CPU_INT16U)(0x0680)
#define  PORTF_BASE_ADDR                       (CPU_INT16U)(0x06A0)

#define  USARTC0_BASE_ADDR                     (CPU_INT16U)(0x08A0)
#define  USARTC1_BASE_ADDR                     (CPU_INT16U)(0x08B0)
#define  USARTD0_BASE_ADDR                     (CPU_INT16U)(0x09A0)
#define  USARTD1_BASE_ADDR                     (CPU_INT16U)(0x09B0)
#define  USARTE0_BASE_ADDR                     (CPU_INT16U)(0x0AA0)
#define  USARTE1_BASE_ADDR                     (CPU_INT16U)(0x0AB0)
#define  USARTF0_BASE_ADDR                     (CPU_INT16U)(0x0BA0)
#define  USARTF1_BASE_ADDR                     (CPU_INT16U)(0x0BB0)

#define  PMIC_BASE_ADDR                        (CPU_INT16U)0x00A0
                                                                /* ---------------- PMIC  REGISTER DEFINITION -------------- */
#define  PMIC_CTRL                   (*(volatile CPU_INT08U *)(PMIC_BASE_ADDR + 0x02))
                                                                /* -------------- PORT  REGISTER DEFINITION ---------------- */
#define  PORTC_DIRSET                (*(volatile CPU_INT08U *)(PORTC_BASE_ADDR + 0x01))
#define  PORTC_DIRCLR                (*(volatile CPU_INT08U *)(PORTC_BASE_ADDR + 0x02))
#define  PORTC_OUTSET                (*(volatile CPU_INT08U *)(PORTC_BASE_ADDR + 0x05))

#define  PORTD_DIRSET                (*(volatile CPU_INT08U *)(PORTD_BASE_ADDR + 0x01))
#define  PORTD_DIRCLR                (*(volatile CPU_INT08U *)(PORTD_BASE_ADDR + 0x02))
#define  PORTD_OUTSET                (*(volatile CPU_INT08U *)(PORTD_BASE_ADDR + 0x05))

#define  PORTE_DIRSET                (*(volatile CPU_INT08U *)(PORTE_BASE_ADDR + 0x01))
#define  PORTE_DIRCLR                (*(volatile CPU_INT08U *)(PORTE_BASE_ADDR + 0x02))
#define  PORTE_OUTSET                (*(volatile CPU_INT08U *)(PORTE_BASE_ADDR + 0x05))

#define  PORTF_DIRSET                (*(volatile CPU_INT08U *)(PORTF_BASE_ADDR + 0x01))
#define  PORTF_DIRCLR                (*(volatile CPU_INT08U *)(PORTF_BASE_ADDR + 0x02))
#define  PORTF_OUTSET                (*(volatile CPU_INT08U *)(PORTF_BASE_ADDR + 0x05))

                                                                /* --------------- USART  REGISTER DEFINITION -------------- */
#define  USARTC0_DATA                (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x00))
#define  USARTC0_STATUS              (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x01))
#define  USARTC0_CTRLA               (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x03))
#define  USARTC0_CTRLB               (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x04))
#define  USARTC0_CTRLC               (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x05))
#define  USARTC0_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x06))
#define  USARTC0_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTC0_BASE_ADDR + 0x07))

#define  USARTC1_DATA                (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x00))
#define  USARTC1_STATUS              (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x01))
#define  USARTC1_CTRLA               (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x03))
#define  USARTC1_CTRLB               (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x04))
#define  USARTC1_CTRLC               (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x05))
#define  USARTC1_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x06))
#define  USARTC1_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTC1_BASE_ADDR + 0x07))

#define  USARTD0_DATA                (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x00))
#define  USARTD0_STATUS              (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x01))
#define  USARTD0_CTRLA               (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x03))
#define  USARTD0_CTRLB               (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x04))
#define  USARTD0_CTRLC               (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x05))
#define  USARTD0_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x06))
#define  USARTD0_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTD0_BASE_ADDR + 0x07))

#define  USARTD1_DATA                (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x00))
#define  USARTD1_STATUS              (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x01))
#define  USARTD1_CTRLA               (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x03))
#define  USARTD1_CTRLB               (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x04))
#define  USARTD1_CTRLC               (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x05))
#define  USARTD1_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x06))
#define  USARTD1_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTD1_BASE_ADDR + 0x07))

#define  USARTE0_DATA                (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x00))
#define  USARTE0_STATUS              (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x01))
#define  USARTE0_CTRLA               (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x03))
#define  USARTE0_CTRLB               (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x04))
#define  USARTE0_CTRLC               (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x05))
#define  USARTE0_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x06))
#define  USARTE0_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTE0_BASE_ADDR + 0x07))

#define  USARTE1_DATA                (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x00))
#define  USARTE1_STATUS              (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x01))
#define  USARTE1_CTRLA               (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x03))
#define  USARTE1_CTRLB               (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x04))
#define  USARTE1_CTRLC               (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x05))
#define  USARTE1_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x06))
#define  USARTE1_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTE1_BASE_ADDR + 0x07))

#define  USARTF0_DATA                (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x00))
#define  USARTF0_STATUS              (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x01))
#define  USARTF0_CTRLA               (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x03))
#define  USARTF0_CTRLB               (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x04))
#define  USARTF0_CTRLC               (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x05))
#define  USARTF0_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x06))
#define  USARTF0_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTF0_BASE_ADDR + 0x07))

#define  USARTF1_DATA                (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x00))
#define  USARTF1_STATUS              (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x01))
#define  USARTF1_CTRLA               (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x03))
#define  USARTF1_CTRLB               (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x04))
#define  USARTF1_CTRLC               (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x05))
#define  USARTF1_BAUDCTRLA           (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x06))
#define  USARTF1_BAUDCTRLB           (*(volatile CPU_INT08U *)(USARTF1_BASE_ADDR + 0x07))

#define  PORTX_USART0_RX                       DEF_BIT_02       /* UART 0 Port C/D/E/F  pins settings                        */
#define  PORTX_USART0_TX                       DEF_BIT_03        

#define  PORTX_USART1_RX                       DEF_BIT_06       /* UART 1 Port C/D/E/D  pins settings                        */
#define  PORTX_USART1_TX                       DEF_BIT_07
                                                                /* -------------- USART REGISTER BIT DEFINES --------------- */
#define  USART_STAT_RXCIF                      DEF_BIT_07
#define  USART_STAT_TXCIF                      DEF_BIT_06
#define  USART_STAT_DREIF                      DEF_BIT_05
#define  USART_STAT_FERR                       DEF_BIT_04
#define  USART_STAT_BUFOVF                     DEF_BIT_03
#define  USART_STAT_PERR                       DEF_BIT_02
#define  USART_STAT_RXB8                       DEF_BIT_00

#define  USART_STAT_ERR_ANY               USART_STAT_FERR  |  \
                                        USART_STAT_BUFOVF  |  \
                                          USART_STAT_PERR 

#define  USART_CTRLB_RX_EN                     DEF_BIT_04
#define  USART_CTRLB_TX_EN                     DEF_BIT_03

#define  USART_CMODE_ASYNC                    (0x00 << 6)
#define  USART_CMODE_SYNC                     (0x01 << 6)
#define  USART_CMODE_IRCOM                    (0x02 << 6)
#define  USART_CMODE_MSPI                     (0x03 << 6)                                                       

#define  USART_PMODE_PARITY_DIS               (0x00 << 4)
#define  USART_PMODE_PARITY_EVEN              (0x02 << 4)
#define  USART_PMODE_PARITY_ODD               (0x03 << 4)

#define  USART_SBMODE_ONE                     (0x00 << 3)
#define  USART_SBMODE_TWO                     (0x01 << 3)

#define  USART_CHSIZE_5                       (0x00 << 0)
#define  USART_CHSIZE_6                       (0x01 << 0)
#define  USART_CHSIZE_7                       (0x02 << 0)
#define  USART_CHSIZE_8                       (0x03 << 0)
#define  USART_CHSIZE_9                       (0x07 << 0)



/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
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
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT32U   clk_div;                                       /* Clock divider                                             */
    CPU_INT32U   per_freq;                                      /* USART peripheral clk                                      */

    
    per_freq           =  BSP_SysClk_DevGetFreq(BSP_SYS_CLK_OUT_PER);    
    clk_div            =  ((per_freq / baud_rate) - 1) / 16;    /* Calculate the clk divider for the baud rate generator     */
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    PORTC_OUTSET       = PORTX_USART0_TX;                       /* set PC3 (TXC0) state as high                              */ 
    PORTC_DIRSET       = PORTX_USART0_TX;                       /* set PC3 (TXC0) as output pin                              */
    PORTC_DIRCLR       = PORTX_USART0_RX;                       /* set PC2 (RXC0) as input pin                               */

    USARTC0_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTC0_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTC0_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTC0_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTC0_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    PORTC_OUTSET       = PORTX_USART1_TX;                       /* set PC7 (TXC1) state as high                              */ 
    PORTC_DIRSET       = PORTX_USART1_TX;                       /* set PC7 (TXC1) as output pin                              */
    PORTC_DIRCLR       = PORTX_USART1_RX;                       /* set PC6 (RXC1) as input pin                               */

    USARTC1_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTC1_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTC1_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTC1_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTC1_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    PORTD_OUTSET       = PORTX_USART0_TX;                       /* set PD3 (TXD0) state as high                              */ 
    PORTD_DIRSET       = PORTX_USART0_TX;                       /* set PD3 (TXD0) as output pin                              */
    PORTD_DIRCLR       = PORTX_USART0_RX;                       /* set PD2 (RXD0) as input pin                               */

    USARTD0_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTD0_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTD0_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTD0_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTD0_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    PORTD_OUTSET       = PORTX_USART1_TX;                       /* set PD7 (TXD1) state as high                              */ 
    PORTD_DIRSET       = PORTX_USART1_TX;                       /* set PD7 (TXD1) as output pin                              */
    PORTD_DIRCLR       = PORTX_USART1_RX;                       /* set PD6 (RXD1) as input pin                               */

    USARTD1_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTD1_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTD1_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTD1_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTD1_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    PORTE_OUTSET       = PORTX_USART0_TX;                       /* set PE3 (TXE0) state as high                              */ 
    PORTE_DIRSET       = PORTX_USART0_TX;                       /* set PE3 (TXE0) as output pin                              */
    PORTE_DIRCLR       = PORTX_USART0_RX;                       /* set PE2 (RXE0) as input pin                               */

    USARTE0_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTE0_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTE0_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTE0_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTE0_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    PORTE_OUTSET       = PORTX_USART1_TX;                       /* set PE7 (TXD1) state as high                              */ 
    PORTE_DIRSET       = PORTX_USART1_TX;                       /* set PE7 (TXD1) as output pin                              */
    PORTE_DIRCLR       = PORTX_USART1_RX;                       /* set PE6 (RXD1) as input pin                               */

    USARTE1_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTE1_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTE1_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTE1_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTE1_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    PORTF_OUTSET       = PORTX_USART0_TX;                       /* set PF7 (TXE0) state as high                              */ 
    PORTF_DIRSET       = PORTX_USART0_TX;                       /* set PF7 (TXE0) as output pin                              */
    PORTF_DIRCLR       = PORTX_USART0_RX;                       /* set PF6 (RXE0) as input pin                               */

    USARTF0_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTF0_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTF0_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTF0_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTF0_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    PORTF_OUTSET       = PORTX_USART1_TX;                       /* set PF7 (TXD1) state as high                              */ 
    PORTF_DIRSET       = PORTX_USART1_TX;                       /* set PF7 (TXD1) as output pin                              */
    PORTF_DIRCLR       = PORTX_USART1_RX;                       /* set PF6 (RXD1) as input pin                               */

    USARTF1_CTRLA      = 0x00;                                  /* Disable all interrupts                                    */
    USARTF1_CTRLC      = USART_CMODE_ASYNC                      /* Asynchronous USART                                        */
                       | USART_PMODE_PARITY_DIS                 /* Parity disable                                            */
                       | USART_SBMODE_ONE                       /* Stop bit : 1                                              */
                       | USART_CHSIZE_8;                        /* 8-bit chatacter                                           */

    USARTF1_BAUDCTRLA  = (CPU_INT08U)((CPU_INT16U)clk_div  & 0xFF);
    USARTF1_BAUDCTRLB  = (CPU_INT08U)((CPU_INT16U)clk_div >>    8);

    USARTF1_CTRLB      = USART_CTRLB_RX_EN                      /* Enable RX & TX                                            */
                       | USART_CTRLB_TX_EN;              
#endif
    
    PMIC_CTRL         |= DEF_BIT_02;                             /* Enable High Level interrupts                             */
    
}

/*
*********************************************************************************************************
*                                 Rx and Tx Communication Handler
*
* Description: This function handles both Rx and Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*
* Note(s)    : This function is empty because Rx and Tx interrupts are both handled in separate
*              handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    
}

/*
*********************************************************************************************************
*                                 ProbeRS232_RxISRHandler
*
* Description: These functions handle the Rx and Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    CPU_INT08U  rx_data;
    CPU_INT08U  status;
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    status = USARTC0_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    status = USARTC1_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    status = USARTD0_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    status = USARTD1_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    status = USARTE0_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    status = USARTE1_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    status = USARTF0_STATUS;
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    status = USARTF1_STATUS;
#endif    
       

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    rx_data = (CPU_INT08U)(USARTC0_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    rx_data = (CPU_INT08U)(USARTC1_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    rx_data = (CPU_INT08U)(USARTD0_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    rx_data = (CPU_INT08U)(USARTD1_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    rx_data = (CPU_INT08U)(USARTE0_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    rx_data = (CPU_INT08U)(USARTE1_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    rx_data = (CPU_INT08U)(USARTF0_DATA & 0xFF);
#endif    

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    rx_data = (CPU_INT08U)(USARTF1_DATA & 0xFF);
#endif    
    
	if (DEF_BIT_IS_SET_ANY(status, USART_STAT_ERR_ANY)) {   /* If a error flag is detected return form the ISR          */    
		return;                                              
    }

    ProbeRS232_RxHandler(rx_data);                          /* Call the generic Rx handler                              */
}


/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*                                      ProbeRS232_TxISRHandler()
*
* Description: Handle Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : none.

*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
    ProbeRS232_TxHandler();  
}


void  ProbeRS232_Tx1 (CPU_INT08U c)
{
   
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    USARTC0_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    USARTC1_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    USARTD0_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    USARTD1_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    USARTE0_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    USARTE1_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    USARTF0_DATA = c;
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    USARTF1_DATA = c;
#endif
    
}

/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntDis()
*
* Description: Enables or disables Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    USARTC0_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    USARTC1_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    USARTD0_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    USARTD1_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    USARTE0_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    USARTE1_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    USARTF0_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    USARTF1_CTRLA &= ~(DEF_BIT_03 | DEF_BIT_02);
#endif
}
/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntEn()
*
* Description: Enable Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    USARTC0_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    USARTC1_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    USARTD0_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    USARTD1_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    USARTE0_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    USARTE1_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    USARTF0_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    USARTF1_CTRLA |= (DEF_BIT_02 | DEF_BIT_03);
#endif
}

/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntDis()
*
* Description: Disable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    USARTC0_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    USARTC1_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    USARTD0_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    USARTD1_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    USARTE0_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    USARTE1_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    USARTF0_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    USARTF1_CTRLA &= ~(DEF_BIT_05 | DEF_BIT_04);
#endif

}

/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntEn()
*
* Description: Enable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    USARTC0_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_1)
    USARTC1_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_2)
    USARTD0_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif
    
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_3)
    USARTD1_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_4)
    USARTE0_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_5)
    USARTE1_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_6)
    USARTF0_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif

#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_7)
    USARTF1_CTRLA |= (DEF_BIT_04 | DEF_BIT_05);
#endif
}

#endif
