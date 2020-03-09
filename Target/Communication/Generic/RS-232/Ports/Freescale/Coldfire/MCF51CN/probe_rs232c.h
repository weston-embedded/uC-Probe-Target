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
*                                    PORT FOR FREESCALE MCF51CN128
*
* Filename : probe_rs232c.h
* Version  : V2.30
*********************************************************************************************************
*/

#ifndef  PROBE_RS232C_PRESENT
#define  PROBE_RS232C_PRESENT

/*
*********************************************************************************************************
*                                        FUNCTION PROTOTYPES
*********************************************************************************************************
*/

extern  void   ProbeRS232_ErrISR(void);                                 /* uC/Probe SCI Error Interrupt Service Routine             */
extern  void   ProbeRS232_RxISR(void);                                  /* uC/Probe SCI Rx Interrupt Service Routine                */
extern  void   ProbeRS232_TxISR(void);                                  /* uC/Probe SCI Tx Interrupt Service Routine                */

#endif
