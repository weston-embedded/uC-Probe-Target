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
* Filename : probe_rs232.c
* Version  : V2.30
*********************************************************************************************************
* Note(s)  : (1) The abbreviations RX and TX refer to communication from the target's perspective.
*
*            (2) The abbreviations RD and WR refer to reading data from the target memory and
*                writing data to the target memory, respectively.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    PROBE_RS232_MODULE
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

/*
*********************************************************************************************************
*                                        RS-232 PACKET FORMAT
*
* Note(s):  (1) All packets include the following parts:
*
*                   (A)  4 1-byte start delimiters, forming the ASCII representation of "uCPr".  These
*                        are the constants PROBE_RS232_PROTOCOL_RX_SD0-PROBE_RS232_PROTOCOL_?X_SD4;
*                   (B)  1 2-byte length, the length of the data segment;
*                   (C)  1 1-byte packet counter.
*                   (D)  1 1-byte padding, unused;
*                   (E)  n   bytes of data; and
*                   (F)  1 1-byte checksum; and
*                   (G)  1 1-byte end delimiter, the character '/', which is the constant PROBE_RS232_PROTOCOL_?X_ED.
*
*                                       +-------------------+-------------------+
*                                       |   'u'   |   'C'   |   'P'   |   'r'   |
*                                       +-------------------+-------------------+
*                                       |       Length      | PktCtr  | Padding |
*                                       +-------------------+-------------------+
*                                       |                  Data                 |   The data segment does not need to end on
*                                       |                   .                   |   a four-byte boundary, as might be inferred
*                                       |                   .                   |   from this diagram.
*                                       |                   .                   |
*                                       +-------------------+-------------------+
*                                       | Checksum|   '/'   |
*                                       +-------------------+
*********************************************************************************************************
*/

                                                                /* ------------- INBOUND PACKET DELIMITERS ------------ */
#define  PROBE_RS232_PROTOCOL_RX_SD0                    0x75u   /* Start delimiters.                                    */
#define  PROBE_RS232_PROTOCOL_RX_SD1                    0x43u
#define  PROBE_RS232_PROTOCOL_RX_SD2                    0x50u
#define  PROBE_RS232_PROTOCOL_RX_SD3                    0x72u
#define  PROBE_RS232_PROTOCOL_RX_ED                     0x2Fu   /* End   delimiter.                                     */

                                                                /* ------------ OUTBOUND PACKET DELIMITERS ------------ */
#define  PROBE_RS232_PROTOCOL_TX_SD0                    0x75u   /* Start delimiters.                                    */
#define  PROBE_RS232_PROTOCOL_TX_SD1                    0x43u
#define  PROBE_RS232_PROTOCOL_TX_SD2                    0x50u
#define  PROBE_RS232_PROTOCOL_TX_SD3                    0x72u
#define  PROBE_RS232_PROTOCOL_TX_ED                     0x2Fu   /* End   delimiter.                                     */

                                                                /* ----------- RECEIVE STATE MACHINE STATES ----------- */
#define  PROBE_RS232_RX_STATE_SD0                          0u   /* Waiting for start first  start delimiter (SD0).      */
#define  PROBE_RS232_RX_STATE_SD1                          1u   /* Waiting for start second start delimiter (SD1).      */
#define  PROBE_RS232_RX_STATE_SD2                          2u   /* Waiting for start third  start delimiter (SD2).      */
#define  PROBE_RS232_RX_STATE_SD3                          3u   /* Waiting for start fourth start delimiter (SD3).      */
#define  PROBE_RS232_RX_STATE_LEN1                         4u   /* Waiting for length,  first  byte.                    */
#define  PROBE_RS232_RX_STATE_LEN2                         5u   /* Waiting for length,  second byte.                    */
#define  PROBE_RS232_RX_STATE_CTR                          6u   /* Waiting for packet counter.                          */
#define  PROBE_RS232_RX_STATE_PAD                          7u   /* Waiting for padding.                                 */
#define  PROBE_RS232_RX_STATE_DATA                         8u   /* Waiting for data.                                    */
#define  PROBE_RS232_RX_STATE_CHKSUM                       9u   /* Waiting for checksum.                                */
#define  PROBE_RS232_RX_STATE_ED                          10u   /* Waiting for end delimiter.                           */

                                                                /* ---------- TRANSMIT STATE MACHINE STATES ----------- */
#define  PROBE_RS232_TX_STATE_SD0                          0u   /* Waiting to send start first  start delim. (SD0).     */
#define  PROBE_RS232_TX_STATE_SD1                          1u   /* Waiting to send start second start delim. (SD1).     */
#define  PROBE_RS232_TX_STATE_SD2                          2u   /* Waiting to send start third  start delim. (SD2).     */
#define  PROBE_RS232_TX_STATE_SD3                          3u   /* Waiting to send start fourth start delim. (SD3).     */
#define  PROBE_RS232_TX_STATE_LEN1                         4u   /* Waiting to send length,  first  byte.                */
#define  PROBE_RS232_TX_STATE_LEN2                         5u   /* Waiting to send length,  second byte.                */
#define  PROBE_RS232_TX_STATE_CTR                          6u   /* Waiting to send packet counter.                      */
#define  PROBE_RS232_TX_STATE_PAD                          7u   /* Waiting to send padding.                             */
#define  PROBE_RS232_TX_STATE_DATA                         8u   /* Waiting to send data.                                */
#define  PROBE_RS232_TX_STATE_CHKSUM                       9u   /* Waiting to send checksum.                            */
#define  PROBE_RS232_TX_STATE_ED                          10u   /* Waiting to send end delimiter.                       */

#define  PROBE_RS232_CHKSUM_EN                      DEF_ENABLED /* Checksums are supported in Probe ver 4.2 or newer.   */


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

                                                                /* ---------------- RX STATE VARIABLES ---------------- */
static  CPU_INT08U   ProbeRS232_RxState;                        /* Current state of rx state machine.                   */
static  CPU_SIZE_T   ProbeRS232_RxLenRem;                       /* Rem bytes of data to rd.                             */

static  CPU_SIZE_T   ProbeRS232_RxPktCtr;                       /* Packet counter of  current pkt.                      */
static  CPU_SIZE_T   ProbeRS232_RxLen;                          /* Length  of data in current pkt.                      */
                                                                /* Data    of current pkt.                              */
static  CPU_INT08U   ProbeRS232_RxBuf[PROBE_RS232_CFG_RX_BUF_SIZE];
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
static  CPU_INT08U   ProbeRS232_RxChkSum;                       /* Checksum of current pkt.                             */
#endif

static  CPU_SIZE_T   ProbeRS232_RxBufWrIx;                      /* Index of next write; also number of bytes in buf.    */


                                                                /* ---------------- TX STATE VARIABLES ---------------- */
static  CPU_INT08U   ProbeRS232_TxState;                        /* Current state of tx state machine.                   */
static  CPU_BOOLEAN  ProbeRS232_TxActiveFlag;                   /* Indicates tx is currently active.                    */

static  CPU_SIZE_T   ProbeRS232_TxLen;                          /* Length  of data in current pkt.                      */
                                                                /* Data    of current pkt.                              */
static  CPU_INT08U   ProbeRS232_TxBuf[PROBE_RS232_CFG_TX_BUF_SIZE];
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
static  CPU_INT08U   ProbeRS232_TxChkSum;                       /* Checksum of current pkt.                             */
#endif

static  CPU_BOOLEAN  ProbeRS232_TxBufInUse;                     /* Indicates tx buf currently holds a pkt.              */
static  CPU_SIZE_T   ProbeRS232_TxBufRdIx;                      /* Index of next read.                                  */



/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeRS232_ParseRxPkt(void);

static  void        ProbeRS232_RxPkt     (void);

static  void        ProbeRS232_RxStoINT8U(CPU_INT08U  rx_data);

static  void        ProbeRS232_RxBufClr  (void);

static  void        ProbeRS232_TxStart   (void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          ProbeRS232_Init()
*
* Description : Initialize the RS-232 communication module.
*
* Argument(s) : baud_rate       The RS-232 baud rate which will be passed to the hardware initialization.
*
* Return(s)   : DEF_OK,   if initialization successful.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  ProbeRS232_Init (CPU_INT32U baud_rate)
{
#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
    CPU_BOOLEAN  ok;
#endif


    ProbeRS232_RxState             =  PROBE_RS232_RX_STATE_SD0; /* Setup rx & tx state machines.                        */
    ProbeRS232_TxState             =  PROBE_RS232_TX_STATE_SD0;

    ProbeRS232_TxLen               =  0u;
    ProbeRS232_TxActiveFlag        =  DEF_FALSE;
    ProbeRS232_TxBufInUse          =  DEF_FALSE;

#if (PROBE_COM_CFG_STAT_EN         == DEF_ENABLED)
    ProbeRS232_RxCtr               =  0u;
    ProbeRS232_TxCtr               =  0u;
#endif

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
    ok = ProbeRS232_OS_Init();
    if (ok == DEF_FAIL) {
        return (DEF_FAIL);
    }
#endif

    ProbeRS232_InitTarget(baud_rate);                           /* Init target-specific code.                           */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         INTERNAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          ProbeRS232_Task()
*
* Description : The task which parses the received packet, forms a response, and begins transmission.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_OS_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
void  ProbeRS232_Task (void)
{
    CPU_SIZE_T  len;


    while (DEF_TRUE) {
        ProbeRS232_OS_Pend();                                   /* Wait for a packet to be received.                    */

        len = ProbeRS232_ParseRxPkt();                          /* Parse packet and formulate a response.               */

        if (len > 0u) {                                         /* If we have a response.                               */
            ProbeRS232_TxLen = len;
            ProbeRS232_TxStart();
        }
    }
}
#endif


/*
*********************************************************************************************************
*                                        ProbeRS232_RxHandler()
*
* Description : Handle a received byte.
*
* Argument(s) : rx_data     Received data byte.
*
* Return(s)   : none.
*
* Caller(s)   : Rx ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxHandler (CPU_INT08U rx_data)
{
#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    ProbeRS232_RxCtr++;
#endif

    switch (ProbeRS232_RxState) {
        case PROBE_RS232_RX_STATE_SD0:                          /* Rx'd the start 1st start delimiter (SD0).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD0) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD1;
                 ProbeRS232_RxBufClr();
             }
             break;


        case PROBE_RS232_RX_STATE_SD1:                          /* Rx'd the start 2nd start delimiter (SD1).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD1) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD2;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_SD2:                          /* Rx'd the start 3rd start delimiter (SD2).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD2) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD3;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_SD3:                          /* Rx'd the start 4th start delimiter (SD3).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD3) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_LEN1;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_LEN1:                         /* Rx'd the 1st len byte.                               */
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_RxChkSum = rx_data;
#endif
             ProbeRS232_RxLenRem = rx_data;
             ProbeRS232_RxState  = PROBE_RS232_RX_STATE_LEN2;
             break;


        case PROBE_RS232_RX_STATE_LEN2:                         /* Rx'd the 2nd len byte.                               */
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_RxChkSum += rx_data;
#endif
             ProbeRS232_RxLenRem |= rx_data << 8;

                                                                /* Chk len.                                             */
             if ((ProbeRS232_RxLenRem == 0u) || (ProbeRS232_RxLenRem > PROBE_RS232_CFG_RX_BUF_SIZE)) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             } else {
                 ProbeRS232_RxLen   = ProbeRS232_RxLenRem;
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_CTR;
             }
             break;


        case PROBE_RS232_RX_STATE_CTR:                          /* Rx'd the packet counter.                             */
             ProbeRS232_RxPktCtr = rx_data;
             ProbeRS232_RxState  = PROBE_RS232_RX_STATE_PAD;
             break;


        case PROBE_RS232_RX_STATE_PAD:                          /* Rx'd the padding byte.                               */
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_DATA;
             break;


        case PROBE_RS232_RX_STATE_DATA:                         /* Rx'd data.                                           */
             ProbeRS232_RxStoINT8U(rx_data);
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_RxChkSum += rx_data;
#endif
             ProbeRS232_RxLenRem--;
             if (ProbeRS232_RxLenRem == 0) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_CHKSUM;
             }
             break;


        case PROBE_RS232_RX_STATE_CHKSUM:                       /* Rx'd the checksum.                                   */
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_RxChkSum -= rx_data;
             if ((ProbeRS232_RxChkSum & 0xFF) == 0x00) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_ED;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
#else
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_ED;
#endif
             break;


        case PROBE_RS232_RX_STATE_ED:                           /* Rx'd the end delimiter.                              */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_ED) {
                 ProbeRS232_RxPkt();                            /* Parse rx'd pkt.                                      */
             }
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             break;


        default:
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             break;
    }
}


/*
*********************************************************************************************************
*                                       ProbeRS232_TxHandler()
*
* Description : Handle a byte transmission.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Tx ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxHandler (void)
{
    CPU_INT08U  tx_data;


#if (PROBE_COM_CFG_STAT_EN == DEF_ENABLED)
    if ((ProbeRS232_TxState != PROBE_RS232_TX_STATE_SD0) ||
        (ProbeRS232_TxLen    > 0u                      )) {
        ProbeRS232_TxCtr++;
    }
#endif

    switch (ProbeRS232_TxState) {
        case PROBE_RS232_TX_STATE_SD0:
             if (ProbeRS232_TxLen > 0u) {                       /* If pkt is waiting to be sent ...                     */
                 ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD0);   /*  ... tx start 1st start delimiter (SD0).             */
                 ProbeRS232_TxActiveFlag = DEF_TRUE;
                 ProbeRS232_TxState      = PROBE_RS232_TX_STATE_SD1;
                 ProbeRS232_TxBufRdIx    = 0u;

             } else {                                           /* If no pkt is waiting to be sent ...                  */
                 ProbeRS232_TxActiveFlag = DEF_FALSE;
                 ProbeRS232_TxIntDis();                         /*  ... dis tx int's.                                   */
             }
             break;


        case PROBE_RS232_TX_STATE_SD1:                          /* Tx start 2nd start delimiter (SD1).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD1);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_SD2;
             break;


        case PROBE_RS232_TX_STATE_SD2:                          /* Tx start 3rd start delimiter (SD2).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD2);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_SD3;
             break;


        case PROBE_RS232_TX_STATE_SD3:                          /* Tx start 4th start delimiter (SD3).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD3);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_LEN1;
             break;


        case PROBE_RS232_TX_STATE_LEN1:                         /* Tx 1st len byte.                                     */
             tx_data             = ProbeRS232_TxLen & 0xFF;
             ProbeRS232_Tx1(tx_data);
             ProbeRS232_TxState  = PROBE_RS232_TX_STATE_LEN2;
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_TxChkSum = tx_data;
#endif
             break;


        case PROBE_RS232_TX_STATE_LEN2:                         /* Tx 2nd len byte.                                     */
             tx_data              = ProbeRS232_TxLen >> 8;
             ProbeRS232_Tx1(tx_data);
             ProbeRS232_TxState   = PROBE_RS232_TX_STATE_CTR;
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_TxChkSum += tx_data;
#endif
             break;


        case PROBE_RS232_TX_STATE_CTR:                          /* Tx packet counter.                                   */
             tx_data            = ProbeRS232_RxPktCtr;
             ProbeRS232_Tx1(tx_data);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_PAD;
             break;


        case PROBE_RS232_TX_STATE_PAD:                          /* Tx padding byte.                                     */
             ProbeRS232_Tx1(0);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_DATA;
             break;


        case PROBE_RS232_TX_STATE_DATA:                         /* Tx data.                                             */
             tx_data = ProbeRS232_TxBuf[ProbeRS232_TxBufRdIx];
             ProbeRS232_Tx1(tx_data);
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_TxChkSum += tx_data;
#endif
             ProbeRS232_TxBufRdIx++;
             if (ProbeRS232_TxBufRdIx >= ProbeRS232_TxLen) {
                 ProbeRS232_TxState = PROBE_RS232_TX_STATE_CHKSUM;
                 ProbeRS232_TxLen   = 0u;
             }
             break;


        case PROBE_RS232_TX_STATE_CHKSUM:                       /* Tx checksum.                                         */
#if (PROBE_RS232_CHKSUM_EN == DEF_TRUE)
             ProbeRS232_Tx1(ProbeRS232_TxChkSum);
#else
             ProbeRS232_Tx1(0);
#endif
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_ED;
             break;


        case PROBE_RS232_TX_STATE_ED:                           /* Tx end delimiter.                                    */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_ED);
             ProbeRS232_TxState    = PROBE_RS232_TX_STATE_SD0;
             ProbeRS232_TxBufInUse = DEF_FALSE;
             break;


        default:
             ProbeRS232_TxState      = PROBE_RS232_TX_STATE_SD0;
             ProbeRS232_TxActiveFlag = DEF_FALSE;
             ProbeRS232_TxIntDis();                             /* No more data to send, dis tx int's.                  */
             break;
    }
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
*                                       ProbeRS232_ParseRxPkt()
*
* Description : Parse a received packet & formulate a response.
*
* Argument(s) : none.
*
* Return(s)   : The number of bytes in the data segment of the packet to transmit in response.
*
* Caller(s)   : ProbeRS232_Task(),
*               ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_SIZE_T  ProbeRS232_ParseRxPkt (void)
{
    CPU_SIZE_T  tx_len;


    if (ProbeRS232_TxBufInUse == DEF_TRUE) {                    /* Do cmds only if tx buf is free.                      */
        return ((CPU_SIZE_T)0);
    }

    ProbeRS232_TxBufInUse = DEF_TRUE;
    tx_len                = ProbeCom_ParseRxPkt((void       *)ProbeRS232_RxBuf,
                                                (void       *)ProbeRS232_TxBuf,
                                                (CPU_SIZE_T  )ProbeRS232_RxLen,
                                                (CPU_SIZE_T  )PROBE_RS232_CFG_TX_BUF_SIZE);

    return (tx_len);
}


/*
*********************************************************************************************************
*                                         ProbeRS232_RxPkt()
*
* Description : Handle a received packet.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxPkt (void)
{
#if (PROBE_RS232_CFG_PARSE_TASK_EN == DEF_ENABLED)
    ProbeRS232_OS_Post();                                       /* We have a whole pkt, signal task to parse it.        */



#else



    CPU_SIZE_T  len;
    CPU_SR_ALLOC();


    len = ProbeRS232_ParseRxPkt();                              /* We have a whole pkt, parse it.                       */
    if (len > 0u) {
        CPU_CRITICAL_ENTER();
        ProbeRS232_TxLen = len;
        ProbeRS232_TxStart();
        CPU_CRITICAL_EXIT();
    }
#endif
}


/*
*********************************************************************************************************
*                                       ProbeRS232_RxStoINT8U()
*
* Description : Store a byte in the receive buffer.
*
* Argument(s) : rx_data     Byte of data to store in the buffer.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxStoINT8U (CPU_INT08U rx_data)
{
    if (ProbeRS232_RxBufWrIx < PROBE_RS232_CFG_RX_BUF_SIZE) {
        ProbeRS232_RxBuf[ProbeRS232_RxBufWrIx++] = rx_data;
    }
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxBufClr()
*
* Description : Clear the data segment buffer write index.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxBufClr (void)
{
    ProbeRS232_RxBufWrIx = 0u;
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxStart()
*
* Description : Cause transmission to begin.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Task(),
*               ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_TxStart (void)
{
    if (ProbeRS232_TxActiveFlag == DEF_FALSE) {                 /* If no other tx is in progress ...                    */
        ProbeRS232_TxHandler();                                 /*  ... handle tx                ...                    */
        ProbeRS232_TxIntEn();                                   /*  ... en     tx ints.                                 */
    }
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
