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
*                                      uC/Probe Terminal Window
*
* Filename : probe_term.c
* Version  : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   MICRIUM_SOURCE

#include  <probe_term_cfg.h>

#ifdef    PROBE_TERM_CFG_CMD_EN
#if       PROBE_TERM_CFG_CMD_EN > 0
#undef    PROBE_TERM_CFG_EN
#define   PROBE_TERM_CFG_EN       1
#endif
#else
#error   "PROBE_TERM_CFG_CMD_EN must be defined as shown in probe_term_cfg.h"
#endif

#ifdef    PROBE_TERM_CFG_TRC_EN
#if       PROBE_TERM_CFG_TRC_EN > 0
#undef    PROBE_TERM_CFG_EN
#define   PROBE_TERM_CFG_EN       1
#endif
#else
#error   "PROBE_TERM_CFG_TRC_EN must be defined as shown in probe_term_cfg.h"
#endif

#ifdef    PROBE_TERM_CFG_EN

#if       PROBE_TERM_CFG_EN     > 0

#define   PROBE_TERM_MODULE
#include  <probe_term.h>
#include  <cpu_core.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define     PROBE_TERM_RX_STATE_IDLE            0               /* Rx State Machine: Idle           State.              */
#define     PROBE_TERM_RX_STATE_INIT            1               /* Rx State Machine: Initialization State.              */
#define     PROBE_TERM_RX_STATE_WAIT4_DS        2               /* Rx State Machine: Wait for Data-Sent.                */

#define     PROBE_TERM_TX_STATE_IDLE            0               /* Tx State Machine: Idle           State.              */
#define     PROBE_TERM_TX_STATE_INIT            1               /* Tx State Machine: Initialization State.              */
#define     PROBE_TERM_TX_STATE_WAIT4_CTS       2               /* Tx State Machine: Wait for Clear-To-Send.            */

#define     PROBE_TERM_TIMEOUT_MSEC          2000               /* Time to wait for probe to raise its flags.           */

#define     PROBE_TERM_Q_MSG_TYPE_CMD           1               /* Message for command line purposes.                   */
#define     PROBE_TERM_Q_MSG_TYPE_TRC           2               /* Message for tracing      purposes.                   */


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

                                                                /* Max size of the buffers (255-chars max).             */
CPU_INT08U  const  ProbeTermBufSize = PROBE_TERM_CFG_BUF_SIZE;


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct  probe_term_states {                             /* --------- UC/PROBE TERMINAL WINDOW STATES ---------- */
    CPU_INT08U         RxState;                                 /* Current state of the Reception    state machine.     */
    CPU_INT08U         TxState;                                 /* Current state of the Transmission state machine.     */
    CPU_INT08U         RxCtr;                                   /* Counter for Reception    timeout purposes.           */
    CPU_INT08U         TxCtr;                                   /* Counter for Transmission timeout purposes.           */
} PROBE_TERM_STATES;

typedef struct  probe_term_sm {                                 /* ----- UC/PROBE TERMINAL WINDOW STATE MACHINES ------ */
#if PROBE_TERM_CFG_CMD_EN > 0
    PROBE_TERM_STATES  Cmd;                                     /* Command line interface.                              */
#endif
#if PROBE_TERM_CFG_TRC_EN > 0
    PROBE_TERM_STATES  Trc;                                     /* Tracing      interface.                              */
#endif
} PROBE_TERM_SM;

typedef struct  probe_term_q_msg  PROBE_TERM_Q_MSG;

        struct  probe_term_q_msg {                              /* ------ UC/PROBE TERMINAL WINDOW MESSAGE QUEUE ------ */
    CPU_CHAR           Msg[PROBE_TERM_CFG_BUF_SIZE];
    CPU_INT08U         MsgLen;
    PROBE_TERM_Q_MSG  *NextPtr;
};

typedef struct  probe_term_q {
    PROBE_TERM_Q_MSG   Q[PROBE_TERM_CFG_Q_SIZE];                /* This queue holds messages for trace and command line.*/
    PROBE_TERM_Q_MSG  *FreePtr;                                 /* Points to the next available item from the pool.     */
    PROBE_TERM_Q_MSG  *HeadPtr;                                 /* Points to the head of the message queue.             */
    PROBE_TERM_Q_MSG  *TailPtr;                                 /* Points to the tail of the message queue.             */
    CPU_INT32U         MsgCtr;                                  /* Number of messages in the queue.                     */
} PROBE_TERM_Q;

typedef struct  probe_term_q_data {
    PROBE_TERM_Q       Cmd;                                     /* Command line interface.                              */
    PROBE_TERM_Q       Trc;                                     /* Tracing      interface.                              */
} PROBE_TERM_Q_DATA;


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

PROBE_TERM_SM      ProbeTermSM;                                 /* Rx and Tx state machine variables.                   */

PROBE_TERM_Q_DATA  ProbeTermQ;                                  /* Queue data for the trace and command line interfaces.*/

CPU_INT08U         ProbeTermTimeoutCnt;                         /* Number of times a flag should be read until it gets  */
                                                                /* raised by uC/Probe.                                  */


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
static  void               ProbeTermCmdSM_Init(void);
#endif
#if PROBE_TERM_CFG_TRC_EN > 0
static  void               ProbeTermTrcSM_Init(void);
#endif

static  void               ProbeTermQ_Init    (PROBE_TERM_Q      *p_q,
                                               CPU_INT08U         max);

static  PROBE_TERM_Q_MSG  *ProbeTermQ_GetSlot (CPU_INT08U         msg_type);

static  void               ProbeTermQ_PutMsg  (CPU_INT08U         msg_type,
                                               PROBE_TERM_Q_MSG  *p_msg);

static  PROBE_TERM_ERR     ProbeTermQ_GetMsg  (CPU_INT08U         msg_type);

static  void               ProbeTermQ_FreeMsg (CPU_INT08U         msg_type,
                                               PROBE_TERM_Q_MSG  *p_msg);


/*
*********************************************************************************************************
*                                         CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
#if (PROBE_TERM_CFG_CMD_MAX_BUF_SIZE > PROBE_TERM_MAX_BUF_SIZE)
#error  "PROBE_TERM_CFG_CMD_MAX_BUF_SIZE cannot be greater than PROBE_TERM_MAX_BUF_SIZE"
#endif
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
#if (PROBE_TERM_CFG_TRC_MAX_BUF_SIZE > PROBE_TERM_MAX_BUF_SIZE)
#error  "PROBE_TERM_CFG_TRC_MAX_BUF_SIZE cannot be greater than PROBE_TERM_MAX_BUF_SIZE"
#endif
#endif


/*
*********************************************************************************************************
*                                          ProbeTermInit()
*
* Description : Initializes the uC/Probe terminal window module for command line and tracing purposes.
*
* Argument(s) : p_err       Pointer to variable that will receive the return error code from this function :
*
*                               PROBE_TERM_ERR_NONE           OS initialization successful.
*                               PROBE_TERM_ERR_OS_INIT_FAIL   OS objects NOT successfully initialized.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  ProbeTermInit (PROBE_TERM_ERR  *p_err)
{
    ProbeTermTimeoutCnt = PROBE_TERM_TIMEOUT_MSEC /
                          PROBE_TERM_OS_CFG_TASK_DLY_MSEC;      /* Calculate the timeout count.                         */

#if PROBE_TERM_CFG_CMD_EN > 0                                   /* Initialize the terminal for command line purposes.   */
    ProbeTermCmdSM_Init();                                      /* Reset the state machine for command line purposes.   */
    ProbeTermQ_Init(&(ProbeTermQ.Cmd), PROBE_TERM_CFG_Q_SIZE);  /* Initialize the queue    for command line purposes.   */
#endif

#if PROBE_TERM_CFG_TRC_EN > 0                                   /* Initialize the terminal for tracing purposes.        */
    ProbeTermTrcSM_Init();                                      /* Reset the state machine for tracing purposes.        */
    ProbeTermQ_Init(&(ProbeTermQ.Trc), PROBE_TERM_CFG_Q_SIZE);  /* Initialize the queue    for tracing purposes.        */
#endif

    ProbeTermOS_Init(p_err);                                    /* Initialize the OS layer by creating the tasks.       */
}


/*
*********************************************************************************************************
*                                             ProbeTermCmdSM_Init()
*
* Description : Initializes the uC/Probe terminal window module for command line interface purposes.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermInit().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
#if PROBE_TERM_CFG_CMD_EN > 0
void  ProbeTermCmdSM_Init (void)
{
                                                                /* Initialize the handshaking flags for command line.   */
    ProbeTerm.Cmd.Target.BufLen = 0;
    ProbeTerm.Cmd.Target.RTS    = DEF_FALSE;
    ProbeTerm.Cmd.Target.DS     = DEF_FALSE;
    ProbeTerm.Cmd.Target.CTS    = DEF_FALSE;

    ProbeTerm.Cmd.Probe.BufLen  = 0;
    ProbeTerm.Cmd.Probe.RTS     = DEF_FALSE;
    ProbeTerm.Cmd.Probe.DS      = DEF_FALSE;
    ProbeTerm.Cmd.Probe.CTS     = DEF_FALSE;

                                                                /* Initialize the state machine for command line.       */
    ProbeTermSM.Cmd.RxCtr       = 0;
    ProbeTermSM.Cmd.RxState     = PROBE_TERM_RX_STATE_INIT;
    ProbeTermSM.Cmd.TxCtr       = 0;
    ProbeTermSM.Cmd.TxState     = PROBE_TERM_TX_STATE_IDLE;
}
#endif


/*
*********************************************************************************************************
*                                         ProbeTermCmdPrint()
*
* Description : Function to output the command line responses back to the uC/Probe terminal window.
*
* Argument(s) : p_str        Pointer to string containing the response.
*
* Return(s)   : p_err        Pointer to variable that will receive the return error code from this function :
*
*                               PROBE_TERM_ERR_NONE           Command line response sent successfully.
*                               PROBE_TERM_ERR_MSG_EMPTY      The command line response is empty.
*                               PROBE_TERM_ERR_Q_FULL         Unable to send, the queue is full.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_CMD_EN > 0
PROBE_TERM_ERR  ProbeTermCmdPrint (CPU_CHAR  *p_str)
{
    PROBE_TERM_ERR     err;
    PROBE_TERM_Q_MSG  *p_msg;
    CPU_INT16U         offset;
    CPU_INT08U         chunk_cnt;
    CPU_INT08U         chunk_ix;
    CPU_CHAR           chunk[PROBE_TERM_CFG_BUF_SIZE];


    if (p_str == (const CPU_CHAR *)0) {
        err = PROBE_TERM_ERR_MSG_EMPTY;
    } else {
        chunk_cnt = (Str_Len(p_str) / PROBE_TERM_CFG_BUF_SIZE) + 1;

        for (chunk_ix = 0; chunk_ix < chunk_cnt; chunk_ix++) {
            offset = chunk_ix * PROBE_TERM_CFG_BUF_SIZE;
            Str_Copy_N(&chunk[0],
                       &p_str[offset],
                        PROBE_TERM_CFG_BUF_SIZE);               /* Extract a chunk.                                     */

                                                                /* Get a slot from the pool.                            */
            p_msg = ProbeTermQ_GetSlot(PROBE_TERM_Q_MSG_TYPE_CMD);

            if (p_msg != (PROBE_TERM_Q_MSG *)0) {               /* Put the message in the queue.                        */
                if (chunk_ix + 1 < chunk_cnt) {
                    p_msg->MsgLen = PROBE_TERM_CFG_BUF_SIZE;
                } else {
                    p_msg->MsgLen = Str_Len(chunk);
                }
                Str_Copy_N(&(p_msg->Msg[0]), &chunk[0], PROBE_TERM_CFG_BUF_SIZE);
                ProbeTermQ_PutMsg(PROBE_TERM_Q_MSG_TYPE_CMD, p_msg);
                ProbeTermOS_Q_CmdMsgRdy();                      /* Signal the command task to process the message.      */
                err = PROBE_TERM_ERR_NONE;
            } else {
                err = PROBE_TERM_ERR_Q_FULL;                    /* The queue is full, unable to print the message.      */
                break;
            }
        }
    }

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                       ProbeTermCmdRxTaskHandler()
*
* Description : This function implements the reception task's state machine.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermCmdTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if PROBE_TERM_CFG_CMD_EN > 0
void  ProbeTermCmdRxTaskHandler (void)
{
    switch (ProbeTermSM.Cmd.RxState) {
        case PROBE_TERM_RX_STATE_IDLE:
             break;

        case PROBE_TERM_RX_STATE_INIT:                          /* Wait for any requests from uC/Probe.                 */
             if (ProbeTerm.Cmd.Probe.RTS == DEF_TRUE) {
                 ProbeTerm.Cmd.Target.CTS = DEF_TRUE;
                 ProbeTermSM.Cmd.RxState  = PROBE_TERM_RX_STATE_WAIT4_DS;
             }
             break;

        case PROBE_TERM_RX_STATE_WAIT4_DS:                      /* Wait for uC/Probe to finish sending the data.        */
             if (ProbeTerm.Cmd.Probe.DS == DEF_TRUE) {
                 ProbeTerm.Cmd.Target.CTS = DEF_FALSE;
                                                                /* Truncate the buffer to the maximum.                  */
                 if (ProbeTerm.Cmd.Probe.BufLen <= PROBE_TERM_CFG_BUF_SIZE) {
                     ProbeTerm.Cmd.Probe.Buf[ProbeTerm.Cmd.Probe.BufLen]  = '\0';
                 } else {
                     ProbeTerm.Cmd.Probe.Buf[PROBE_TERM_CFG_BUF_SIZE - 1] = '\0';
                 }
                 if (ProbeTermAppHookRxPtr != (PROBE_TERM_APP_HOOK_RX)0) {
                     (*ProbeTermAppHookRxPtr)(&ProbeTerm.Cmd.Probe.Buf[0]);
                 }
                 ProbeTermSM.Cmd.RxState = PROBE_TERM_RX_STATE_INIT;
                 ProbeTermSM.Cmd.RxCtr   = 0;
             } else if (ProbeTermSM.Cmd.RxCtr >= ProbeTermTimeoutCnt) {
                 ProbeTermCmdSM_Init();                         /* Restart if uC/Probe takes too long to respond.       */
             } else {
                 ProbeTermSM.Cmd.RxCtr++;                       /* Increase the timeout counter.                        */
             }
             break;

        default:
             ProbeTermSM.Cmd.RxState = PROBE_TERM_RX_STATE_INIT;
             break;
    }
}
#endif


/*
*********************************************************************************************************
*                                          ProbeTermCmdTxTaskHandler()
*
* Description : This function implements the transmission task's state machine.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermCmdTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if PROBE_TERM_CFG_CMD_EN > 0
void  ProbeTermCmdTxTaskHandler (void)
{
    PROBE_TERM_ERR  err;


    switch (ProbeTermSM.Cmd.TxState) {
        case PROBE_TERM_TX_STATE_IDLE:
             ProbeTermOS_Q_CmdMsgWait();
             err = ProbeTermQ_GetMsg(PROBE_TERM_Q_MSG_TYPE_CMD);
             if (err == PROBE_TERM_ERR_NONE) {
                 ProbeTermSM.Cmd.TxState = PROBE_TERM_TX_STATE_INIT;
             }
             break;

        case PROBE_TERM_TX_STATE_INIT:                          /* Initialize the transmission of data to uC/Probe.     */
             ProbeTerm.Cmd.Target.DS  = DEF_FALSE;
             ProbeTerm.Cmd.Target.RTS = DEF_TRUE;
             ProbeTermSM.Cmd.TxState  = PROBE_TERM_TX_STATE_WAIT4_CTS;
             break;

        case PROBE_TERM_TX_STATE_WAIT4_CTS:                     /* Wait for uC/Probe to be ready to receive data.       */
             if (ProbeTerm.Cmd.Probe.CTS == DEF_TRUE) {
                 ProbeTerm.Cmd.Target.RTS  = DEF_FALSE;
                 ProbeTerm.Cmd.Target.DS   = DEF_TRUE;
                 ProbeTermSM.Cmd.TxCtr     = 0;
                 ProbeTermSM.Cmd.TxState   = PROBE_TERM_TX_STATE_IDLE;
             } else if (ProbeTermSM.Cmd.TxCtr >= ProbeTermTimeoutCnt) {
                    ProbeTermCmdSM_Init();                      /* Restart if uC/Probe takes too long to respond.       */
             } else {
                 ProbeTermSM.Cmd.TxCtr++;                       /* Increase the timeout counter.                        */
             }
             break;

        default:
             ProbeTermSM.Cmd.TxState = PROBE_TERM_TX_STATE_IDLE;
             break;
    }
}
#endif


/*
*********************************************************************************************************
*                                         ProbeTermTrcSM_Init()
*
* Description : Initializes the uC/Probe terminal window state machine for tracing purposes.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermInit().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
#if PROBE_TERM_CFG_TRC_EN > 0
void  ProbeTermTrcSM_Init (void)
{
                                                                /* Initialize the handshaking flags for tracing.        */
    ProbeTerm.Trc.Target.BufLen = 0;
    ProbeTerm.Trc.Target.RTS    = DEF_FALSE;
    ProbeTerm.Trc.Target.DS     = DEF_FALSE;

    ProbeTerm.Trc.Probe.CTS     = DEF_FALSE;

                                                                /* Initialize the state machine for tracing purposes.   */
    ProbeTermSM.Trc.TxCtr       = 0;
    ProbeTermSM.Trc.TxState     = PROBE_TERM_TX_STATE_IDLE;
}
#endif


/*
*********************************************************************************************************
*                                        ProbeTermTrcPrint()
*
* Description : Function to output trace events to the uC/Probe terminal window.
*
* Argument(s) : p_str        Pointer to string containing the trace event information.
*
* Return(s)   :              The error code from this function :
*
*                               PROBE_TERM_ERR_NONE           Command line response sent successfully.
*                               PROBE_TERM_ERR_MSG_EMPTY      The command line response is empty.
*                               PROBE_TERM_ERR_Q_FULL         Unable to send, the queue is full.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_TRC_EN > 0
PROBE_TERM_ERR  ProbeTermTrcPrint (CPU_CHAR  *p_str)
{
    PROBE_TERM_ERR     err;
    PROBE_TERM_Q_MSG  *p_msg;
    CPU_INT16U         offset;
    CPU_INT08U         chunk_cnt;
    CPU_INT08U         chunk_ix;
    CPU_CHAR           chunk[PROBE_TERM_CFG_BUF_SIZE];


    if (p_str == (const CPU_CHAR *)0) {
        err = PROBE_TERM_ERR_MSG_EMPTY;
    } else {
                                                                /* Calculate number of chunks.                          */
        chunk_cnt = (Str_Len(p_str) / PROBE_TERM_CFG_BUF_SIZE) + 1;

        for (chunk_ix = 0; chunk_ix < chunk_cnt; chunk_ix++) {
            offset = chunk_ix * PROBE_TERM_CFG_BUF_SIZE;
            Str_Copy_N(&chunk[0],
                       &p_str[offset],
                        PROBE_TERM_CFG_BUF_SIZE - 1);           /* Extract the chunk.                                   */

                                                                /* Get a slot from the queue.                           */
            p_msg = ProbeTermQ_GetSlot(PROBE_TERM_Q_MSG_TYPE_TRC);

            if (p_msg != (PROBE_TERM_Q_MSG *)0) {               /* Put the message in the queue.                        */
                p_msg->MsgLen = Str_Len(chunk);
                Str_Copy_N(&(p_msg->Msg[0]), &chunk[0], PROBE_TERM_CFG_BUF_SIZE);
                ProbeTermQ_PutMsg(PROBE_TERM_Q_MSG_TYPE_TRC, p_msg);
                ProbeTermOS_Q_TrcMsgRdy();                      /* Signal the trace task to process the message.        */
                err = PROBE_TERM_ERR_NONE;
            } else {
                err = PROBE_TERM_ERR_Q_FULL;                    /* The queue is full, unable to print the message.      */
                break;
            }
        }
    }

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeTermTrcTaskHandler()
*
* Description : This function implements the transmission task's state machine.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermOS_TrcTask() in the OS layer.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if PROBE_TERM_CFG_TRC_EN > 0
void  ProbeTermTrcTaskHandler (void)
{
    PROBE_TERM_ERR  err;


    switch (ProbeTermSM.Trc.TxState) {
        case PROBE_TERM_TX_STATE_IDLE:                          /* Wait until a trace message is put in the queue.      */
             ProbeTermOS_Q_TrcMsgWait();
             err = ProbeTermQ_GetMsg(PROBE_TERM_Q_MSG_TYPE_TRC);
             if (err == PROBE_TERM_ERR_NONE) {
                 ProbeTermSM.Trc.TxState = PROBE_TERM_TX_STATE_INIT;
             }
             break;

        case PROBE_TERM_TX_STATE_INIT:                          /* Initialize the transmission of data to uC/Probe.     */
             ProbeTerm.Trc.Target.DS  = DEF_FALSE;
             ProbeTerm.Trc.Target.RTS = DEF_TRUE;
             ProbeTermSM.Trc.TxState  = PROBE_TERM_TX_STATE_WAIT4_CTS;
             break;

        case PROBE_TERM_TX_STATE_WAIT4_CTS:                     /* Wait for uC/Probe to be ready to receive data.       */
             if (ProbeTerm.Trc.Probe.CTS == DEF_TRUE) {
                 ProbeTerm.Trc.Target.RTS = DEF_FALSE;
                 ProbeTerm.Trc.Target.DS  = DEF_TRUE;
                 ProbeTermSM.Trc.TxCtr    = 0;
                 ProbeTermSM.Trc.TxState  = PROBE_TERM_TX_STATE_IDLE;
             } else if (ProbeTermSM.Trc.TxCtr >= ProbeTermTimeoutCnt) {
                 ProbeTermTrcSM_Init();                         /* Restart if uC/Probe takes too long to respond.       */
             } else {
                 ProbeTermSM.Trc.TxCtr++;                       /* Increase the timeout counter.                        */
             }
             break;

        default:
             ProbeTermSM.Trc.TxState = PROBE_TERM_TX_STATE_IDLE;
             break;
    }
}
#endif


/*
*********************************************************************************************************
*                                         ProbeTermQ_Init()
*
* Description : Initializes the uC/Probe queues.
*
* Argument(s) : p_q        Pointer to the queue that needs to be initialized.
*               max        Maximum size of the queue.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermInit().
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  ProbeTermQ_Init (PROBE_TERM_Q *p_q, CPU_INT08U max)
{
    PROBE_TERM_Q_MSG  *p_msg;
    CPU_INT08U         ix;

                                                                /* Initialize the queue.                                */
    for (ix = 0u; ix < (max - 1u); ix++) {
        p_msg          = &(p_q->Q[ix]);
        p_msg->NextPtr = &(p_q->Q[ix + 1u]);
    }

    p_msg          = &(p_q->Q[ix]);
    p_msg->NextPtr =  (PROBE_TERM_Q_MSG *)0;
    p_q->MsgCtr    =   0u;
    p_q->FreePtr   = &(p_q->Q[0]);
}


/*
*********************************************************************************************************
*                                       ProbeTermQ_GetRef()
*
* Description : Get a reference to the queue.
*
* Argument(s) : msg_type   Type of message queue, for command line or trace purposes.
*
* Return(s)   : Pointer to message object, if NO error(s).
*
*               Pointer to NULL,           otherwise.
*
* Caller(s)   : ProbeTermTrcPrint(),
*               ProbeTermCmdPrint().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  PROBE_TERM_Q  *ProbeTermQ_GetRef (CPU_INT08U msg_type)
{
    PROBE_TERM_Q  *p_q;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    switch (msg_type) {
#if PROBE_TERM_CFG_TRC_EN > 0
        case PROBE_TERM_Q_MSG_TYPE_TRC:
             p_q = &(ProbeTermQ.Trc);
             break;
#endif
#if PROBE_TERM_CFG_CMD_EN > 0
        case PROBE_TERM_Q_MSG_TYPE_CMD:
             p_q = &(ProbeTermQ.Cmd);
             break;
#endif
        default:
             return ((PROBE_TERM_Q *)0);
    }
    CPU_CRITICAL_EXIT();

    return (p_q);
}


/*
*********************************************************************************************************
*                                       ProbeTermQ_GetSlot()
*
* Description : Get a slot from the pool of available message slots.
*
* Argument(s) : msg_type   Type of message slot, for command line or trace purposes.
*
* Return(s)   : Pointer to message object, if NO error(s).
*
*               Pointer to NULL,           otherwise.
*
* Caller(s)   : ProbeTermTrcPrint(),
*               ProbeTermCmdPrint().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  PROBE_TERM_Q_MSG  *ProbeTermQ_GetSlot (CPU_INT08U msg_type)
{
    PROBE_TERM_Q      *p_q;
    PROBE_TERM_Q_MSG  *p_msg;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    p_q = ProbeTermQ_GetRef(msg_type);

    if (p_q->FreePtr == (PROBE_TERM_Q_MSG *)0) {
        CPU_CRITICAL_EXIT();
        return ((PROBE_TERM_Q_MSG *)0);
    }

    p_msg        = p_q->FreePtr;
    p_q->FreePtr = p_q->FreePtr->NextPtr;
    p_q->MsgCtr++;
    CPU_CRITICAL_EXIT();

    p_msg->NextPtr = (PROBE_TERM_Q_MSG *)0;

    return (p_msg);
}


/*
*********************************************************************************************************
*                                         ProbeTermQ_PutMsg()
*
* Description : Enqueue a message object.
*
* Argument(s) : msg_type   Type of message to enqueue, for command line or trace purposes.
*               p_msg      Pointer to the message that needs to be enqueued.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermTrcPrint(),
*               ProbeTermCmdPrint().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeTermQ_PutMsg (CPU_INT08U         msg_type,
                                 PROBE_TERM_Q_MSG  *p_msg)
{
    PROBE_TERM_Q  *p_q;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    p_q = ProbeTermQ_GetRef(msg_type);

    if (p_q->HeadPtr == (PROBE_TERM_Q_MSG *)0) {
        p_q->HeadPtr          = p_msg;
    } else {
        p_q->TailPtr->NextPtr = p_msg;
    }
    p_q->TailPtr = p_msg;
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*                                         ProbeTermQ_FreeMsg()
*
* Description : Returns a message slot to the pool.
*
* Argument(s) : msg_type   Type of message in the queue, for command line or trace purposes.
*               p_msg      Pointer to the message object that needs to be freed.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeTermTrcTask(),
*               ProbeTermCmdTask().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeTermQ_FreeMsg (CPU_INT08U         msg_type,
                                  PROBE_TERM_Q_MSG  *p_msg)
{
    PROBE_TERM_Q  *p_q;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    p_q = ProbeTermQ_GetRef(msg_type);

    if (p_q->FreePtr == (PROBE_TERM_Q_MSG *)0) {
        p_msg->NextPtr = (PROBE_TERM_Q_MSG *)0;
    } else {
        p_msg->NextPtr = p_q->FreePtr;
    }
    p_q->FreePtr = p_msg;
    p_q->MsgCtr--;
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*                                         ProbeTermQ_GetMsg()
*
* Description : Returns a message slot to the pool.
*
* Argument(s) : msg_type   Type of message to return, for command line or trace purposes.
*
* Return(s)   :            The error code from this function :
*
*                               PROBE_TERM_ERR_NONE              Command line response sent successfully.
*                               PROBE_TERM_ERR_MSG_TYPE_INVALID  The message type is invalid.
*                               PROBE_TERM_ERR_Q_EMPTY           The queue is empty.
*
* Caller(s)   : ProbeTermTrcTask(),
*               ProbeTermCmdTask().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  PROBE_TERM_ERR  ProbeTermQ_GetMsg (CPU_INT08U  msg_type)
{
    PROBE_TERM_Q      *p_q;
    PROBE_TERM_Q_MSG  *p_msg;
    PROBE_TERM_ERR     err;
    CPU_SR_ALLOC();


    err = PROBE_TERM_ERR_NONE;

    CPU_CRITICAL_ENTER();
    p_q = ProbeTermQ_GetRef(msg_type);

    p_msg = p_q->HeadPtr;                                       /* Extract the message from the queue (FIFO).           */

    if (p_q->HeadPtr != (PROBE_TERM_Q_MSG *)0) {
        p_q->HeadPtr = p_msg->NextPtr;
    }
    CPU_CRITICAL_EXIT();

    if (p_msg != (PROBE_TERM_Q_MSG *)0) {
        switch (msg_type) {
#if PROBE_TERM_CFG_TRC_EN > 0
            case PROBE_TERM_Q_MSG_TYPE_TRC:
                 Str_Copy_N(&ProbeTerm.Trc.Target.Buf[0], &(p_msg->Msg[0]), p_msg->MsgLen);
                 ProbeTerm.Trc.Target.BufLen = p_msg->MsgLen;
                 break;
#endif
#if PROBE_TERM_CFG_CMD_EN > 0
            case PROBE_TERM_Q_MSG_TYPE_CMD:
                 Str_Copy_N(&ProbeTerm.Cmd.Target.Buf[0], &(p_msg->Msg[0]), p_msg->MsgLen);
                 ProbeTerm.Cmd.Target.BufLen = p_msg->MsgLen;
                 break;
#endif

            default:
                 err = PROBE_TERM_ERR_MSG_TYPE_INVALID;
                 break;
        }
        ProbeTermQ_FreeMsg(msg_type, p_msg);                    /* Return slot to the pool.                             */
    } else {
        err = PROBE_TERM_ERR_Q_EMPTY;                           /* No message to return, the queue is empty.            */
    }

    return (err);
}
#endif
#endif