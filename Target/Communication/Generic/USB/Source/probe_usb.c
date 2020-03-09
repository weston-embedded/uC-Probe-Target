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
*                                         COMMUNICATION: USB
*
* Filename : probe_usb.c
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

#define    PROBE_USB_MODULE
#include  <probe_usb.h>
#include  <usbd_core.h>
#include  <usbd_vendor.h>
#include  <os.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_usb.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_USB_EN == DEF_ENABLED)                       /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          USB PACKET FORMAT
*
* Note(s):  (1) All packets include the following parts:
*
*                   (A)  4 1-byte start delimiters, forming the ASCII representation of "uCPr".  These
*                        are the constants PROBE_USB_PROTOCOL_RX_SD0-PROBE_USB_PROTOCOL_?X_SD4;
*                   (B)  1 2-byte length, the length of the data segment;
*                   (C)  1 2-byte padding, unused;
*                   (D)  n   bytes of data;
*                   (E)  1 1-byte checksum; and
*                   (F)  1 1-byte end delimiter, the character '/', which is the constant PROBE_USB_PROTOCOL_?X_ED.
*
*                                       +-------------------+-------------------+
*                                       |   'u'   |   'C'   |   'P'   |   'r'   |
*                                       +-------------------+-------------------+
*                                       |       Length      |     Padding       |
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
#define  PROBE_USB_PROTOCOL_RX_SD0                      0x75        /* (A) Start delimiters.                                */
#define  PROBE_USB_PROTOCOL_RX_SD1                      0x43
#define  PROBE_USB_PROTOCOL_RX_SD2                      0x50
#define  PROBE_USB_PROTOCOL_RX_SD3                      0x72
#define  PROBE_USB_PROTOCOL_RX_ED                       0x2F        /* (E) End   delimiter.                                 */

                                                                    /* ------------ OUTBOUND PACKET DELIMITERS ------------ */
#define  PROBE_USB_PROTOCOL_TX_SD0                      0x75        /* (A) Start delimiters.                                */
#define  PROBE_USB_PROTOCOL_TX_SD1                      0x43
#define  PROBE_USB_PROTOCOL_TX_SD2                      0x50
#define  PROBE_USB_PROTOCOL_TX_SD3                      0x72
#define  PROBE_USB_PROTOCOL_TX_ED                       0x2F        /* (E) End   delimiter.                                 */


#define  PROBE_USB_CHKSUM_EN                        DEF_FALSE       /* DO NOT CHANGE                                        */


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
static  const  CPU_INT08U  ProbeUSB_MS_PropertyNameGUID[] = {
    'D', 0u, 'e', 0u, 'v', 0u, 'i', 0u, 'c', 0u, 'e', 0u,
    'I', 0u, 'n', 0u, 't', 0u, 'e', 0u, 'r', 0u, 'f', 0u, 'a', 0u, 'c', 0u, 'e', 0u,
    'G', 0u, 'U', 0u, 'I', 0u, 'D', 0u, 0u,  0u
};

static  const  CPU_INT08U  ProbeUSB_MS_GUID[] = {
    '{', 0u, '3', 0u, '2', 0u, '4', 0u, 'D', 0u, 'F', 0u, '0', 0u, '0', 0u, 'B', 0u,
    '-', 0u, '5', 0u, '7', 0u, '1', 0u, 'C', 0u, '-', 0u, '4', 0u, 'B', 0u, '6', 0u, '1', 0u,
    '-', 0u, '9', 0u, '1', 0u, '3', 0u, '9', 0u,
    '-', 0u, 'C', 0u, 'F', 0u, '6', 0u, 'C', 0u, '1', 0u, '5', 0u, '1', 0u, '9', 0u, '1', 0u, '6', 0u, 'E', 0u, '4', 0u,  '}', 0u, 0u, 0u
};
#endif


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

static  CPU_INT08U  ProbeUSB_RxBuf[PROBE_USB_CFG_RX_BUF_SIZE +  4]; /* Buffer size = 2 bytes padding                    */
                                                                    /*             + n bytes data                       */
                                                                    /*             + 1 byte  checksum                   */
                                                                    /*             + 1 byte  end delimiter              */

static  CPU_INT08U  ProbeUSB_TxBuf[PROBE_USB_CFG_TX_BUF_SIZE + 10]; /* Buffer size = 4 bytes start delimiter            */
                                                                    /*             + 2 bytes data length                */
                                                                    /*             + 2 bytes padding                    */
                                                                    /*             + n bytes data                       */
                                                                    /*             + 1 byte  checksum                   */
                                                                    /*             + 1 byte  end delimiter              */


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_INT16U   ProbeUSB_RxPkt    (       CPU_INT08U      class_nbr);
static  void         ProbeUSB_TxPkt    (       CPU_INT08U      class_nbr,
                                               CPU_INT16U      tx_len);
static  CPU_BOOLEAN  ProbeUSB_TargetReq(       CPU_INT08U      class_nbr,
                                        const  USBD_SETUP_REQ *p_setup_req);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           ProbeUSB_Init()
*
* Description : Initialize the uC/Probe USB Communications Module.
*
* Argument(s) : dev_nbr     Device number.
*
*               cfg_hs      Index of high-speed configuration this interface is part of.
*
*               cfg_fs      Index of full-speed configuration this interface is part of.
*
* Return(s)   : DEF_OK,   if the uC/Probe USB communication interface is successfully initialized.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) If PROBE_USB_CFG_INIT_STACK is DEF_TRUE,  then the USB stack is initialized in ProbeUSB_Task().
*                   If          "               is DEF_FALSE, then the USB stack is initialized in the user
*                                                             application and this function merely gets
*                                                             endpoints from the stack and creates an interface.
*********************************************************************************************************
*/

CPU_BOOLEAN  ProbeUSB_Init (CPU_INT08U  dev_nbr,
                            CPU_INT08U  cfg_hs,
                            CPU_INT08U  cfg_fs)
{
    USBD_ERR    err;
    USBD_ERR    err_hs;
    USBD_ERR    err_fs;
    CPU_INT08U  class_nbr;


    err_hs = USBD_ERR_NONE;
    err_fs = USBD_ERR_NONE;

    USBD_Vendor_Init(&err);                                     /* Init Vendor class.                                   */
    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }
                                                                /* Create a Vendor class instance.                      */
    class_nbr = USBD_Vendor_Add(DEF_FALSE, 0u, (USBD_VENDOR_REQ_FNCT)ProbeUSB_TargetReq, &err);
    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }

    if (cfg_hs != USBD_CFG_NBR_NONE) {
                                                                /* Add vendor class to HS dflt cfg.                     */
        USBD_Vendor_CfgAdd(class_nbr, dev_nbr, cfg_hs, &err_hs);
    }

    if (cfg_fs != USBD_CFG_NBR_NONE) {
                                                                /* Add vendor class to FS dflt cfg.                     */
        USBD_Vendor_CfgAdd(class_nbr, dev_nbr, cfg_fs, &err_fs);
    }

    if ((err_hs != USBD_ERR_NONE) &&                            /* If HS and FS cfg fail, stop class init.              */
        (err_fs != USBD_ERR_NONE)) {
        return (DEF_FAIL);
    }

    ProbeUSB_OS_Init(class_nbr);                                /* Create the USB task and other kernel objects.        */

#if (USBD_CFG_MS_OS_DESC_EN == DEF_ENABLED)
    USBD_DevSetMS_VendorCode(dev_nbr, 1u, &err);
    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }

    USBD_Vendor_MS_ExtPropertyAdd(class_nbr,
                                  USBD_MS_OS_PROPERTY_TYPE_REG_SZ,
                                  ProbeUSB_MS_PropertyNameGUID,
                                  sizeof(ProbeUSB_MS_PropertyNameGUID),
                                  ProbeUSB_MS_GUID,
                                  sizeof(ProbeUSB_MS_GUID),
                                 &err);
    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }
#endif


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
*                                           ProbeUSB_Task()
*
* Description : Task for USB Communication Module.
*
* Argument(s) : class_nbr   Vendor class instance number.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_OS_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeUSB_Task (CPU_INT08U  class_nbr)
{
    CPU_BOOLEAN  conn;
    OS_ERR       os_err;
    CPU_INT16U   rx_len;
    CPU_INT16U   tx_len;


    while (DEF_TRUE) {
                                                                /* Wait for cfg activated by host.                      */
        conn = USBD_Vendor_IsConn(class_nbr);
        while (conn != DEF_YES) {
            OSTimeDlyHMSM(0, 0, 0, 250, OS_OPT_TIME_HMSM_STRICT, &os_err);

            conn = USBD_Vendor_IsConn(class_nbr);
        }

        rx_len = ProbeUSB_RxPkt(class_nbr);

        if (rx_len != 0) {
                                                                /* Parse pkt & formulate a response.                    */
            tx_len = ProbeCom_ParseRxPkt((void     *)(ProbeUSB_RxBuf + 2),
                                         (void     *)(ProbeUSB_TxBuf + 8),
                                         (CPU_INT16U)rx_len,
                                         (CPU_INT16U)PROBE_USB_CFG_TX_BUF_SIZE);

            ProbeUSB_TxPkt(class_nbr, tx_len);                  /* Tx the response.                                     */
        }
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
*                                          ProbeUSB_RxPkt()
*
* Description : Receive a packet and strip off header.
*
* Argument(s) : class_nbr   Vendor class instance number.
*
* Return(s)   : 0                        if an error occurred.
*               Number of bytes received if no error occurred.
*
* Caller(s)   : ProbeUSB_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeUSB_RxPkt (CPU_INT08U  class_nbr)
{
    CPU_INT16U  nbr_bytes_rx;
    CPU_INT16U  len;
    USBD_ERR    err;
#if (PROBE_USB_CHKSUM_EN == DEF_TRUE)
    CPU_INT16U  i;
    CPU_INT08U  chk_sum;
#endif


                                                                /* Rd start delims.                                     */
    nbr_bytes_rx = USBD_Vendor_Rd(         class_nbr,
    		                      (void *)&ProbeUSB_RxBuf[0],
    		                               4,
    		                               PROBE_USB_CFG_RX_TIMEOUT_MS,
    		                              &err);

                                                                /* Verify start delims.                                 */
    if ((ProbeUSB_RxBuf[0] == PROBE_USB_PROTOCOL_RX_SD0) &&
        (ProbeUSB_RxBuf[1] == PROBE_USB_PROTOCOL_RX_SD1) &&
        (ProbeUSB_RxBuf[2] == PROBE_USB_PROTOCOL_RX_SD2) &&
        (ProbeUSB_RxBuf[3] == PROBE_USB_PROTOCOL_RX_SD3)) {

                                                                /* Rd len of pkt to rx.                                 */
        nbr_bytes_rx = USBD_Vendor_Rd(         class_nbr,
        		                      (void *)&ProbeUSB_RxBuf[0],
        		                               2,
        		                               PROBE_USB_CFG_RX_TIMEOUT_MS,
        		                              &err);

        if (nbr_bytes_rx != 2) {
            return (0);
        }

        len          = (CPU_INT16U)(ProbeUSB_RxBuf[0] << 0)
                     | (CPU_INT16U)(ProbeUSB_RxBuf[1] << 8);

                                                                /* Verify pkt size.                                     */
        if ((len != 0                        ) &&
            (len <= PROBE_USB_CFG_RX_BUF_SIZE)) {

                                                                /* Rx the rem of pkt = 2   padding bytes                */
                                                                /*                   + len data    bytes                */
                                                                /*                   + 2   chksum  bytes                */
            nbr_bytes_rx = USBD_Vendor_Rd(         class_nbr,
            		                      (void *)&ProbeUSB_RxBuf[0],
            		                      (CPU_INT32U)(len + 4),
            		                      PROBE_USB_CFG_RX_TIMEOUT_MS,
            		                              &err);

            if (nbr_bytes_rx != len + 4) {
                return (0);
            }

#if (PROBE_USB_CHKSUM_EN == DEF_TRUE)
                                                                /* Calc chksum.                                         */
            chk_sum = ProbeUSB_RxBuf[0] + ProbeUSB_RxBuf[1];

            for (i = 0; i < len; i++) {
                chk_sum += ProbeUSB_RxBuf[i + 2];
            }
                                                                /* Verify chksum.                                       */
            if ((ProbeUSB_RxBuf[len + 2] == chk_sum                 ) &&
                (ProbeUSB_RxBuf[len + 3] == PROBE_USB_PROTOCOL_RX_ED)) {
#else
            if ((ProbeUSB_RxBuf[len + 3] == PROBE_USB_PROTOCOL_RX_ED)) {
#endif
                return (len);                                   /* Rtn the nbr of bytes of data.                        */
            } else {
                return (0);
            }
        } else {
            return (0);
        }
    } else {
        return (0);
    }
}


/*
*********************************************************************************************************
*                                             ProbeUSB_TxPkt()
*
* Description : Add header to data and transmit packet.
*
* Argument(s) : tx_len      Number of bytes of data to transmit.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeUSB_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeUSB_TxPkt (CPU_INT08U class_nbr, CPU_INT16U tx_len)
{
    USBD_ERR    err;
#if (PROBE_USB_CHKSUM_EN == DEF_TRUE)
    CPU_INT16U  i;
    CPU_INT08U  chk_sum;
#endif

                                                                /* (a) Start delimiters                                 */
    ProbeUSB_TxBuf[0] = PROBE_USB_PROTOCOL_TX_SD0;
    ProbeUSB_TxBuf[1] = PROBE_USB_PROTOCOL_TX_SD1;
    ProbeUSB_TxBuf[2] = PROBE_USB_PROTOCOL_TX_SD2;
    ProbeUSB_TxBuf[3] = PROBE_USB_PROTOCOL_TX_SD3;

                                                                /* (b) Two-byte packet length                           */
    ProbeUSB_TxBuf[4] = tx_len & 0xFF;                          /* ... Lower byte                                       */
    ProbeUSB_TxBuf[5] = tx_len >> 8;                            /* ... Upper byte                                       */

                                                                /* (c) Padding                                          */
    ProbeUSB_TxBuf[6] = 0;
    ProbeUSB_TxBuf[7] = 0;
                                                                /* (d) Data                                             */
#if (PROBE_USB_CHKSUM_EN == DEF_TRUE)
    chk_sum           = ProbeUSB_TxBuf[4] + ProbeUSB_TxBuf[5];
    for (i = 0; i < tx_len; i++) {
        chk_sum += ProbeUSB_TxBuf[i + 8];
    }
    ProbeUSB_TxBuf[tx_len + 8] = chk_sum;                       /* (e) Checksum                                         */
#else
    ProbeUSB_TxBuf[tx_len + 8] = 0;                             /* (e) Checksum                                         */
#endif


    ProbeUSB_TxBuf[tx_len + 9] = PROBE_USB_PROTOCOL_TX_ED;      /* (f) End delimeter                                    */

                                                                /* (g) Transmit the response                            */
    (void)USBD_Vendor_Wr(         class_nbr,
                         (void *)&ProbeUSB_TxBuf[0],
                                  tx_len + 10U,
                                  PROBE_USB_CFG_TX_TIMEOUT_MS,
                                  DEF_FALSE,
                                 &err);
}


/*
*********************************************************************************************************
*                                        ProbeUSB_TargetReq()
*
* Description : Process target requests.
*
* Argument(s) : class_nbr       Class instance number.
*
*               p_setup_req     Pointer to setup request structure.
*
* Return(s)   : DEF_OK,   if NO error(s) occurred and request is supported.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Vendor class.
*
* Note(s)     : (1) USB Spec 2.0, section 9.3 'USB Device Requests' specifies the Setup packet format.
*
*               (2) The customer may process inside this callback his proprietary vendor-specific
*                   requests decoding.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  ProbeUSB_TargetReq (       CPU_INT08U       class_nbr,
                                         const  USBD_SETUP_REQ  *p_setup_req)
{
    (void)&class_nbr;
    (void)&p_setup_req;

    return (DEF_FAIL);                                          /* Indicate request is not supported.                   */
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
