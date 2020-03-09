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
*                                            Oscilloscope
*
* File    : PROBE_SCOPE_CFG.H
* Version : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            CONFIGURATION
*********************************************************************************************************
*/

#define  PROBE_SCOPE_MAX_CH                       8    /* The maximum number of channels: [1,8].                      */
#define  PROBE_SCOPE_MAX_SAMPLES               1000    /* The maximum number of samples per channel.                  */
#define  PROBE_SCOPE_16_BIT_EN                    1    /* The maximum size of each sample is 16-bits: [0,1].          */
#define  PROBE_SCOPE_32_BIT_EN                    1    /* The maximum size of each sample is 32-bits: [0,1].          */
#define  PROBE_SCOPE_SAMPLING_CLK_HZ_DFLT      1000    /* Default freq (Hz) to configure the timer at init.           */
#define  PROBE_SCOPE_IPL                         13
