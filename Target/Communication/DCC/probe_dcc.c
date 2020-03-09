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
*                                         COMMUNICATION: DCC
*
* Filename : probe_dcc.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This code in this module responds to DCC requests from a J-Link or an
*                AtmelICE JTAG debugger.  The target REQUIRES either an ARM7 or ARM9 target
*                processor.
*
*            (2) Certain aspects of this code--such as the inline assembly in ProbeDCC_ReadCtrl() or
*                the #pragma directive used to suppress the compiler warning--are specific to IAR
*                EWARM.  If a J-Link or an AtmelICE JTAG debugger is used with a different
*                toolchain, these elements might need to be modified.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    PROBE_DCC_MODULE
#include  <probe_dcc.h>


                                                                /* This directive suppresses warnings for non-void ...  */
#pragma  diag_suppress=Pe940                                    /* ... functions with no return values.                 */

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  PROBE_DCC_OP_READ_U32                    DEF_BIT_24
#define  PROBE_DCC_OP_READ_U16                    DEF_BIT_25
#define  PROBE_DCC_OP_READ_U8                     DEF_BIT_26
#define  PROBE_DCC_OP_GET_CAPS                    DEF_BIT_27
#define  PROBE_DCC_OP_WRITE_U32                   DEF_BIT_28
#define  PROBE_DCC_OP_WRITE_U16                   DEF_BIT_29
#define  PROBE_DCC_OP_WRITE_U8                    DEF_BIT_30
#define  PROBE_DCC_OP_ODD_ADDR                    DEF_BIT_31
#define  PROBE_DCC_OP_COMMAND                     DEF_BIT_00

#define  PROBE_DCC_COMM_CTRL_RD                   DEF_BIT_00
#define  PROBE_DCC_COMM_CTRL_WR                   DEF_BIT_01

#define  PROBE_DCC_SIGNATURE                      0x91CA0000u
#define  PROBE_DCC_CONFIG                         0x00000077u


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


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  CPU_INT32U  ProbeDCC_Cmd;
static  CPU_INT32U  ProbeDCC_Addr;
static  CPU_INT32U  ProbeDCC_ItemCnt;
static  CPU_INT32U  ProbeDCC_Data;


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
*                                          ProbeDCC_RdCtrl()
*
* Description : Retrieves data from the comms control register.
*
* Argument(s) : none.
*
* Return(s)   : The contents of the comms control register
*
* Caller(s)   : ProbeDCC_Handler().
*
* Note(s)     : (1) This function uses a coprocessor register transfer instruction to place the contents
*                   of the comms control register in R0.  Thus, the function does not contain an
*                   explicit return statement.  "#pragma diag_suppress=Pe940", which appears at the
*                   top of this file, is used to suppress the warning that normally results from non-
*                   void functions lacking return statements.
*********************************************************************************************************
*/

static  __arm  CPU_INT32U  ProbeDCC_RdCtrl (void)
{
    __asm("mrc  P14,0,R0,C0,C0");
}


/*
*********************************************************************************************************
*                                            ProbeDCC_Rd()
*
* Description : Retrieves data from the comms data read register.
*
* Argument(s) : none.
*
* Return(s)   : The contents of the comms data read register
*
* Caller(s)   : ProbeDCC_Handler().
*
* Note(s)     : (1) This function uses a coprocessor register transfer instruction to place the contents
*                   of the comms data read register in R0.  Thus, the function does not contain an
*                   explicit return statement.  "#pragma diag_suppress=Pe940", which appears at the
*                   top of this file, is used to suppress the warning that normally results from non-
*                   void functions lacking return statements.
*********************************************************************************************************
*/

static  __arm  CPU_INT32U  ProbeDCC_Rd (void)
{
    __asm("mrc  P14,0,R0,C1,C0");
}


/*
*********************************************************************************************************
*                                            ProbeDCC_Wr()
*
* Description : Places data in the comms data write register.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeDCC_Handler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  __arm  void  ProbeDCC_Wr (CPU_INT32U data)
{
    __asm("mcr  P14,0,R0,C1,C0");
}


/*
*********************************************************************************************************
*                                         ProbeDCC_Handler()
*
* Description : Read commands from the DCC comms data read register.  Data may be transferred to or from
*               memory based on those commands.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function should be called periodically.  PROBE_CPU_ARM_DCC_EN can be set to 1 to
*                   disable the use of this function.
*********************************************************************************************************
*/

void  ProbeDCC_Handler (void)
{
    CPU_INT32U  reg_val;
    CPU_INT32U  ctrl;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();                                       /* Disable interrupts                                       */

                                                                /* Check for the presence of new data                       */
    ctrl = ProbeDCC_RdCtrl();
    if (DEF_BIT_IS_SET(ctrl, PROBE_DCC_COMM_CTRL_RD) == DEF_YES) {
        reg_val = ProbeDCC_Rd();                                /* Read the new data                                        */

                                                                /* Determine whether a command has been received            */
        if (DEF_BIT_IS_SET(reg_val, PROBE_DCC_OP_COMMAND) == DEF_YES) {
            ProbeDCC_Cmd = reg_val;
                                                                /* Check for an odd address in the next operation           */
            if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_ODD_ADDR) == DEF_YES) {
                ProbeDCC_Addr |= DEF_BIT_00;
            }
                                                                /* If data will be read, adjust OSDCC_ItemCnt               */
            if (DEF_BIT_IS_SET_ANY(ProbeDCC_Cmd, (PROBE_DCC_OP_READ_U32 | PROBE_DCC_OP_READ_U16 |
                                                  PROBE_DCC_OP_READ_U8  | PROBE_DCC_OP_GET_CAPS )) == DEF_YES) {

                ProbeDCC_ItemCnt = (ProbeDCC_Cmd >> 2) & 0x0000FFFFu;
            } else {                                            /* Data will be written; initialize OSDCC_Data              */

                if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_WRITE_U32) == DEF_YES) {
                    ProbeDCC_Data |= (ProbeDCC_Cmd << 14) & 0xFFFF0000u;
                } else {
                    ProbeDCC_Data  = (ProbeDCC_Cmd >>  2) & 0x0000FFFFu;
                }

                                                                /* Write a single byte                                      */
                if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_WRITE_U8) == DEF_YES) {
                    *(CPU_REG08 *)ProbeDCC_Addr = ProbeDCC_Data;
                    ProbeDCC_Addr += 1u;
                }

                                                                /* Write two bytes                                          */
                if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_WRITE_U16) == DEF_YES) {
                    *(CPU_REG16 *)ProbeDCC_Addr = ProbeDCC_Data;
                    ProbeDCC_Addr += 2u;
                }

                                                                /* Write four bytes                                         */
                if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_WRITE_U32) == DEF_YES) {
                    *(CPU_REG32 *)ProbeDCC_Addr = ProbeDCC_Data;
                    ProbeDCC_Addr += 4u;
                }
            }
            CPU_CRITICAL_EXIT();
            return;
        }

        ProbeDCC_Addr = reg_val;                                /* An address was received; ProbeDCC_Addr is updated        */
    }
                                                                /* Determine whether data must be read                      */
    if (ProbeDCC_ItemCnt != 0u) {
                                                                /* Confirm that the comms data write register...            */
                                                                /* ...is free from the processor point of view              */
        ctrl = ProbeDCC_ReadCtrl();
        if (DEF_BIT_IS_CLR(ctrl, PROBE_DCC_COMM_CTRL_WR) == DEF_YES) {

            reg_val = (PROBE_DCC_CONFIG | PROBE_DCC_SIGNATURE);

                                                                /* Read a single byte                                       */
            if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_READ_U8) == DEF_YES) {
                reg_val = *(CPU_REG08 *)ProbeDCC_Addr;
                ProbeDCC_Addr += 1u;
            }

                                                                /* Read two bytes                                           */
            if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_READ_U16) == DEF_YES) {
                reg_val = *(CPU_REG16 *)ProbeDCC_Addr;
                ProbeDCC_Addr += 2u;
            }

                                                                /* Read four bytes                                          */
            if (DEF_BIT_IS_SET(ProbeDCC_Cmd, PROBE_DCC_OP_READ_U32) == DEF_YES) {
                reg_val = *(CPU_REG32 *)ProbeDCC_Addr;
                ProbeDCC_Addr += 4u;
            }

            ProbeDCC_Wr(reg_val);                               /* Place data in the comms data write register              */
            ProbeDCC_ItemCnt--;                                 /* Decrement the number of items to be read                 */
        }
    }
    CPU_CRITICAL_EXIT();
}
