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
*                                              uC/Probe
*                                    Terminal Window for uC/Probe
*
*                                          APPLICATION HOOKS
*
* File    : app_probe_term.c
* Version : V2.30
*********************************************************************************************************
*/

#include <probe_term.h>
#include  <probe_term_cfg.h>

#if PROBE_TERM_CFG_CMD_EN > 0

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  AppProbeTermHookRx (CPU_CHAR  *p_str);


/*
************************************************************************************************************************
*                                              SET ALL APPLICATION HOOKS
*
* Description: Set ALL application hooks.
*
* Arguments  : none.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermSetAllHooks (void)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    ProbeTermAppHookRxPtr = AppProbeTermHookRx;
    CPU_CRITICAL_EXIT();
}


/*
************************************************************************************************************************
*                                             CLEAR ALL APPLICATION HOOKS
*
* Description: Clear ALL application hooks.
*
* Arguments  : none.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermClrAllHooks (void)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    ProbeTermAppHookRxPtr = (PROBE_TERM_APP_HOOK_RX)0;
    CPU_CRITICAL_EXIT();
}


/*
************************************************************************************************************************
*                                            APPLICATION RECEPTION HOOK
*
* Description: This function is called when the embedded target has received a string from µC/Probe.
*              The string from µC/Probe is the command than needs to be processed, for example 'dir'
*              this callback function allows you to parse the command, process the command and send
*              the response back to µC/Probe by calling ProbeTermCmdPrint() as shown below.
*
* Arguments  : p_str   is a pointer to the string.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermHookRx (CPU_CHAR  *p_str)
{
    CPU_CHAR  buf[PROBE_TERM_CFG_BUF_SIZE];


    if (Str_CmpIgnoreCase_N(p_str, "dir", 3) == 0) {
        ProbeTermCmdPrint("file.txt\nphoto.jpg\nsong.mp3\n");
    } else if (Str_CmpIgnoreCase_N(p_str, "ipconfig", 8) == 0) {
        ProbeTermCmdPrint("IPv4 Address......: 10.10.1.149\n");
        ProbeTermCmdPrint("Subnet Mask.......: 255.255.255.0\n");
        ProbeTermCmdPrint("Default Gateway...: 10.10.1.1\n");
    } else if (Str_CmpIgnoreCase_N(p_str, "echo", 4) == 0) {
        if (Str_Len(p_str) > 5) {
            Str_Copy_N(&buf[0], &p_str[5], PROBE_TERM_CFG_BUF_SIZE - 2);
            Str_Cat_N(&buf[Str_Len(buf)], "\n\0", 2);
        } else {
            buf[0] = '\0';
        }
        ProbeTermCmdPrint(&buf[0]);
    } else if (Str_CmpIgnoreCase_N(p_str, "trace", 5) == 0) {
        if (Str_Len(p_str) > 6) {
            Str_Copy_N(&buf[0], &p_str[6], PROBE_TERM_CFG_BUF_SIZE - 2);
            Str_Cat_N(&buf[Str_Len(buf)], "\n\0", 2);
        } else {
            buf[0] = '\0';
        }
        ProbeTermCmdPrint("Ok.\n");
        ProbeTermTrcPrint(&buf[0]);
    } else {
        ProbeTermCmdPrint("Error: unrecognized or incomplete command line.\n");
    }
}
#endif