/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   USB APPLICATION INITIALIZATION
*
*                                              TEMPLATE
*
* Filename : app_usbd.c
* Version  : V2.30
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   APP_USBD_MODULE
#include  <app_usbd.h>

#if (APP_CFG_PROBE_EN == DEF_ENABLED)
#include  <probe_usb.h>
#endif

#if (APP_CFG_USBD_EN == DEF_ENABLED)
#include  <usbd_core.h>
#include  <usbd_dev_cfg.h>
#include  <usbd_bsp_template.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


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

                                                                /* ---------- USB DEVICE CALLBACKS FUNCTIONS ---------- */
static  void  App_USBD_EventReset  (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventSuspend(CPU_INT08U   dev_nbr);
static  void  App_USBD_EventResume (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventCfgSet (CPU_INT08U   dev_nbr,
                                    CPU_INT08U   cfg_val);
static  void  App_USBD_EventCfgClr (CPU_INT08U   dev_nbr,
                                    CPU_INT08U   cfg_val);
static  void  App_USBD_EventConn   (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventDisconn(CPU_INT08U   dev_nbr);

static  USBD_BUS_FNCTS  App_USBD_BusFncts = {
    App_USBD_EventReset,
    App_USBD_EventSuspend,
    App_USBD_EventResume,
    App_USBD_EventCfgSet,
    App_USBD_EventCfgClr,
    App_USBD_EventConn,
    App_USBD_EventDisconn
};


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           App_USBD_Init()
*
* Description : Initialize USB device stack.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if the stack was created successfully.
*               DEF_FAIL, if the stack could not be created.
*
* Caller(s)   : App_USB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  App_USBD_Init (void)
{
    CPU_INT08U   dev_nbr;
    CPU_INT08U   cfg_hs_nbr;
    CPU_INT08U   cfg_fs_nbr;
    CPU_BOOLEAN  ok;
    USBD_ERR     err;


    APP_TRACE_DBG(("\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("=                    USB DEVICE INITIALIZATION                    =\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("Initializing USB Device...\r\n"));


    USBD_Init(&err);

    if (err!= USBD_ERR_NONE) {
        APP_TRACE_DBG(("... init failed w/err = %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    APP_TRACE_DBG(("    Adding controller driver ... \r\n"));
                                                                     /* Add USB device instance.                           */
    dev_nbr = USBD_DevAdd(&USBD_DevCfg_Template,
                          &App_USBD_BusFncts,
                          &USBD_DrvAPI_Template,
                          &USBD_DrvCfg_Template,
                          &USBD_DrvBSP_Template,
                          &err);

    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add controller driver w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    USBD_DevSelfPwrSet(dev_nbr, DEF_NO, &err);                  /* Device is not self-powered.                          */

    cfg_hs_nbr = USBD_CFG_NBR_NONE;
    cfg_fs_nbr = USBD_CFG_NBR_NONE;

#if (USBD_CFG_HS_EN == DEF_ENABLED)
    APP_TRACE_DBG(("    Adding HS configuration ... \r\n"));

    cfg_hs_nbr = USBD_CfgAdd( dev_nbr,                          /* Add HS configuration.                               */
                              USBD_DEV_ATTRIB_SELF_POWERED,
                              100u,
                              USBD_DEV_SPD_HIGH,
                             "HS configuration",
                             &err);
    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add HS configuration w/err =  %d\r\n\r\n", err));
        if (err != USBD_ERR_DEV_INVALID_SPD) {                  /* Continue if dev is not high-speed.                   */
            return (DEF_FAIL);
        }
    }
#endif

    APP_TRACE_DBG(("    Adding FS configuration ... \r\n"));

    cfg_fs_nbr = USBD_CfgAdd( dev_nbr,                          /* Add FS configuration.                                */
                              USBD_DEV_ATTRIB_SELF_POWERED,
                              100u,
                              USBD_DEV_SPD_FULL,
                             "FS configuration",
                             &err);
    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add FS configuration w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

#if (USBD_CFG_HS_EN == DEF_ENABLED)
    if ((cfg_fs_nbr != USBD_CFG_NBR_NONE) &&
        (cfg_hs_nbr != USBD_CFG_NBR_NONE)) {

        USBD_CfgOtherSpeed(dev_nbr,                             /* Associate both config for other spd desc.            */
                           cfg_hs_nbr,
                           cfg_fs_nbr,
                          &err);
        if (err != USBD_ERR_NONE) {
            APP_TRACE_DBG(("    ... could not associate other speed configuration w/err =  %d\r\n\r\n", err));
            return (DEF_FAIL);
        }
    }
#endif

    APP_TRACE_DBG(("\r\n"));
    APP_TRACE_DBG(("        ===========================================================\r\n"));
    APP_TRACE_DBG(("        Initializing USB classes ... \r\n\r\n"));

#if (APP_CFG_USBD_PROBE_EN == DEF_ENABLED && PROBE_COM_CFG_USB_EN == DEF_ENABLED)
    ok = ProbeUSB_Init(dev_nbr,                                 /* Initialize the uC/Probe USB interface.               */
                       cfg_hs_nbr,
                       cfg_fs_nbr);
    if (ok != DEF_OK) {
        APP_TRACE_DBG(("    ... could not initialize Vendor class for uC/Probe w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }
#endif

    USBD_DevStart(dev_nbr, &err);
    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not start device stack w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    APP_TRACE_DBG(("        .... USB classes initialization done.\r\n"));

   (void)&ok;


    return (DEF_OK);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            USB CALLBACKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        App_USBD_EventReset()
*
* Description : Bus reset event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Reset()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventReset (CPU_INT08U  dev_nbr)
{
    (void)&dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventSuspend()
*
* Description : Bus suspend event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Suspend()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventSuspend (CPU_INT08U  dev_nbr)
{
    (void)&dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventResume()
*
* Description : Bus Resume event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Resume()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr)
{
    (void)&dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgSet()
*
* Description : Set configuration callback event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
*               cfg_nbr     Active device configuration number selected by the host.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_CfgOpen() via 'p_dev->BusFnctsPtr->CfgSet()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)&dev_nbr;
    (void)&cfg_val;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgClr()
*
* Description : Clear configuration event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
*               cfg_nbr     Current device configuration number that will be closed.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_CfgClose via 'p_dev->BusFnctsPtr->CfgClr()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)&dev_nbr;
    (void)&cfg_val;
}


/*
*********************************************************************************************************
*                                        App_USBD_EventConn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Conn()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventConn (CPU_INT08U  dev_nbr)
{
    (void)&dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventDisconn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr     USB device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Disconn()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventDisconn (CPU_INT08U  dev_nbr)
{
    (void)&dev_nbr;
}


/*
*********************************************************************************************************
*                                            USBD_Trace()
*
* Description : Function to output or log USB trace events.
*
* Argument(s) : p_str       Pointer to string containing the trace event information.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_DbgTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_Trace (const  CPU_CHAR  *p_str)
{
    (void)&p_str;
}
#endif

