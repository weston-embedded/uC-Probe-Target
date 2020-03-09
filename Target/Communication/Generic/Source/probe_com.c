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
*                                       COMMUNICATION: GENERIC
*
* Filename : probe_com.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) This file contains code to respond to generic (non protocol-dependent) commands
*                received by the target.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   PROBE_COM_MODULE
#include  <probe_com.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        DATA FORMATS DEFINES
*********************************************************************************************************
*/

#define  PROBE_COM_FMT_TX_ERR                         0x8000u

#define  PROBE_COM_FMT_RX_QUERY                       0x0001u   /* Query setup parameter or capability.                 */
#define  PROBE_COM_FMT_TX_QUERY                       0x8001u

#define  PROBE_COM_FMT_RX_RD                          0x0002u   /* Read data at memory address.                         */
#define  PROBE_COM_FMT_TX_RD                          0x8002u

#define  PROBE_COM_FMT_RX_WR                          0x0003u   /* Write data at memory address.                        */
#define  PROBE_COM_FMT_TX_WR                          0x8003u

#define  PROBE_COM_FMT_RX_RD_MULTI                    0x0007u   /* Read data at multiple memory addresses.              */
#define  PROBE_COM_FMT_TX_RD_MULTI                    0x8007u
#define  PROBE_COM_FMT_TX_RD_MULTI_LO                   0x07u
#define  PROBE_COM_FMT_TX_RD_MULTI_HI                   0x80u

#define  PROBE_COM_FMT_RX_WR_MULTI                    0x0008u   /* Write data at multiple memory addresses.             */
#define  PROBE_COM_FMT_TX_WR_MULTI                    0x8008u

#define  PROBE_COM_FMT_RX_STR_OUT                     0x0009u   /* Transmit output string to Probe.                     */
#define  PROBE_COM_FMT_TX_STR_OUT                     0x8009u

#define  PROBE_COM_FMT_RX_STR_IN                      0x000Au   /* Receive input string from Probe.                     */
#define  PROBE_COM_FMT_TX_STR_IN                      0x800Au

#define  PROBE_COM_FMT_RX_TERMINAL_EXEC               0x000Bu   /* Execute terminal command.                            */
#define  PROBE_COM_FMT_TX_TERMINAL_EXEC               0x800Bu

#define  PROBE_COM_FMT_RX_TERMINAL_OUT                0x000Cu   /* Transmit output data to Probe.                       */
#define  PROBE_COM_FMT_TX_TERMINAL_OUT                0x800Cu

#define  PROBE_COM_FMT_RX_TERMINAL_IN                 0x000Du   /* Receive input data from Probe.                       */
#define  PROBE_COM_FMT_TX_TERMINAL_IN                 0x800Du

/*
*********************************************************************************************************
*                                      STATUS CONSTANTS DEFINES
*********************************************************************************************************
*/

#define  PROBE_COM_STATUS_OK                            0x01u
#define  PROBE_COM_STATUS_TERMINAL_EXEC_NOT_RDY         0xF4u   /* Terminal not rdy to exec new cmd.                    */
#define  PROBE_COM_STATUS_TERMINAL_IN_OVF               0xF5u   /* Terminal in buf ovf'd.                               */
#define  PROBE_COM_STATUS_TERMINAL_OUT_NONE             0xF6u   /* Terminal out buf empty.                              */
#define  PROBE_COM_STATUS_STR_IN_OVF                    0xF7u   /* String in buf ovf'd.                                 */
#define  PROBE_COM_STATUS_STR_OUT_NONE                  0xF8u   /* String out buf empty.                                */
#define  PROBE_COM_STATUS_UNKNOWN_REQUEST               0xF9u   /* Unknown req.                                         */
#define  PROBE_COM_STATUS_QUERY_NOT_SUPPORTED           0xFCu   /* Query not supported.                                 */
#define  PROBE_COM_STATUS_TX_PKT_TOO_LARGE              0xFDu   /* Reply pkt too large for buf.                         */
#define  PROBE_COM_STATUS_RX_PKT_WRONG_SIZE             0xFEu   /* Req pkt not correct size.                            */
#define  PROBE_COM_STATUS_FAIL                          0xFFu   /* Req exec failed (or unknown err).                    */

/*
*********************************************************************************************************
*                                           QUERIES DEFINES
*********************************************************************************************************
*/

                                                                /* ------------------- CONFIGURATION ------------------ */
#define  PROBE_COM_QUERY_MAX_RX_SIZE                  0x0101u   /* Get data size of largest pkt target can rx.          */
#define  PROBE_COM_QUERY_MAX_TX_SIZE                  0x0102u   /* Get data size of largest pkt target can tx.          */

                                                                /* ----------------- TARGET PROPERTIES ---------------- */
#define  PROBE_COM_QUERY_ENDIANNESS_TEST              0x0201u   /* Test endianness of target.                           */
#define  PROBE_COM_QUERY_STATUS                       0x0202u   /* Target status.                                       */

                                                                /* ------------- COMMUNICATION CAPABILITIES ----------- */
#define  PROBE_COM_QUERY_FMT_SUPPORT                  0x1001u   /* Get list of supported req's.                         */
#define  PROBE_COM_QUERY_VERSION                      0x1002u   /* Get target code ver.                                 */

/*
*********************************************************************************************************
*                                          MODIFIERS DEFINES
*********************************************************************************************************
*/

#define  PROBE_COM_MODIFIER_NONE                  DEF_BIT_NONE
#define  PROBE_COM_MODIFIER_STR_OUT_AVAIL         DEF_BIT_00    /* Str out is avail.                                    */
#define  PROBE_COM_MODIFIER_TERMINAL_EXEC_DONE    DEF_BIT_01    /* Terminal cmd exec done.                              */
#define  PROBE_COM_MODIFIER_TERMINAL_OUT_AVAIL    DEF_BIT_02    /* Terminal out is avail.                               */

/*
*********************************************************************************************************
*                                        HEADER SIZES DEFINES
*
* Note(s):  (1) The first four bytes in all tx data segments is identical:
*
*               (a) A 2-byte format;
*               (b) A 1-byte status;
*               (c) A 1-byte modifier, currently unused.
*
*           (2) The first two bytes in all rx data segments is identical:
*
*               (a) A 2-byte format.
*********************************************************************************************************
*/

#define  PROBE_COM_SIZE_RX_HDR                             2u
#define  PROBE_COM_SIZE_TX_HDR                             4u

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

typedef  struct  probe_com_buf {
    CPU_SIZE_T   Len;
    CPU_SIZE_T   IxRd;
    CPU_SIZE_T   IxWr;
    CPU_INT08U  *DataPtr;
} PROBE_COM_BUF;


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

#if (PROBE_COM_CFG_STR_REQ_EN       == DEF_ENABLED)
static  CPU_CHAR                       ProbeCom_StrInBufData[PROBE_COM_CFG_STR_IN_BUF_SIZE];
static  PROBE_COM_BUF                  ProbeCom_StrInBuf;

static  CPU_CHAR                       ProbeCom_StrOutBufData[PROBE_COM_CFG_STR_OUT_BUF_SIZE];
static  PROBE_COM_BUF                  ProbeCom_StrOutBuf;
#endif

#if (PROBE_COM_CFG_TERMINAL_REQ_EN  == DEF_ENABLED)
static  PROBE_COM_TERMINAL_EXEC_FNCT   ProbeCom_TerminalExecHandler;
static  CPU_BOOLEAN                    ProbeCom_TerminalExecuting;

static  PROBE_COM_TERMINAL_IN_FNCT     ProbeCom_TerminalInHandler;

static  CPU_INT08U                    *ProbeCom_TerminalOutBufPtr;
static  CPU_SIZE_T                     ProbeCom_TerminalOutBufIx;
static  CPU_SIZE_T                     ProbeCom_TerminalOutBufLen;
#endif

static  CPU_INT32U                     ProbeCom_EndiannessTest;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_INT08U   ProbeCom_PktModifier     (void);

static  CPU_SIZE_T   ProbeCom_ReqErr          (CPU_INT08U      *ptx_buf,
                                               CPU_INT08U       pcomm_err);


                                                                /* ------------------- HANDLE REQ'S ------------------- */
static  CPU_SIZE_T   ProbeCom_ReqQuery        (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqRd           (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqRdMulti      (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

#if (PROBE_COM_CFG_WR_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T   ProbeCom_ReqWr           (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqWrMulti      (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);
#endif

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T   ProbeCom_ReqStrIn        (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqStrOut       (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);
#endif

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T   ProbeCom_ReqTerminalExec (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqTerminalIn   (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);

static  CPU_SIZE_T   ProbeCom_ReqTerminalOut  (CPU_INT08U      *prx_buf,
                                               CPU_INT08U      *ptx_buf,
                                               CPU_SIZE_T       rx_pkt_size,
                                               CPU_SIZE_T       tx_buf_size);
#endif

                                                                /* ------------------- DYNAMIC BUFFER ----------------- */
#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  void         ProbeCom_BufInit         (PROBE_COM_BUF   *pbuf,
                                               CPU_INT08U      *pdata,
                                               CPU_SIZE_T       len);

static  CPU_SIZE_T   ProbeCom_BufRd           (PROBE_COM_BUF   *pbuf,
                                               CPU_INT08U      *pdest,
                                               CPU_SIZE_T       len)
;
static  CPU_SIZE_T   ProbeCom_BufWr           (PROBE_COM_BUF   *pbuf,
                                               CPU_INT08U      *psrc,
                                               CPU_SIZE_T       len);

static  CPU_BOOLEAN  ProbeCom_BufIsEmpty      (PROBE_COM_BUF   *pbuf);
#endif

#if 0
static  CPU_BOOLEAN  ProbeCom_BufIsFull       (PROBE_COM_BUF   *pbuf);
#endif

                                                                /* ------------------- STATIC BUFFER ------------------ */
static  CPU_INT08U   ProbeCom_GetINT08U       (CPU_INT08U     **pbuf);

static  CPU_INT16U   ProbeCom_GetINT16U       (CPU_INT08U     **pbuf);

static  CPU_INT32U   ProbeCom_GetINT32U       (CPU_INT08U     **pbuf);

static  void         ProbeCom_StoINT08U       (CPU_INT08U     **pbuf,
                                               CPU_INT08U       data);

static  void         ProbeCom_StoINT16U       (CPU_INT08U     **pbuf,
                                               CPU_INT16U       data);

#if 0
static  void         ProbeCom_StoINT32U       (CPU_INT08U     **pbuf,
                                               CPU_INT32U       data);
#endif

                                                                /* -------------- DETERMINE PKT MODIFIER -------------- */
#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
static  CPU_BOOLEAN  ProbeCom_StrOutAvail     (void);
#endif


#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_BOOLEAN  ProbeCom_TerminalOutAvail(void);

static  CPU_BOOLEAN  ProbeCom_TerminalExecDone(void);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           ProbeCom_Init()
*
* Description : Initialize the module.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if initialization successful.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  ProbeCom_Init (void)
{
#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
    CPU_BOOLEAN  ok;
#endif


#if (PROBE_COM_CFG_STR_REQ_EN      == DEF_ENABLED)
    Mem_Clr((void     *)&ProbeCom_StrInBufData[0],
            (CPU_SIZE_T) PROBE_COM_CFG_STR_IN_BUF_SIZE);

    ProbeCom_BufInit((PROBE_COM_BUF *)&ProbeCom_StrInBuf,
                     (CPU_INT08U    *)&ProbeCom_StrInBufData[0],
                     (CPU_SIZE_T     ) PROBE_COM_CFG_STR_IN_BUF_SIZE);

    Mem_Clr((void     *)&ProbeCom_StrOutBufData[0],
            (CPU_SIZE_T) PROBE_COM_CFG_STR_OUT_BUF_SIZE);

    ProbeCom_BufInit((PROBE_COM_BUF *)&ProbeCom_StrOutBuf,
                     (CPU_INT08U    *)&ProbeCom_StrOutBufData[0],
                     (CPU_SIZE_T     ) PROBE_COM_CFG_STR_OUT_BUF_SIZE);
#endif


#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
    ProbeCom_TerminalExecHandler   = (void        *)0;
    ProbeCom_TerminalExecuting     = (CPU_BOOLEAN  )DEF_NO;

    ProbeCom_TerminalInHandler     = (void        *)0;

    ProbeCom_TerminalOutBufPtr     = (CPU_INT08U  *)0;
    ProbeCom_TerminalOutBufIx      = (CPU_SIZE_T   )0;
    ProbeCom_TerminalOutBufLen     = (CPU_SIZE_T   )0;

    ok = ProbeCom_OS_Init();

    if (ok == DEF_FAIL) {
        return (DEF_FAIL);
    }
#endif


#if (PROBE_COM_CFG_STAT_EN         == DEF_ENABLED)
    ProbeCom_RxPktCtr              =  0u;
    ProbeCom_TxPktCtr              =  0u;

    ProbeCom_TxSymCtr              =  0u;
    ProbeCom_TxSymByteCtr          =  0u;
    ProbeCom_ErrPktCtr             =  0u;

#if (PROBE_COM_CFG_STR_REQ_EN      == DEF_ENABLED)
    ProbeCom_StrRxCtr              =  0u;
    ProbeCom_StrRxOvfErrCtr        =  0u;

    ProbeCom_StrTxCtr              =  0u;
    ProbeCom_StrTxOvfErrCtr        =  0u;
#endif

#if (PROBE_COM_CFG_WR_REQ_EN       == DEF_ENABLED)
    ProbeCom_RxSymCtr              =  0u;
    ProbeCom_RxSymByteCtr          =  0u;
#endif
#endif

    ProbeCom_EndiannessTest        =  0x12345678uL;

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                          ProbeCom_StrRd()
*
* Description : Read input data.
*
* Argument(s) : pdest       Pointer to the destination buffer.
*
*               len         Length of the destination buffer, in octets/characters.
*
* Return(s)   : Number of octets/characters read.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function implements a non-blocking read.  It will read as much data as fits
*                   into the buffer, up to 'len' bytes/characters.  The calling application should
*                   monitor the return value to see if more data needs to be read.
*
*               (2) Since this function never blocks, it should not be called in a tight loop without a
*                   delay.
*
*               (3) This function MAY be called from an ISR.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
CPU_SIZE_T  ProbeCom_StrRd (CPU_CHAR    *pdest,
                            CPU_SIZE_T   len)
{
    CPU_SIZE_T  len_rd;


    len_rd = ProbeCom_BufRd((PROBE_COM_BUF *)&ProbeCom_StrInBuf,
                            (CPU_INT08U    *) pdest,
                            (CPU_SIZE_T     ) len);

    return (len_rd);
}
#endif


/*
*********************************************************************************************************
*                                          ProbeCom_StrWr()
*
* Description : Write or buffer data for output.
*
* Argument(s) : psrc        Pointer to the source buffer.
*
*               len         Length of the source buffer, in octets/characters.
*
* Return(s)   : Number of octets/characters written or buffered.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function implements a non-blocking write.  It will write as much data as fits
*                   into the buffer, up to 'len' bytes/characters.  The calling application should
*                   monitor the return value to see if more data from the buffer needs to be written.
*
*               (2) Since this function never blocks, it should not be called in a tight loop without a
*                   delay.
*
*               (3) This function MAY be called from an ISR.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
CPU_SIZE_T  ProbeCom_StrWr (CPU_CHAR    *psrc,
                            CPU_SIZE_T   len)
{
    CPU_SIZE_T  len_wr;


    len_wr = ProbeCom_BufWr((PROBE_COM_BUF *)&ProbeCom_StrOutBuf,
                            (CPU_INT08U    *) psrc,
                            (CPU_SIZE_T     ) len);

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    if (len_wr < len) {
        ProbeCom_StrTxOvfErrCtr++;
    }
#endif

    return (len_wr);
}
#endif


/*
*********************************************************************************************************
*                                       ProbeCom_TerminalOut()
*
* Description : Output data over terminal.
*
* Argument(s) : psrc        Pointer to the source buffer.
*
*               len         Length of source buffer, in octets/characters.
*
* Return(s)   : Number of octets/characters output.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function implements a blocking write.  It will queue the request and wait until
*                   all of the data has been buffered or transmitted before returning.
*
*               (2) Terminal data may ONLY be output while a command is being executed.
*
*                   (a) Generic read/write functionality is provided by the string read/write functions
*                       (see 'ProbeCom_StrRd()', 'ProbeCom_StrWr()').
*
*               (3) This function MUST NOT be called from an ISR.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
CPU_SIZE_T  ProbeCom_TerminalOut (CPU_CHAR    *psrc,
                                  CPU_SIZE_T   len)
{
    CPU_SR_ALLOC();


                                                                /* ------------------- VALIDATE ARGS ------------------ */
    if (psrc == (CPU_CHAR *)0) {                                /* Validate NULL ptr.                                   */
        return ((CPU_SIZE_T)0);
    }

    if (len == 0u) {                                            /* Validate NULL len.                                   */
        return ((CPU_SIZE_T)0);
    }



                                                                /* ------------------- START OUTPUT ------------------- */
    CPU_CRITICAL_ENTER();
    if (ProbeCom_TerminalExecuting == DEF_NO) {                 /* Chk if cmd exec'ing.                                 */
        CPU_CRITICAL_EXIT();
        return ((CPU_SIZE_T)0);
    }

    if (ProbeCom_TerminalOutBufPtr != (CPU_INT08U *)0) {        /* Chk if out in progress.                              */
        CPU_CRITICAL_EXIT();
        return ((CPU_SIZE_T)0);
    }

    ProbeCom_TerminalOutBufPtr = (CPU_INT08U *)psrc;            /* Schedule output.                                     */
    ProbeCom_TerminalOutBufIx  = (CPU_SIZE_T  )0;
    ProbeCom_TerminalOutBufLen = (CPU_SIZE_T  )len;

    ProbeCom_OS_TerminalOutWait();                              /* Wait for output completion.                          */

    ProbeCom_TerminalOutBufPtr = (CPU_INT08U *)0;
    ProbeCom_TerminalOutBufIx  = (CPU_SIZE_T  )0;
    ProbeCom_TerminalOutBufLen = (CPU_SIZE_T  )0;
    CPU_CRITICAL_EXIT();



                                                                /* ----------------------- RTN ------------------------ */
    return (len);
}
#endif


/*
*********************************************************************************************************
*                                   ProbeCom_TerminalExecComplete()
*
* Description : Signal completion of command execution.
*
* Argument(s) : none.
*
* Return(s)   : The number of bytes in the data segment of the packet to transmit in response.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
void  ProbeCom_TerminalExecComplete (void)
{
    CPU_BOOLEAN  executing;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    executing = ProbeCom_TerminalExecuting;
    if (executing == DEF_YES) {
        ProbeCom_TerminalExecuting = DEF_NO;
    }
    CPU_CRITICAL_EXIT();
}
#endif


/*
*********************************************************************************************************
*                                     ProbeCom_TerminalExecSet()
*
* Description : Set the handler that will be invoked to process a terminal command.
*
* Argument(s) : handler         The handler that will be invoked.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The handler should be a function with the following prototype:
*
*                       void  App_TerminalExecFnct (CPU_CHAR    *pstr,
*                                                   CPU_SIZE_T   len);
*
*                   where 'pstr' is a pointer to the command string, and 'len' is the length of the
*                   command string (in characters) excluding the final NULL byte.  The command string
*                   will NOT include a terminating new line or line feed.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
void  ProbeCom_TerminalExecSet (PROBE_COM_TERMINAL_EXEC_FNCT  handler)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    ProbeCom_TerminalExecHandler = handler;
    CPU_CRITICAL_EXIT();
}
#endif


/*
*********************************************************************************************************
*                                      ProbeCom_TerminalInSet()
*
* Description : Set the handler that will be invoked to process terminal input.
*
* Argument(s) : handler         The handler that will be invoked.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The handler should be a function with the following prototype:
*
*                       void  App_TerminalInFnct (CPU_CHAR    *pstr,
*                                                 CPU_SIZE_T   len);
*
*                   where 'pstr' is a pointer to the input string, and 'len' is the length of the input
*                   string (in characters) excluding the final NULL byte.  The input string will NOT
*                   include a terminating new line or line feed.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
void  ProbeCom_TerminalInSet (PROBE_COM_TERMINAL_IN_FNCT  handler)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr;
#endif


    CPU_CRITICAL_ENTER();
    ProbeCom_TerminalInHandler = handler;
    CPU_CRITICAL_EXIT();
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_ParseRxPkt()
*
* Description : Parse a packet & formulate a response.
*
* Argument(s) : prx_pkt         Pointer to the receive  packet buffer
*
*               ptx_pkt         Pointer to the transmit packet buffer
*
*               rx_pkt_size     Size of the received packet
*
*               tx_pkt_size     Size of the transmit packet buffer
*
* Return(s)   : The number of bytes in the data segment of the packet to transmit in response.
*
* Caller(s)   : Tasks/receive handlers in communications-specific drivers, (e.g., probe_rs232,
*               probe_usb, probe_tcpip, etc.).
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_SIZE_T  ProbeCom_ParseRxPkt (void        *prx_pkt,
                                 void        *ptx_pkt,
                                 CPU_SIZE_T   rx_pkt_size,
                                 CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T   tx_buf_wr;
    CPU_INT16U   format;
    CPU_INT08U  *prx_pkt_08;
    CPU_INT08U  *ptx_pkt_08;


    if (rx_pkt_size < 2u) {
        return (0);
    }

    prx_pkt_08  = (CPU_INT08U *)prx_pkt;
    ptx_pkt_08  = (CPU_INT08U *)ptx_pkt;
    format      = (prx_pkt_08[1] << 8) + prx_pkt_08[0];
    prx_pkt_08 += 2u;

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_RxPktCtr++;
    ProbeCom_TxPktCtr++;
#endif

    switch (format) {
        case PROBE_COM_FMT_RX_QUERY:
             tx_buf_wr = ProbeCom_ReqQuery(       prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_RD:
             tx_buf_wr = ProbeCom_ReqRd(          prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_RD_MULTI:
             tx_buf_wr = ProbeCom_ReqRdMulti(     prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

#if (PROBE_COM_CFG_WR_REQ_EN == DEF_ENABLED)
        case PROBE_COM_FMT_RX_WR:
             tx_buf_wr = ProbeCom_ReqWr(          prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_WR_MULTI:
             tx_buf_wr = ProbeCom_ReqWrMulti(     prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;
#endif

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
        case PROBE_COM_FMT_RX_STR_IN:
             tx_buf_wr = ProbeCom_ReqStrIn(       prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_STR_OUT:
             tx_buf_wr = ProbeCom_ReqStrOut(      prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;
#endif

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
        case PROBE_COM_FMT_RX_TERMINAL_EXEC:
             tx_buf_wr = ProbeCom_ReqTerminalExec(prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_TERMINAL_IN:
             tx_buf_wr = ProbeCom_ReqTerminalIn(  prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_TERMINAL_OUT:
             tx_buf_wr = ProbeCom_ReqTerminalOut( prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;
#endif

        default:
             tx_buf_wr = ProbeCom_ReqErr(                     ptx_pkt_08, PROBE_COM_STATUS_UNKNOWN_REQUEST);
             break;
    }

    return (tx_buf_wr);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       ProbeCom_StrOutAvail()
*
* Description : Check if string output is waiting for transmission.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE   if a  string output is waiting for transmission.
*               DEF_FALSE  if no string output is waiting for transmission.
*
* Caller(s)   : ProbeCom_PktModifier().
*
* Note(s)     : (1) See Notes for 'ProbeCom_StrWr()'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
static  CPU_BOOLEAN  ProbeCom_StrOutAvail (void)
{
    CPU_BOOLEAN  avail;
    CPU_BOOLEAN  empty;


    empty = ProbeCom_BufIsEmpty(&ProbeCom_StrOutBuf);
    avail = (empty == DEF_YES) ? (DEF_FALSE) : (DEF_TRUE);

    return (avail);
}
#endif



/*
*********************************************************************************************************
*                                     ProbeCom_TerminalOutAvail()
*
* Description : Check if terminal output is waiting for transmission.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE   if a  terminal output is waiting for transmission.
*               DEF_FALSE  if no terminal output is waiting for transmission.
*
* Caller(s)   : ProbeCom_PktModifier().
*
* Note(s)     : (1) See Notes for 'ProbeCom_TerminalOut()'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_BOOLEAN  ProbeCom_TerminalOutAvail (void)
{
    CPU_SIZE_T   buf_len;
    CPU_BOOLEAN  avail;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    buf_len = ProbeCom_TerminalOutBufLen;
    CPU_CRITICAL_EXIT();

    avail   = (buf_len == 0u) ? (DEF_FALSE) : (DEF_TRUE);

    return (avail);
}
#endif



/*
*********************************************************************************************************
*                                     ProbeCom_TerminalExecDone()
*
* Description : Check if terminal execution is done.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE   if terminal command execution is     done.
*               DEF_FALSE  if terminal command execution is NOT done.
*
* Caller(s)   : ProbeCom_PktModifier().
*
* Note(s)     : (1) See Notes for 'ProbeCom_TerminalOut()'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_BOOLEAN  ProbeCom_TerminalExecDone (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR       cpu_sr;
#endif
    CPU_BOOLEAN  executing;
    CPU_BOOLEAN  done;


    CPU_CRITICAL_ENTER();
    executing = ProbeCom_TerminalExecuting;
    CPU_CRITICAL_EXIT();

    done = (executing == DEF_YES) ? (DEF_FALSE) : (DEF_TRUE);

    return (done);
}
#endif


/*
*********************************************************************************************************
*                                       ProbeCom_PktModifier()
*
* Description : Get packet modfier byte.
*
* Argument(s) : none.
*
* Return(s)   : The modifier byte.
*
* Caller(s)   : Various command handlers (ProbeCom_Req????()).
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  ProbeCom_PktModifier (void)
{
#if ((PROBE_COM_CFG_STR_REQ_EN      == DEF_ENABLED) || \
     (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED))
    CPU_BOOLEAN  avail;
#endif
#if  (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
    CPU_BOOLEAN  done;
#endif
    CPU_INT08U   mod;


    mod = PROBE_COM_MODIFIER_NONE;

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
    avail = ProbeCom_StrOutAvail();
    if (avail == DEF_TRUE) {
        DEF_BIT_SET(mod, PROBE_COM_MODIFIER_STR_OUT_AVAIL);
    }
#endif

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
    avail = ProbeCom_TerminalOutAvail();
    if (avail == DEF_TRUE) {
        DEF_BIT_SET(mod, PROBE_COM_MODIFIER_TERMINAL_OUT_AVAIL);
    }

    done = ProbeCom_TerminalExecDone();
    if (done == DEF_TRUE) {
        DEF_BIT_SET(mod, PROBE_COM_MODIFIER_TERMINAL_EXEC_DONE);
    }
#endif

    return (mod);
}


/*
*********************************************************************************************************
*                                          ProbeCom_ReqErr()
*
* Description : Formulate error response when the target receives a request it cannot handle.
*
* Argument(s) : ptx_buf      Pointer to the transmit buffer
*
*               comm_err     Error that occurred
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : Various command handlers (ProbeCom_Req????()).
*
* Note(s)     : (1) The TX format:
*
*                   (A) A 2-byte format  , indicating the data segment format;
*                   (B) A 1-byte constant, PROBE_RS232_OK,   if the location can    be read;
*                       A 1-byte constant, PROBE_RS232_FAIL, if the location cannot be read.
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeCom_ReqErr (CPU_INT08U  *ptx_buf,
                                     CPU_INT08U   comm_err)
{
    CPU_SIZE_T  tx_len;


#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_ErrPktCtr++;
#endif

    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_ERR);         /* Tx pkt fmt.                                          */
    ProbeCom_StoINT08U(&ptx_buf, comm_err);                     /* Target status.                                       */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* Modifier.                                            */

    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;
    return (tx_len);                                            /* Rtn TX data segment size =  4 (= Tx hdr size).       */
}


/*
*********************************************************************************************************
*                                         ProbeCom_ReqQuery()
*
* Description : Parse the FMT_QUERY command & formulate response.  This command asks the target
*               about its capabilities, which are returned in response.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte query , indicating the query type.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |          Query          |
*                         +-------------------------+-------------------------+
*
*              (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (E) A n-byte answer, the answer to the query.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                       Answer                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeCom_ReqQuery (CPU_INT08U  *prx_buf,
                                       CPU_INT08U  *ptx_buf,
                                       CPU_SIZE_T   rx_pkt_size,
                                       CPU_SIZE_T   tx_buf_size)
{
    CPU_INT16U  query;
    CPU_INT32U  status;
    CPU_SIZE_T  tx_len;
    CPU_INT32U  version;


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */

                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size =  2 (= Rx hdr size)                   */
                                                                /*               +  2 (= Query      ).                  */
    if (rx_pkt_size != 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

                                                                /* ------------------- HANDLE QUERY ------------------- */
    query = ProbeCom_GetINT16U(&prx_buf);                       /* Rd the query.                                        */

    switch (query) {                                            /* Determine if query is supported.                     */
        case PROBE_COM_QUERY_MAX_RX_SIZE:
        case PROBE_COM_QUERY_MAX_TX_SIZE:
        case PROBE_COM_QUERY_FMT_SUPPORT:
        case PROBE_COM_QUERY_ENDIANNESS_TEST:
        case PROBE_COM_QUERY_STATUS:
        case PROBE_COM_QUERY_VERSION:
             break;

        default:
             tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_QUERY_NOT_SUPPORTED);
             return (tx_len);
    }

                                                                /* Sto tx pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_QUERY);       /*  (a) TX pkt fmt.                                     */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /*  (b) Target status.                                  */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /*  (c) Modifier.                                       */

                                                                /* Sto tx data segment data.                            */
    switch (query) {
        case PROBE_COM_QUERY_MAX_RX_SIZE:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_CFG_RX_MAX_SIZE);
             ProbeCom_StoINT16U(&ptx_buf, 0);
             tx_len  = 4u;
             break;


        case PROBE_COM_QUERY_MAX_TX_SIZE:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_CFG_TX_MAX_SIZE);
             ProbeCom_StoINT16U(&ptx_buf, 0);
             tx_len  = 4u;
             break;


        case PROBE_COM_QUERY_FMT_SUPPORT:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_QUERY);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_RD);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_RD_MULTI);
             tx_len  = 8u;
#if (PROBE_COM_CFG_WR_REQ_EN == DEF_ENABLED)
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_WR);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_WR_MULTI);
             tx_len += 4u;
#endif
#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_STR_IN);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_STR_OUT);
             tx_len += 4u;
#endif
#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_TERMINAL_EXEC);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_TERMINAL_IN);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_TERMINAL_OUT);
             tx_len += 6u;
#endif
             break;


        case PROBE_COM_QUERY_ENDIANNESS_TEST:
             ProbeCom_EndiannessTest = 0x12345678;
             Mem_Copy((void     *) ptx_buf,
                      (void     *)&ProbeCom_EndiannessTest,
                      (CPU_SIZE_T) 4);
             tx_len  = 4u;
             break;


        case PROBE_COM_QUERY_STATUS:
             status = (CPU_INT32U)ProbeCom_PktModifier();
             Mem_Copy((void     *) ptx_buf,
                      (void     *)&status,
                      (CPU_SIZE_T) 4);
             tx_len = 4u;
             break;


        case PROBE_COM_QUERY_VERSION:
             version = PROBE_COM_VERSION;
             Mem_Copy((void     *) ptx_buf,
                      (void     *)&version,
                      (CPU_SIZE_T) 4);
             tx_len  = 4u;
             break;


        default:
             tx_len  = 0u;
             break;
    }

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len += PROBE_COM_SIZE_TX_HDR;
    return (tx_len);                                            /* TX pkt size = nbytes (= Tx data size)                */
                                                                /*             + 4      (= Tx hdr  size).               */
}


/*
*********************************************************************************************************
*                                          ProbeCom_ReqRd()
*
* Description : Parse the FMT_RD request & formulate response.  This command causes the target to send
*               data read from its memory for a certain {memory address, data length} pair (which is
*               given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format , indicating the data segment format;
*                   (B) A 2-byte length , indicating the number of bytes to read;
*                   (C) A 4-byte address, the starting address of the data to read.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |     Number of bytes     |
*                         +-------------------------+-------------------------+
*                         |                      Address                      |
*                         +-------------------------+-------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                        Data                       |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeCom_ReqRd (CPU_INT08U  *prx_buf,
                                    CPU_INT08U  *ptx_buf,
                                    CPU_SIZE_T   rx_pkt_size,
                                    CPU_SIZE_T   tx_buf_size)
{
    CPU_ADDR    sym_addr;
    CPU_SIZE_T  sym_size;
    CPU_SIZE_T  tx_len;


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size )                   */
                                                                /*               + 2 (= Sym size    )                   */
                                                                /*               + 4 (= Sym addr    ).                  */
    if (rx_pkt_size != 8u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }


                                                                /* -------------------- HANDLE RD --------------------- */
    sym_size = ProbeCom_GetINT16U(&prx_buf);                    /* Get nbr of bytes to rd.                              */

                                                                /* Get rd addr.                                         */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
    sym_addr = (CPU_ADDR)ProbeCom_GetINT32U(&prx_buf);
#else
    sym_addr = (CPU_ADDR)ProbeCom_GetINT16U(&prx_buf);
#endif

    if (sym_size + PROBE_COM_SIZE_TX_HDR > tx_buf_size) {       /* If tx pkt will NOT fit in buf, rtn err.              */
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_TX_PKT_TOO_LARGE);
        return (tx_len);
    }

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_TxSymCtr++;                                        /* Inc tx sym ctr.                                      */
    ProbeCom_TxSymByteCtr += sym_size;
#endif

                                                                /* Store tx pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_RD);          /*  (a) Tx pkt fmt.                                     */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /*  (b) Target status.                                  */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /*  (c) Modifier.                                       */

    Mem_Copy((void     *)ptx_buf,                               /* Save tx data segment data.                           */
             (void     *)sym_addr,
             (CPU_SIZE_T)sym_size);

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = sym_size + PROBE_COM_SIZE_TX_HDR;
    return (tx_len);                                            /* Tx pkt size = nbytes (= Tx data   size)              */
                                                                /*             + 4      (= Tx header size).             */
}


/*
*********************************************************************************************************
*                                        ProbeCom_ReqRdMulti()
*
* Description : Parse the FMT_RD_MULTI request & formulate respnse.  This command causes the target to
*               send data read from its memory for a certain set of {memory address, data length} pairs
*               (which are given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,          indicating the data segment format;
*                   (B) A 5-byte item descriptor, for each item in the list, consisting of:
*
*                       (1) A 4-byte address, the starting address of the data to read;
*                       (2) A 1-byte length,  indicating the number of bytes to read.
*
*                         +-------------------------+------------+------------+
*                         |          Format         | Num. bytes |         Addr       ---
*                         +-------------------------+------------+------------+        |    Item 1
*                         ress                                   | Num. bytes |       ---
*                         +--------------------------------------+------------+        |
*                         |                      Address                      |        |    Item 2
*                         +------------+--------------------------------------+       ---
*                         | Num. bytes |                                   Addr        |    Item 3
*                         +------------+--------------------------------------+        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         +--------------------------------------+------------+        .
*                         ress                                   | Num. bytes |       ---
*                         ---------------------------------------+------------+        |   Item n
*                         |                      Address                      |        |
*                         +---------------------------------------------------+       ---
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) For each item, the following is sent:
*
*                       (1) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+       ---
*                         |                        Data                       |        |    Item 1
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +---------------------------------------------------+       ---
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         +---------------------------------------------------+       ---
*                         |                        Data                       |        |    Item n
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +---------------------------------------------------+       ---
*
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeCom_ReqRdMulti (CPU_INT08U  *prx_buf,
                                         CPU_INT08U  *ptx_buf,
                                         CPU_SIZE_T   rx_pkt_size,
                                         CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T   rx_pkt_ix;
#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    CPU_INT32U   sym_byte_ctr;
    CPU_INT32U   sym_ctr;
#endif
    CPU_ADDR     sym_addr;
    CPU_SIZE_T   sym_size;
    CPU_SIZE_T   tx_len;
    CPU_INT08U  *ptx_buf_start;


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2 (= Rx hdr size)                   */
                                                                /*                + 5 (= 1 item desc).                  */
    if (rx_pkt_size  < 7u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

                                                                /* -------------------- HANDLE RD --------------------- */
    ptx_buf_start = ptx_buf;                                    /* Save ptr to tx buf in case pkt is too long for buf.  */
    tx_len        = PROBE_COM_SIZE_TX_HDR;                      /* Init tx pkt len = 4 = size of hdr.                   */

                                                                /* Store TX pkt hdr :                                   */
    ptx_buf[0]    = PROBE_COM_FMT_TX_RD_MULTI_LO;               /* (a) TX pkt fmt.                                      */
    ptx_buf[1]    = PROBE_COM_FMT_TX_RD_MULTI_HI;
    ptx_buf[2]    = PROBE_COM_STATUS_OK;                        /* (b) Target status.                                   */
    ptx_buf[3]    = ProbeCom_PktModifier();                     /* (c) Modifier.                                        */
    ptx_buf      += PROBE_COM_SIZE_TX_HDR;

    rx_pkt_ix     = 7u;                                         /* Tx pkt ix after 1st item = 2 (= Rx hdr size)         */
                                                                /*                          + 5 (= 1 item desc).        */

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    sym_ctr      = 0;
    sym_byte_ctr = 0;
#endif
                                                                /* Sto data for each item.                              */
    while (rx_pkt_ix <= rx_pkt_size) {
        sym_size   =  prx_buf[0];                               /* (a) Get nbr of bytes to rd.                          */

                                                                /* (b) Get rd addr.                                     */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
        sym_addr   = ((CPU_ADDR)prx_buf[4] << 8) + prx_buf[3];
        sym_addr   = ((CPU_ADDR)sym_addr   << 8) + prx_buf[2];
        sym_addr   = ((CPU_ADDR)sym_addr   << 8) + prx_buf[1];
#else
        sym_addr   = ((CPU_ADDR)prx_buf[2] << 8) + prx_buf[1];
#endif

        prx_buf   += 5u;
        rx_pkt_ix += 5u;

        tx_len    += sym_size;                                  /* (c) Add nbr of bytes to pkt len.                     */

        if (tx_len > tx_buf_size) {                             /* (d) Will pkt be too long for TX buf? ...             */
            ptx_buf = ptx_buf_start;                            /*     ... rtn err.                                     */
            tx_len  = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_TX_PKT_TOO_LARGE);
            return (tx_len);
        }

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
        sym_ctr++;                                              /* (e) Inc local sym ctr.                               */
        sym_byte_ctr += sym_size;
#endif
                                                                /* (f) Otherwise, save TX data.                         */
        Mem_Copy((void     *)ptx_buf,
                 (void     *)sym_addr,
                 (CPU_SIZE_T)sym_size);

        ptx_buf += sym_size;
    }

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_TxSymCtr     += sym_ctr;                           /* Inc global sym ctr.                                  */
    ProbeCom_TxSymByteCtr += sym_byte_ctr;
#endif

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return (tx_len);
}


/*
*********************************************************************************************************
*                                          ProbeCom_ReqWr()
*
* Description : Parse the FMT_WR request & formulate response.  This command causes the target to write
*               into its memory data from a certain {memory address, data length, data} triplet which is
*               given in the request.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,  indicating the data segment format;
*                   (B) A 2-byte length,  indicating the number of bytes to write;
*                   (C) A 4-byte address, the starting address at which the data should be written;
*                   (D) The memory data.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |      Number of bytes    |
*                         +-------------------------+-------------------------+
*                         |                      Address                      |
*                         +---------------------------------------------------+
*                         |                        Data                       |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_WR_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T  ProbeCom_ReqWr (CPU_INT08U  *prx_buf,
                                    CPU_INT08U  *ptx_buf,
                                    CPU_SIZE_T   rx_pkt_size,
                                    CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T  sym_size;
    CPU_ADDR    sym_addr;
    CPU_SIZE_T  tx_len;
    CPU_SR_ALLOC();


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2      (= Rx hdr size)              */
                                                                /*                + 2      (= Sym size   )              */
                                                                /*                + 4      (= Sym addr   )              */
                                                                /*                + nbytes (= Data       ).             */
    if (rx_pkt_size < 8u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }



                                                                /* -------------------- HANDLE WR --------------------- */
    sym_size = ProbeCom_GetINT16U(&prx_buf);                    /* Get nbr of bytes to wr.                              */

                                                                /* Get wr addr.                                         */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
    sym_addr = (CPU_ADDR)ProbeCom_GetINT32U(&prx_buf);
#else
    sym_addr = (CPU_ADDR)ProbeCom_GetINT16U(&prx_buf);
    prx_buf += 2;
#endif

    if (rx_pkt_size != (8 + sym_size)) {                        /* If rx data segment is NOT expected size ... rtn err. */
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

                                                                /* Copy data into mem.                                  */
    CPU_CRITICAL_ENTER();
    Mem_Copy((void     *)sym_addr,
             (void     *)prx_buf,
             (CPU_SIZE_T)sym_size);
    CPU_CRITICAL_EXIT();

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_RxSymCtr++;                                        /* Inc global sym ctr.                                  */
    ProbeCom_RxSymByteCtr += sym_size;
#endif

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_WR);          /* (a) TX pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */



                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;                 /* Tx pkt size = 4 (= Tx hdr size).                     */
    return (tx_len);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_ReqWrMulti()
*
* Description : Parse the FMT_WR_MULTI request & formulate response.  This command causes the target to
*               write certain data into its memroy, for a certain number of {memory address, data length,
*               data} triplets (which are given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,  indicating the data segment format;
*                   (B) A n-byte item descriptor, for each item in the list, consiting of the following:
*
*                       (1) A 1-byte length,  indicating the number of bytes to write;
*                       (2) A 4-byte address, the starting address at which the data should be written;
*                       (3) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         | Num. bytes |         Addr       ---
*                         +-------------------------+------------+------------+        |
*                         ress                                   |            |        |
*                         +--------------------------------------+            |        |    Item 1
*                         |                       Data                        |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +------------+--------------------------------------+       ---
*                         | Num. bytes |                                   Addr        |    Item 2
*                         +------------+--------------------------------------+        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .            +------------+        .
*                         |                         .            | Num. bytes |       ---
*                         ---------------------------------------+------------+        |
*                         |                      Address                      |        |
*                         +---------------------------------------------------+        |    Item n
*                         |                       Data                        |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +------------+--------------------------------------+       ---
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_WR_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T  ProbeCom_ReqWrMulti (CPU_INT08U  *prx_buf,
                                         CPU_INT08U  *ptx_buf,
                                         CPU_SIZE_T   rx_pkt_size,
                                         CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T  rx_pkt_ix;
#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    CPU_INT32U  sym_ctr;
    CPU_INT32U  sym_byte_ctr;
#endif
    CPU_ADDR    sym_addr;
    CPU_SIZE_T  sym_size;
    CPU_SIZE_T  tx_len;
    CPU_SR_ALLOC();


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2      (= Rx hdr size )             */
                                                                /*                + 1      (= Nbr of bytes)             */
                                                                /*                + 4      (= Addr        )             */
                                                                /*                + nbytes (= Data        ).            */
    if (rx_pkt_size < 7) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

                                                                /* -------------------- HANDLE WR --------------------- */
    rx_pkt_ix  = 7;                                             /* Rx pkt ix after 1st item = 2 (= Rx hdr size)         */
                                                                /*                          + 5 (= 1 item desc).        */

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    sym_ctr      = 0;
    sym_byte_ctr = 0;
#endif
                                                                /* Sto data for each item.                              */
    while (rx_pkt_ix <= rx_pkt_size) {
        sym_size = prx_buf[0];                                  /* (a) Get nbr of bytes to wr.                          */

                                                                /* (b) Get wr addr.                                     */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
        sym_addr = ((CPU_ADDR)prx_buf[4] << 8) + prx_buf[3];
        sym_addr = ((CPU_ADDR)sym_addr   << 8) + prx_buf[2];
        sym_addr = ((CPU_ADDR)sym_addr   << 8) + prx_buf[1];
#else
        sym_addr = ((CPU_ADDR)prx_buf[2] << 8) + prx_buf[1];
#endif

        prx_buf += 5u;

        if (rx_pkt_ix + sym_size > rx_pkt_size) {
            tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
            return (tx_len);
        }

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
        sym_ctr++;                                              /* (c) Inc local sym ctr.                               */
        sym_byte_ctr += sym_size;
#endif
                                                                /* (d) Sto data into pkt.                               */
        CPU_CRITICAL_ENTER();
        Mem_Copy((void     *)sym_addr,
                 (void     *)prx_buf,
                 (CPU_SIZE_T)sym_size);
        CPU_CRITICAL_EXIT();

        prx_buf   += sym_size;
        rx_pkt_ix += 5u + sym_size;
    }

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_RxSymCtr     += sym_ctr;                           /* Inc global sym ctr.                                  */
    ProbeCom_RxSymByteCtr += sym_byte_ctr;
#endif

                                                                /* Sto tx pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_WR_MULTI);    /* (a) tx pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;                 /* Tx pkt size = 4 (= Tx hdr size).                     */
    return (tx_len);
}
#endif


/*
*********************************************************************************************************
*                                         ProbeCom_ReqStrIn()
*
* Description : Parse the FMT_STR_IN command & formulate response.  This command sends a string to the
*               target.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding;
*                   (C) A n-byte string.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*                         |                       String                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T  ProbeCom_ReqStrIn (CPU_INT08U  *prx_buf,
                                       CPU_INT08U  *ptx_buf,
                                       CPU_SIZE_T   rx_pkt_size,
                                       CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T   len;
    CPU_SIZE_T   len_wr;
    CPU_SIZE_T   tx_len;
    CPU_INT08U  *pstr;


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size)                    */
                                                                /*               + 2 (= Padding    ).                   */
    if (rx_pkt_size <= 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }



                                                                /* ------------------- HANDLE STR --------------------- */
    pstr   = prx_buf     + 2u;                                   /* Ignore padding.                                      */
    len    = rx_pkt_size - 4u;

    len_wr = ProbeCom_BufWr(&ProbeCom_StrInBuf,                 /* Rd str from buf.                                     */
                             pstr,
                             len);

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    if (len_wr < len) {
        ProbeCom_StrRxOvfErrCtr++;
    }

    ProbeCom_StrRxCtr += len_wr;
#endif


                                                                /* ------------------ STORE RESPONSE ------------------ */
                                                                /* Sto tx pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_STR_IN);      /* (a) Tx pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */



                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;                 /* Tx pkt size = 4 (= Tx hdr size).                     */
    return (tx_len);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_ReqStrOut()
*
* Description : Parse the FMT_STR_OUT command & formulate response.  This command asks the target to send
*               a string that it is currently storing.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) A n-byte string, the string which is stored in the target's buffer.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                       String                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (3) See Notes for 'ProbeCom_StrWr()'.
*
*               (4) Probe requires that a NULL byte ends the string.  Consequently, if the data
*                   section of the TX packet is n bytes long, then only (n - 1) bytes have been read from
*                   the string buffer.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  CPU_SIZE_T  ProbeCom_ReqStrOut (CPU_INT08U  *prx_buf,
                                        CPU_INT08U  *ptx_buf,
                                        CPU_SIZE_T   rx_pkt_size,
                                        CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T  len_rd;
    CPU_SIZE_T  tx_len;
    CPU_SIZE_T  tx_max;


   (void)&prx_buf;                                              /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size)                    */
                                                                /*               + 2 (= Padding    ).                   */
    if (rx_pkt_size != 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

                                                                /* --------------------- STORE STR -------------------- */
                                                                /* Sto TX pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_STR_OUT);     /* (a) TX pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */

    tx_max = (tx_buf_size - PROBE_COM_SIZE_TX_HDR) - 1;         /* Calc size of largest tx str.                         */

    len_rd = ProbeCom_BufRd(&ProbeCom_StrOutBuf,                /* Rd str from buf.                                     */
                             ptx_buf,
                             tx_max);

    if (len_rd == 0u) {                                         /* If there is NO str ... rtn err.                      */
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_STR_OUT_NONE);
        return (tx_len);
    }

   *(ptx_buf + len_rd) = (CPU_INT08U)0;                         /* Wr NULL byte.                                        */

#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeCom_StrTxCtr += len_rd;
#endif


                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = PROBE_COM_SIZE_TX_HDR + len_rd + 1u;
    return (tx_len);                                            /* Tx pkt size = 4      (= Tx hdr size)                 */
                                                                /*             + nbytes (= Str    size)                 */
                                                                /*             + 1      (= NULL byte  ).                */
}
#endif


/*
*********************************************************************************************************
*                                     ProbeCom_ReqTerminalExec()
*
* Description : Parse the FMT_TERMINAL_EXEC command & formulate response.  This command sends a terminal
*               command to the target for execution.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding;
*                   (C) A n-byte string.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*                         |                  Terminal Command                 |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T  ProbeCom_ReqTerminalExec (CPU_INT08U  *prx_buf,
                                              CPU_INT08U  *ptx_buf,
                                              CPU_SIZE_T   rx_pkt_size,
                                              CPU_SIZE_T   tx_buf_size)
{
    PROBE_COM_TERMINAL_EXEC_FNCT   handler;
    CPU_SIZE_T                     len;
    CPU_SIZE_T                     tx_len;
    CPU_INT08U                    *pstr;
    CPU_SR_ALLOC();


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size)                    */
                                                                /*               + 2 (= Padding    ).                   */
    if (rx_pkt_size <= 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }



                                                                /* -------------------- HANDLE STR -------------------- */
    CPU_CRITICAL_ENTER();
    handler = ProbeCom_TerminalExecHandler;
    CPU_CRITICAL_EXIT();

    if (handler != (PROBE_COM_TERMINAL_EXEC_FNCT)0) {           /* If handler assigned ...                              */
        pstr = prx_buf     + 2u;                                /* ... ignore padding  ...                              */
        len  = rx_pkt_size - 2u;

        ProbeCom_TerminalExecuting = DEF_YES;

        handler((CPU_CHAR *)pstr,                               /* ... exec handler.                                    */
                (CPU_SIZE_T)len);
    }



                                                                /* ------------------ STORE RESPONSE ------------------ */
                                                                /* Sto tx pkt hdr :                                     */
                                                                /* (a) Tx pkt fmt.                                      */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_TERMINAL_EXEC);
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */



                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;                 /* Tx pkt size = 4 (= Tx hdr size).                     */
    return (tx_len);
}
#endif


/*
*********************************************************************************************************
*                                      ProbeCom_ReqTerminalIn()
*
* Description : Parse the FMT_TERMINAL_IN command & formulate response.  This command sends a terminal
*               input to the target.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding;
*                   (C) A n-byte string.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*                         |                   Terminal Input                  |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_ENABLED)
static  CPU_SIZE_T  ProbeCom_ReqTerminalIn (CPU_INT08U  *prx_buf,
                                            CPU_INT08U  *ptx_buf,
                                            CPU_SIZE_T   rx_pkt_size,
                                            CPU_SIZE_T   tx_buf_size)
{
    PROBE_COM_TERMINAL_IN_FNCT   handler;
    CPU_SIZE_T                   len;
    CPU_SIZE_T                   tx_len;
    CPU_INT08U                  *pstr;
    CPU_SR_ALLOC();


   (void)&tx_buf_size;                                          /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size)                    */
                                                                /*               + 2 (= Padding    ).                   */
    if (rx_pkt_size <= 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }



                                                                /* -------------------- HANDLE STR -------------------- */
    CPU_CRITICAL_ENTER();
    handler = ProbeCom_TerminalInHandler;
    CPU_CRITICAL_EXIT();

    if (handler != (PROBE_COM_TERMINAL_IN_FNCT)0) {             /* If handler assigned.                                 */
        pstr = prx_buf     + 4u;
        len  = rx_pkt_size - 4u;

        handler((CPU_CHAR *)pstr,
                (CPU_SIZE_T)len);
    }



                                                                /* ------------------ STORE RESPONSE ------------------ */
                                                                /* Sto tx pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_TERMINAL_IN); /* (a) Tx pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */



                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = (CPU_SIZE_T)PROBE_COM_SIZE_TX_HDR;                 /* Tx pkt size = 4 (= Tx hdr size).                     */
    return (tx_len);
}
#endif


/*
*********************************************************************************************************
*                                      ProbeCom_ReqTerminalOut()
*
* Description : Parse the FMT_TERMINAL_OUT command & formulate response.  This command asks the target to
*               send terminal output that it is currently storing.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) A n-byte string, the string which is stored in the target's buffer.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                  Terminal Output                  |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (3) See Notes for 'ProbeCom_StrWr()'.
*
*               (4) Probe requires that a NULL byte ends the string.  Consequently, if the data
*                   section of the TX packet is n bytes long, then only (n - 1) bytes have been read from
*                   the string buffer.
*
*               (5) A NULL modifier is returned so that string transmission will not monopolize the
*                   transmission channel.  If more string is queued in the string buffer, additional
*                   string bytes will be sent following the response to the next received packet.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_TERMINAL_REQ_EN == DEF_TRUE)
static  CPU_SIZE_T  ProbeCom_ReqTerminalOut (CPU_INT08U  *prx_buf,
                                             CPU_INT08U  *ptx_buf,
                                             CPU_SIZE_T   rx_pkt_size,
                                             CPU_SIZE_T   tx_buf_size)
{
    CPU_SIZE_T   buf_ix;
    CPU_SIZE_T   buf_len;
    CPU_SIZE_T   tx_len;
    CPU_SIZE_T   tx_max;
    CPU_INT08U  *pbuf;
    CPU_SR_ALLOC();


   (void)&prx_buf;                                              /* Prevent compiler warning.                            */


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx hdr size)                    */
                                                                /*               + 2 (= Padding    ).                   */
    if (rx_pkt_size != 4u) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE);
        return (tx_len);
    }

    CPU_CRITICAL_ENTER();
    pbuf    = ProbeCom_TerminalOutBufPtr;
    buf_ix  = ProbeCom_TerminalOutBufIx;
    buf_len = ProbeCom_TerminalOutBufLen;
    CPU_CRITICAL_EXIT();

    if (pbuf == (CPU_INT08U *)0) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_TERMINAL_OUT_NONE);
        return (tx_len);
    }

    if (buf_ix == buf_len) {
        tx_len = ProbeCom_ReqErr(ptx_buf, PROBE_COM_STATUS_TERMINAL_OUT_NONE);
        return (tx_len);
    }

    tx_max = buf_len - buf_ix;
    if (tx_max > (tx_buf_size - PROBE_COM_SIZE_TX_HDR) - 1) {
        tx_max = (tx_buf_size - PROBE_COM_SIZE_TX_HDR) - 1;
    }


                                                                /* --------------------- STORE OUT -------------------- */
                                                                /* Sto TX pkt hdr :                                     */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_TERMINAL_OUT);/* (a) TX pkt fmt.                                      */
    ProbeCom_StoINT08U(&ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT08U(&ptx_buf, 0);                            /* (c) Modifier (see Notes #5).                         */

    Mem_Copy((void       *) ptx_buf,                            /* Copy str from buf.                                   */
             (void       *)&pbuf[buf_ix],
             (CPU_SIZE_T  ) tx_max);

   *(ptx_buf + tx_max)        = (CPU_INT08U)0;                  /* Wr NULL byte.                                        */

    CPU_CRITICAL_ENTER();
    ProbeCom_TerminalOutBufIx = buf_ix + tx_max;                /* Update buf ix.                                       */
    CPU_CRITICAL_EXIT();

    if (buf_ix + tx_max >= buf_len) {
        ProbeCom_OS_TerminalOutSignal();
    }


                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    tx_len = PROBE_COM_SIZE_TX_HDR + tx_max + 1u;
    return (tx_len);                                            /* Tx pkt size = 4      (= Tx hdr size)                 */
                                                                /*             + nbytes (= Str    size)                 */
                                                                /*             + 1      (= NULL byte  ).                */
}
#endif


/*
*********************************************************************************************************
*                                         ProbeCom_BufInit()
*
* Description : Initialize buffer.
*
* Argument(s) : pbuf            Pointer to buffer.
*
*               pdata           Pointer to buffer data.
*
*               len             Buffer length.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The string buffer is implemented as a circular buffer.  This function is one of two
*                   points of access for this buffer, the other being in the task or ISR which forms the
*                   tx packets.  Only this function should modify the global current write index
*                   (pbuf->IxWr); only the task or ISR which forms the packets should modify the global
*                   current read index (pbuf->IxRd).
*
*               (2) The global current write index (pbuf->IxWr) is the index of the next location in the
*                   buffer to write.  The global current read index (pbuf->IxRd) is the index of the next
*                   location in the buffer to read.
*
*               (3) The string buffer, an array of pbuf->Len bytes, can only hold (pbuf->Len - 1) bytes
*                   so that the condition
*
*                                        pbuf->IxWr == pbuf->IxRd
*
*                   will be true if and only if the buffer is empty.  Consequently, this function
*                   always leaves an empty space in the buffer.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  void  ProbeCom_BufInit (PROBE_COM_BUF  *pbuf,
                                CPU_INT08U     *pdata,
                                CPU_SIZE_T      len)
{
    pbuf->Len      = len;
    pbuf->IxRd     = 0;
    pbuf->IxWr     = 0;
    pbuf->DataPtr  = pdata;
}
#endif


/*
*********************************************************************************************************
*                                          ProbeCom_BufRd()
*
* Description : Read from buffer.
*
* Argument(s) : pbuf            Pointer to buffer.
*
*               pdest           Pointer to destination buffer.
*
*               len             Number of octets to read.
*
* Return(s)   : The number of octets read from buffer.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  CPU_SIZE_T  ProbeCom_BufRd (PROBE_COM_BUF  *pbuf,
                                    CPU_INT08U     *pdest,
                                    CPU_SIZE_T      len)
{
    CPU_SIZE_T  cnt_copy;
    CPU_SIZE_T  cnt_full;
    CPU_SIZE_T  cnt_rem;
    CPU_SIZE_T  ix_rd;
    CPU_SIZE_T  ix_wr;
    CPU_SIZE_T  len_buf;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    ix_rd   = pbuf->IxRd;
    ix_wr   = pbuf->IxWr;
    len_buf = pbuf->Len;

    if (ix_wr == ix_rd) {                                       /* Buf is empty.                                        */
        CPU_CRITICAL_EXIT();
        return ((CPU_SIZE_T)0);
    }



                                                                /* -------------- COPY DATA FROM BUF END -------------- */
    if (ix_wr > ix_rd) {                                        /* Get str at buf interval [rd_ix, wr_ix).              */
        cnt_full = ix_wr - ix_rd;
    } else {                                                    /* Get str at buf interval [rd_ix, end_ix).             */
        cnt_full = len_buf - ix_rd;
    }

    cnt_copy  = DEF_MIN(cnt_full, len);                         /* Copy data.                                           */
    Mem_Copy((void *)pdest, (void *)&pbuf->DataPtr[ix_rd], cnt_copy);


    if ((ix_wr > ix_rd) || (ix_wr == 0u)) {
        ix_rd += cnt_copy;
        if (ix_rd == len_buf) {
            pbuf->IxRd = 0u;
        } else {
            pbuf->IxRd = ix_rd;
        }
        CPU_CRITICAL_EXIT();

        return (cnt_copy);
    }



                                                                /* -------------- COPY DATA AT BUF START -------------- */
    ix_rd    += cnt_copy;
    pdest    += cnt_copy;                                       /* Adj buf ptr.                                         */
    cnt_rem   = len - cnt_copy;                                 /* Calc nbr data octets to copy from buf start.         */
    cnt_copy  = DEF_MIN(ix_wr, cnt_rem);

    if (cnt_copy > 0u) {
                                                                /* Copy data.                                           */
        Mem_Copy((void *)pdest, (void *)&pbuf->DataPtr[0], cnt_copy);
        pbuf->IxRd = cnt_copy;
    } else {
        if (ix_rd == len_buf) {
            pbuf->IxRd = 0u;
        } else {
            pbuf->IxRd = ix_rd;
        }
    }
    CPU_CRITICAL_EXIT();

    return ((len - cnt_rem) + cnt_copy);
}
#endif


/*
*********************************************************************************************************
*                                          ProbeCom_BufWr()
*
* Description : Write to buffer.
*
* Argument(s) : pbuf            Pointer to buffer.
*
*               psrc            Pointer to source buffer.
*
*               len             Number of octets to write.
*
*
* Return(s)   : Number of octets stored in buffer.
*
* Caller(s)   : Hardware Port's ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  CPU_SIZE_T  ProbeCom_BufWr (PROBE_COM_BUF  *pbuf,
                                    CPU_INT08U     *psrc,
                                    CPU_SIZE_T      len)
{
    CPU_SIZE_T  cnt_copy;
    CPU_SIZE_T  cnt_empty;
    CPU_SIZE_T  cnt_rem;
    CPU_SIZE_T  ix_rd;
    CPU_SIZE_T  ix_wr;
    CPU_SIZE_T  len_buf;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    ix_rd   = pbuf->IxRd;
    ix_wr   = pbuf->IxWr;
    len_buf = pbuf->Len;

                                                                /* --------------- COPY DATA AT BUF END --------------- */
    if (ix_rd > ix_wr) {                                        /* Sto str at buf interval [wr_ix, rd_ix - 1).          */
        cnt_empty = (ix_rd   - ix_wr) - 1u;
    } else if (ix_rd == 0u) {                                   /* Sto str at buf interval [wr_ix, end_ix - 1).         */
        cnt_empty = (len_buf - ix_wr) - 1u;
    } else {                                                    /* Sto str at buf interval [wr_ix, end_ix).             */
        cnt_empty =  len_buf - ix_wr;
    }

    cnt_copy  = DEF_MIN(cnt_empty, len);                        /* Copy data.                                           */
    Mem_Copy((void *)&pbuf->DataPtr[ix_wr], (void *)psrc, cnt_copy);


    if ((ix_rd > ix_wr) || (ix_rd == 0u)) {
        pbuf->IxWr = ix_wr + cnt_copy;
        CPU_CRITICAL_EXIT();

        return (cnt_copy);
    }



                                                                /* -------------- COPY DATA AT BUF START -------------- */
    ix_wr    += cnt_copy;
    psrc     += cnt_copy;                                       /* Adj buf ptr.                                         */
    cnt_rem   = len - cnt_copy;                                 /* Calc nbr data octets to copy from buf start.         */
    cnt_copy  = DEF_MIN(ix_rd - 1u, cnt_rem);

    if (cnt_copy > 0u) {
                                                                /* Copy data.                                           */
        Mem_Copy((void *)&pbuf->DataPtr[0], (void *)psrc, cnt_copy);
        pbuf->IxWr = cnt_copy;
    } else {
        if (ix_wr == len_buf) {
            pbuf->IxWr = 0u;
        } else {
            pbuf->IxWr = ix_wr;
        }
    }
    CPU_CRITICAL_EXIT();

    return ((len - cnt_rem) + cnt_copy);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_BufIsEmpty()
*
* Description : Determine whether buffer is empty.
*
* Argument(s) : pbuf            Pointer to buffer.
*
* Return(s)   : DEF_YES, if buffer is empty.
*               DEF_NO,  if buffer contains any data.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_STR_REQ_EN == DEF_TRUE)
static  CPU_BOOLEAN  ProbeCom_BufIsEmpty  (PROBE_COM_BUF  *pbuf)
{
    CPU_BOOLEAN  empty;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    empty = ((pbuf->IxRd == pbuf->IxWr) ? (DEF_YES) : (DEF_NO));
    CPU_CRITICAL_EXIT();

    return (empty);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_BufIsFull()
*
* Description : Determine whether buffer is full.
*
* Argument(s) : pbuf            Pointer to buffer.
*
* Return(s)   : DEF_YES, if buffer is full.
*               DEF_NO,  otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if 0
static  CPU_BOOLEAN  ProbeCom_BufIsFull  (PROBE_COM_BUF  *pbuf)
{
    CPU_BOOLEAN  full;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    if (pbuf->IxWr == 0) {
        full = ((pbuf->IxRd == pbuf->Len  - 1) ? (DEF_YES) : (DEF_NO));
    } else {
        full = ((pbuf->IxRd == pbuf->IxWr - 1) ? (DEF_YES) : (DEF_NO));
    }
    CPU_CRITICAL_EXIT();

    return (full);
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_GetINT08U()
*                                        ProbeCom_GetINT16U()
*                                        ProbeCom_GetINT32U()
*
* Description : Retrieve data from a buffer and increment buffer pointer.
*
* Argument(s) : pbuf            The pointer to the buffer pointer.
*
* Return(s)   : A 8-, 16-, or 32-bit datum, respectively.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  ProbeCom_GetINT08U (CPU_INT08U **pbuf)
{
    return (*((*pbuf)++));
}


static  CPU_INT16U  ProbeCom_GetINT16U (CPU_INT08U **pbuf)
{
    CPU_INT16U  lowbyte;
    CPU_INT16U  highbyte;


    lowbyte  = ProbeCom_GetINT08U(pbuf);
    highbyte = ProbeCom_GetINT08U(pbuf);
    return ((CPU_INT16U)((CPU_INT16U)(highbyte << 8) | (CPU_INT16U)lowbyte));
}

static  CPU_INT32U  ProbeCom_GetINT32U (CPU_INT08U **pbuf)
{
    CPU_INT32U  highword;
    CPU_INT32U  lowword;


    lowword  = ProbeCom_GetINT16U(pbuf);
    highword = ProbeCom_GetINT16U(pbuf);
    return ((CPU_INT32U)((highword << 16) | lowword));
}


/*
*********************************************************************************************************
*                                        ProbeCom_StoINT08U()
*                                        ProbeCom_StoINT16U()
*                                        ProbeCom_StoINT32U()
*
* Description : Store data into a buffer and increment the buffer pointer.
*
* Argument(s) : pbuf            The pointer to the buffer pointer.
*
*               data            A 8-, 16-, or 32-bit datum, respectively.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeCom_StoINT08U (CPU_INT08U **pbuf, CPU_INT08U data)
{
    *((*pbuf)++) = data;
}


static  void  ProbeCom_StoINT16U (CPU_INT08U **pbuf, CPU_INT16U data)
{
    ProbeCom_StoINT08U(pbuf, (CPU_INT08U)(data & 0x00FF));
    ProbeCom_StoINT08U(pbuf, (CPU_INT08U)(data >> 8));
}


#if 0
static  void  ProbeCom_StoINT32U (CPU_INT08U **pbuf, CPU_INT32U data)
{
    ProbeCom_StoINT16U(pbuf, (CPU_INT16U)(data & 0x0000FFFFL));
    ProbeCom_StoINT16U(pbuf, (CPU_INT16U)(data >> 16));
}
#endif
