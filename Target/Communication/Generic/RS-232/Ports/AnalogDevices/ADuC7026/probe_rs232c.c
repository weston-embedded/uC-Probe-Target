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
*                                PORT FOR THE ANALOG DEVICES ADuC7026
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

#include  <includes.h>
#include  <probe_com.h>
#include  <probe_rs232.h>


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

                                                                /* ----------- REGISTER BASE SPECIFICATIONS ----------- */
#define  ADuC7xxx_GPIO_BASE          ((CPU_INT32U)0xFFFFF400)
#define  ADuC7xxx_COM_BASE           ((CPU_INT32U)0xFFFF0700)

                                                                /* --------------- UART REGISTER DEFINES -------------- */
#define  ADuC7xxx_COMTX        (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x00))         /* Transmit      register.  */
#define  ADuC7xxx_COMRX        (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x00))         /* Receive       register.  */
#define  ADuC7xxx_COMDIV0      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x00))         /* Low-byte div  register.  */
#define  ADuC7xxx_COMIEN0      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x04))         /* Int en        register.  */
#define  ADuC7xxx_COMDIV1      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x04))         /* High-byte div register.  */
#define  ADuC7xxx_COMIID0      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x08))         /* Int ID        register.  */
#define  ADuC7xxx_COMCON0      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x0C))         /* Line  control register.  */
#define  ADuC7xxx_COMCON1      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x10))         /* Modem control register.  */
#define  ADuC7xxx_COMSTA0      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x14))         /* Line  status  register.  */
#define  ADuC7xxx_COMSTA1      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x18))         /* Modem status  register.  */
#define  ADuC7xxx_COMSCR       (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x1C))         /* Scratch       register.  */
#define  ADuC7xxx_COMIEN1      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x20))         /* Network en    register.  */
#define  ADuC7xxx_COMIID1      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x24))         /* Network int   register.  */
#define  ADuC7xxx_COMADR       (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x28))         /* Network addr  register.  */
#define  ADuC7xxx_COMDIV2      (*(volatile CPU_INT08U *)(ADuC7xxx_COM_BASE + 0x2C))         /* Frac baud div register.  */

                                                                /* --------------- GPIO REGISTER DEFINES -------------- */
#define  ADuC7xxx_GP1CON       (*(volatile CPU_INT32U *)(ADuC7xxx_GPIO_BASE + 0x04))        /* GPIO0 config register.   */

                                                                /* -------------------- IRQ SOURCES ------------------- */
#define  ADuC7xxx_IRQ_UART                        14


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
* Argument(s) : baud_rate       Intended baud rate of the RS-232.
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

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    CPU_INT16U  div;
    CPU_INT08U  divlo;
    CPU_INT08U  divhi;
    CPU_INT32U  clk_freq;
    CPU_INT32U  gp;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
                                                                /* ------------- COMPUTE DIVISOR BAUD RATE ------------ */
    clk_freq = BSP_CPU_ClkFreq();
    div      = (CPU_INT16U)((clk_freq / 16 / baud_rate + 1) / 2);
    divlo    = (CPU_INT08U)(div & 0x00FF);
    divhi    = (CPU_INT08U)((div >> 8) & 0x00FF);

                                                                /* ------------------ SETUP UART GPIO ----------------- */
    gp               = ADuC7xxx_GP1CON;
    gp              &= 0xFFFFFF00;
    gp              |= 0x00000011;
    ADuC7xxx_GP1CON  = gp;

                                                                /* -------------------- SETUP UART -------------------- */
    ADuC7xxx_COMCON0 = DEF_BIT_07;                              /* Access low-byte divider latch.                       */
    ADuC7xxx_COMDIV0 = divlo;                                   /* Load   low-byte of divisor.                          */
    ADuC7xxx_COMDIV1 = divhi;                                   /* Load  high-byte of divisor.                          */
    ADuC7xxx_COMCON0 = 0x07;                                    /* 8 data bits, 1 stop bit, no parity.                  */
    ADuC7xxx_COMIEN0 = 0x00;                                    /* Disable both RX and TX interrupts.                   */

                                                                /* ------------------ SETUP UART IRQ ------------------- */
    BSP_IntVectSet(ADuC7xxx_IRQ_UART, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(ADuC7xxx_IRQ_UART);
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
* Note(s)     : (1) none.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    volatile  CPU_INT08U  rx_data;
    volatile  CPU_INT08U  lsr;
    volatile  CPU_INT08U  iir;


#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    iir = ADuC7xxx_COMIID0 & 0x0F;
    while (iir != 1) {
        switch (iir) {
            case  2:                                            /* Transmitted character?                               */
                 ProbeRS232_TxHandler();
                 break;

            case  4:                                            /* Received a character?                                */
                 rx_data = ADuC7xxx_COMRX;
                 ProbeRS232_RxHandler(rx_data);                 /* Call the generic Rx handler                          */
                 break;

            case 6:
                 lsr = ADuC7xxx_COMSTA0;
                 break;

            default:
                 break;
        }
        iir = ADuC7xxx_COMIID0 & 0x0F;
    }
#endif
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    ADuC7xxx_COMIEN0 &= ~(DEF_BIT_00 | DEF_BIT_02);
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
    ADuC7xxx_COMIEN0 |= DEF_BIT_00 | DEF_BIT_02;
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
#if (PROBE_RS232_CFG_COMM_SEL == PROBE_RS232_UART_0)
    ADuC7xxx_COMTX = c;
#endif
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
    ADuC7xxx_COMIEN0 &= ~DEF_BIT_01;
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
    ADuC7xxx_COMIEN0 |= DEF_BIT_01;
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
