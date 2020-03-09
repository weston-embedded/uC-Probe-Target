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
*                                  Port for the Microchip PIC32
*
* Filename : probe_rs232c.c
* Version  : V2.30
*********************************************************************************************************
*/

#include  <includes.h>
#include  <probe_com_cfg.h>
#include  "uart.h"

#if (PROBE_COM_METHOD_RS232 == DEF_TRUE)

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*/

#if PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1

#define  PROBERS232_PUTCHAR(x)      putcUART1(x)
#define  PROBERS232_DATARDY()       DataRdyUART1()
#define  PROBERS232_GETCHAR()       ReadUART1()
#define  PROBERS232_INTCONFIG(x)    ConfigIntUART1(x)
#define  PROBERS232_START(x, y, z)  OpenUART1(x, y, z)
#define  PROBERS232_INTENABLE(x)    EnIntUART1(x)
#define  PROBERS232_INTDISABLE(x)   DisIntUART1(x)
#define  PROBERS232_INTEN           IEC0
#define  PROBERS232_INTSTAT         IFS0
#define  PROBERS232_INTCLR          IFS0CLR
#define  PROBERS232_INTMASK         0x1C000000
#define  PROBERS232_INTTX           0x10000000
#define  PROBERS232_INTRX           0x08000000
#define  PROBERS232_INTERR          0x04000000

#elif PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2

#define  PROBERS232_PUTCHAR(x)      putcUART2(x)
#define  PROBERS232_DATARDY()       DataRdyUART2()
#define  PROBERS232_GETCHAR()       ReadUART2()
#define  PROBERS232_INTCONFIG(x)    ConfigIntUART2(x)
#define  PROBERS232_START(x, y, z)  OpenUART2(x, y, z)
#define  PROBERS232_INTENABLE(x)    EnIntUART2(x)
#define  PROBERS232_INTDISABLE(x)   DisIntUART2(x)
#define  PROBERS232_INTEN           IEC1
#define  PROBERS232_INTSTAT         IFS1
#define  PROBERS232_INTCLR          IFS1CLR
#define  PROBERS232_INTMASK         0x00000700
#define  PROBERS232_INTTX           0x00000400
#define  PROBERS232_INTRX           0x00000200
#define  PROBERS232_INTERR          0x00000100

#endif

/*
*********************************************************************************************************
*                                        FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  ProbeRS232_Config    (CPU_INT32U baud_rate);
static  void  ProbeRS232_ConfigInt (void);

/*
*********************************************************************************************************
*                       Initialize COM port for uC/Probe RS-232 Communication Module
*
* Description: Initialize UART for uC/Probe communication.
*
* Argument(s): baud    intended baud rate of the RS-232 channel.
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud)
{
    ProbeRS232_Config(PROBE_BAUD_RATE);                                 /* Configure the UART for the specified baud rate   */
    ProbeRS232_ConfigInt();                                             /* Configure the interrupt settings                 */
}

/*
*********************************************************************************************************
*                       Configure COM port for uC/Probe RS-232 Communication Module
*
* Description: configure UART for uC/Probe communication.
*
* Argument(s): baud    intended baud rate of the RS-232 channel.
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

static  void  ProbeRS232_Config (CPU_INT32U baud_rate)
{    
    CPU_INT32U  ubrg;
    CPU_INT32U  config1;
    CPU_INT32U  config2;
    
  
    mPORTAClearBits(DEF_BIT_07);                                        /* Turn off RA7 on startup.                         */
    mPORTASetPinsDigitalOut(DEF_BIT_07);                                /* Make RA7 as output.                              */
    
    ubrg    = UART_CalcBRG(baud_rate);                                  /* Calculate the correct multiplier                 */
    
    config1 = UART_EN                                                   /* UART module enabled                              */
            | UART_IDLE_CON                                             /* UART works in IDLE mode                          */
            | UART_RX_TX                                                /* Communication is done through the normal pins    */
            | UART_DIS_WAKE                                             /* Disable Wake-up on START bit detect during SLEEP */
            | UART_DIS_LOOPBACK                                         /* Disable loop back                                */
            | UART_DIS_ABAUD                                            /* Input to capture module from ICx pin             */
            | UART_NO_PAR_8BIT                                          /* 8 bits no parity                                 */
            | UART_1STOPBIT                                             /* 1 stop bit                                       */
            | UART_IRDA_DIS                                             /* IrDA disabled                                    */
            | UART_MODE_FLOWCTRL                                        /* UART pins in flow control mode                   */
            | UART_DIS_BCLK_CTS_RTS                                     /* Disable BCLK, CTS, and RTS pins                  */
            | UART_NORMAL_RX                                            /* UxRX idle stat is '1'                            */
            | UART_BRGH_SIXTEEN;                                        /* 16x baud clock                                   */
            
    config2 = UART_TX_PIN_LOW                                           /* IrDA encoded UxTx idle stat is '0'               */
            | UART_RX_ENABLE                                            /* Enable UxRx pin                                  */
            | UART_TX_ENABLE                                            /* Enable UxTx pin                                  */
            | UART_INT_TX_BUF_EMPTY                                     /* Interrupt on TXBUF becoming empty                */
            | UART_INT_RX_CHAR                                          /* Interrupt on every char received                 */
            | UART_ADR_DETECT_DIS                                       /* Disable 9-bit address detect                     */
            | UART_RX_OVERRUN_CLEAR;                                    /* Rx buffer overrun status bit clear               */
            
    PROBERS232_START(config1, config2, ubrg);                           /* Configure the settings                           */
}

/*
*********************************************************************************************************
*                 Configure interrupt settings for uC/Probe RS-232 Communication Module
*
* Description: Configure interrupts settings for uC/Probe communication.
*
* Argument(s): None
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

static  void  ProbeRS232_ConfigInt (void)
{
    CPU_INT08U  config;
    
    
    config = UART_INT_PR3                                               /* Interrupt priority 3                             */
           | UART_INT_SUB_PR0;                                           /* Interrupt sub priority 0                         */
           
    PROBERS232_INTCONFIG(config);                                       /* Configure the settings                           */
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
* Note(s)    : None
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    CPU_INT32U  status;
    CPU_INT32U  int_en;
    
    
    status             = PROBERS232_INTSTAT;                            /* Read the interrupt status register               */
    int_en             = PROBERS232_INTEN;                              /* Read the interrupt enabled register              */
    
    status            &= PROBERS232_INTMASK;                            /* Mask all other interrupts                        */
    PROBERS232_INTCLR  = status;                                        /* Clear all triggered interrupts                   */
    status            &= int_en;                                        /* Mask non-enabled interrupts                      */
    
    if (status & PROBERS232_INTTX) {                                    /* If a Tx interrupt has occured and is enabled...  */
        ProbeRS232_TxHandler();                                         /* Inform Probe about the interrupt                 */
    }
    
    if (status & PROBERS232_INTRX) {                                    /* If a Rx interrupt has occured and is enabled...  */
        ProbeRS232_RxHandler(PROBERS232_GETCHAR());                     /* Notify Probe and provide the incoming character  */
    }
}

/*
*********************************************************************************************************
*                                 Rx Communication Handler
*
* Description: This functions handle Rx interrupts
*
* Argument(s): None
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
}

/*
*********************************************************************************************************
*                                 Tx Communication Handler
*
* Description: This functions handle Tx interrupts
*
* Argument(s): None
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
}

/*
*********************************************************************************************************
*                                      Transmit One Byte
*
* Description: This function transmits one byte.
*
* Argument(s): c   byte to transmit.
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
    PROBERS232_PUTCHAR(c);                                              /* Put one character out onto the serial port               */
}

/*
*********************************************************************************************************
*                                      Enable/disable Tx Interrupts
*
* Description: Enable or disable Tx interrupts.
*
* Argument(s): None
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
    PROBERS232_INTDISABLE(UART_TX_INT);                                 /* Disable Tx interrupts                                    */
}

void  ProbeRS232_TxIntEn (void)
{
    PROBERS232_INTENABLE(UART_TX_INT);                                  /* Enable Tx interrupts                                     */
}


/*
*********************************************************************************************************
*                                      Enable/disable Rx Interrupts
*
* Description: Enable or disable Rx interrupts.
*
* Argument(s): None
*
* Returns    : None
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
    PROBERS232_INTDISABLE(UART_RX_INT);                                 /* Disable Rx Interrupts                                    */
}

void  ProbeRS232_RxIntEn (void)
{
    PROBERS232_INTENABLE(UART_RX_INT);                                  /* Enable Rx interrupts                                     */
}

#endif
