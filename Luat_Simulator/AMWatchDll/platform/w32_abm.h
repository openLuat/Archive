/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    w32_abm.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          This file is intends for creating ABM module.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#if !defined(__W32_ABM__)
#define __W32_ABM__

#define ABM_MAX_APP_ACCT_NUM        4

#define ABM_MAX_REPORT_BEARER_NUM   (3)

#define ABM_DEFAULT_QOS     (0)    /* ref. SOC_DEFAULT_QOS_PROFILE */
#define ABM_NULL_BEARER     (0)    /* for message to SOC when no bearer context exists */

/* all network account id */
#define CBM_ALL_NWK_ACCT_ID     (0x7f)
/* all qos id */
#define CBM_ALL_QOS_ID          (0xff)
/* invalid module id */
#define CBM_INVALID_MOD_ID  (0)
/* invalid application id */
#define CBM_INVALID_APP_ID  (0)

/* max account number in 32-bits account id */
#define CBM_MAX_ACCT_NUM        (2)

/* sim id */
typedef enum
{
	CBM_SIM_ID_SIM1, /* sim card one */
	CBM_SIM_ID_SIM2, /* sim card two */
	CBM_SIM_ID_SIM3, /* sim card there */
	CBM_SIM_ID_SIM4, /* sim card four */
	CBM_SIM_ID_TOTAL /* total sim card number */
} cbm_sim_id_enum;

typedef enum
{
	//ABM error causes    
	ABM_CAUSE_START = 3272,
	ABM_E_SUCCESS,
	ABM_E_INVALID_NW_ACC_ID,
	ABM_E_GPRS_REACTIVATE,
	ABM_E_GPRS_PROTOCOL_REJECTION,
	ABM_E_CSD_REACTIVATE,
	ABM_E_CSD_PPP_NEGOTIATED_FAILED,
	ABM_E_CSD_ACT_FAILED,
	ABM_E_CSD_CALL_SETUP_FAILED,
	ABM_E_WIFI_REACTIVATE,
	ABM_E_WIFI_NOTREADY,
	ABM_E_REJECTED,
	ABM_E_SLOT_LIMITED,
	ABM_E_ABORT,
	ABM_E_NON_AUTO_DEACT,
	ABM_E_BEARER_IS_HOLD, /* 3287 */
}ps_cause_enum;

/* Bearer type enum */
typedef enum 
{
	ABM_E_SOCKET_BEARER_IP_ANY,
	ABM_E_SOCKET_BEARER_GSM_CSD,
	ABM_E_SOCKET_BEARER_GSM_GPRS,
	ABM_E_SOCKET_BEARER_WIFI
} abm_bearer_enum;

typedef enum
{
	MMI_ABM_BEARER_EVENT_CANCEL,
	MMI_ABM_BEARER_EVENT_ACCEPT_THIS_TIME,
	MMI_ABM_BEARER_EVENT_TOTAL
} mmi_abm_bearer_event_result_enum;

typedef enum
{
	MMI_ABM_BEARER_EVT_ALWAYS_ASK,
	MMI_ABM_BEARER_EVT_FALLBACK,
	MMI_ABM_BEARER_EVT_WIFI_PS_FALLBACK,
	MMI_ABM_BEARER_EVT_DISCONNECT,
	MMI_ABM_BEARER_EVT_SWITCH,
	MMI_ABM_BEARER_EVT_TOTAL
} mmi_abm_bearer_event_enum;

/* Whether bearer is activated or deactivated by socket or not */
typedef enum
{
	ABM_STATE_NULL = 0x00,
	ABM_STATE_SOC_ACTIVATE = 0x01, /* Activate by SOC */
	ABM_STATE_SOC_DEACTIVATE = 0x02, /* Deactivate by SOC */
	ABM_STATE_MMI_CNMGR_DEACTIVATE = 0x04, /* Deactivate by MMI connection management */
	ABM_STATE_MMI_APP_DEACTIVATE = 0x08, /* Deactivate APP bearer by MMI */
	ABM_STATE_MMI_NON_APP_DEACTIVATE = 0x10, /* Deactivate non-app bearer by MMI */
	ABM_STATE_MMI_CBM_DEACTIVATE = 0x20, /* Deactivate by MMI connection management */
	ABM_STATE_TOTAL
} abm_bearer_state_enum;

/* bearer type */
typedef enum
{
    CBM_BEARER_NONE, /* no valid bearer */
    CBM_CSD = 0x01, /* CSD bearer */
    CBM_BEARER_GSM_CSD = CBM_CSD, /* CSD bearer */
    CBM_GPRS = 0x02, /* GPRS bearer */
    CBM_BEARER_GSM_GPRS = CBM_GPRS, /* GPRS bearer */
    CBM_EDGE = 0x04, /* EDGE bearer */
    CBM_BEARER_EDGE = CBM_EDGE, /* EDGE bearer */
    
    CBM_WIFI = 0x08, /* WIFI bearer */
    CBM_BEARER_WIFI = CBM_WIFI, /* WIFI bearer */

    CBM_WIFI_HS = 0x10, /* WIFI hot spot bearer */
    CBM_BEARER_HOTSPOT = CBM_WIFI_HS, /* WIFI hot spot bearer */
    
    CBM_USB_TR  = 0x20, /* USB Tethering */
    CBM_BEARER_USB_TR = CBM_USB_TR, /* USB Tethering */
    
    CBM_LOOPBACK = 0x40, /* loopback */
    CBM_BEARER_LOOPBACK = CBM_LOOPBACK, /* loopback */

    CBM_UMTS = 0x80, /* 3G UMTS (WCDMA) bearer */
    CBM_BEARER_UMTS = CBM_UMTS, /* 3G UMTS (WCDMA) bearer */


    CBM_3G = CBM_UMTS, /* Not real activated bearer type. This is 3G general name.
                          It contains all 3G bearer types, such as WCDMA, STDWCDMA, etc. */
    CBM_BEARER_3G = CBM_UMTS, /* Not real activated bearer type. This is 3G general name.
                                 It contains all 3G bearer types, such as WCDMA, STDWCDMA, etc. */

    CBM_PS  = CBM_GPRS | CBM_EDGE | CBM_BEARER_3G, /* Not real activated bearer type. This is packet service general name.
                                                      It contains all packet service types, such as GPRS, EDGE, UMTS, etc. */
    CBM_BEARER_PS = CBM_PS, /* Not real activated bearer type. This is packet service general name.
                               It contains all packet service types, such as GPRS, EDGE, UMTS, etc. */
                               
    CBM_BEARER_TOTAL = 0xff
} cbm_bearer_enum;

typedef enum
{
	/* no bearer event */
	CBM_APP_BEARER_EVT_NONE = 0,
	/* always ask event */
	CBM_APP_BEARER_EVT_ALWAYS_ASK = 0x01,
	/* bearer fallback level one event */
	CBM_APP_BEARER_EVT_FB_L1       = 0x02, 
	/* bearer fallback level one with new connection event */
	CBM_APP_BEARER_EVT_FB_L1_WITH_NEW_CONN           = 0x04, 
	/* support non-auto bearer fallback level two with new connection */
	CBM_APP_BEARER_EVT_FB_L2_WITH_NEW_CONN   = 0x08, 
	/* support wifi to gprs fallback */
	CBM_APP_BEARER_EVT_FB_WIFI_TO_PS = 0x10,
	CBM_APP_BEARER_EVT_SWITCH = 0x20,
	CBM_APP_BEARER_EVT_DISCONNECT = 0x40,
	CBM_APP_BEARER_EVT_TOTAL
} cbm_app_bearer_event_enum;

typedef enum
{
	ABM_APP_BEARER_FB_L1                    = 0x0001,
	ABM_APP_BEARER_FB_L2                    = 0x0002,
	ABM_APP_ALWAYS_ASK                      = 0x0004,
	/* Need report the notification to AP */
	ABM_APP_ACCT_NEED_NOTIFY                = 0x0008,
	/* Hold the bearer deactivation state */
	ABM_APP_BEARER_DELAY_DISC               = 0x0010,
	/* The account query is canceled in always-ask case */
	ABM_APP_CANCEL_ACCT_QUERY               = 0x0020,
	/* CSD timer is expired and we need ABM to report the CSD auto time-out info */
	ABM_APP_CSD_AUTO_DISC_EXPIRE            = 0x0040,
	ABM_APP_GPRS_AUTO_DISC_EXPIRE           = 0x0080,
	ABM_APP_NON_AUTO_DEACT                  = 0x0100,
	ABM_APP_ACCT_HAS_SELECTED               = 0x0200,
	ABM_APP_ABORT_NWK_SRV                   = 0x0400,
	ABM_APP_STATUS_TOTAL
} abm_app_status_enum;

typedef struct
{
	UINT32 type;
} abm_app_report_mod_id;

/* account profile */
typedef struct
{
	BOOL                is_always_ask; /* is always_ask account or not */
	cbm_sim_id_enum         sim_id; /* sim card id. it could be SIM one or SIM two */
	UINT8               account_id; /* real account id */
} cbm_acct_profile_struct;

/* account info */
typedef struct
{
	cbm_acct_profile_struct account[CBM_MAX_ACCT_NUM]; /* account profile */
	UINT8               acct_num; /* number of real accounts in 32-bits account id */
	UINT8               app_id; /* application identification */
} cbm_account_info_struct;

/* Application id */
typedef struct
{
	abm_app_report_mod_id report_mod_id[ABM_MAX_REPORT_BEARER_NUM]; /* module id set by application */
	UINT8   ap_id; /* application id */
} abm_app_id_struct;

/* ABM application pool */
typedef struct
{
#ifdef __ABM_DISALLOW_NWK_SRV_SUPPRT__
	UINT32          allow_bearer;
#endif
	UINT32          app_event; /* application current events */
	UINT32          ori_account_id[ABM_MAX_APP_ACCT_NUM]; /* original account id: account id + ap id */ 

	UINT32           account_id[ABM_MAX_APP_ACCT_NUM]; /* sim id+ account id */
	abm_app_id_struct   app; /* application id */
	abm_bearer_state_enum  app_state[ABM_MAX_APP_ACCT_NUM]; /* bearer is activated or deactivated by SOC */

	UINT8           account_state[ABM_MAX_APP_ACCT_NUM]; /* application's account state */    
	UINT8           reg_state; /* application's registered bearer status */
	UINT8           reg_type;  /* application's registered bearer type */
#ifdef __APP_SECONDARY_PDP_SUPPORT__
	UINT8           qos_profile_id[ABM_MAX_APP_ACCT_NUM]; /* qos profile id*/
	UINT8           context_id[ABM_MAX_APP_ACCT_NUM]; /* context id */
#endif /* __APP_SECONDARY_PDP_SUPPORT__ */

#ifdef __CBM_BEARER_SWITCH__
	UINT8            cur_fb_status;
	UINT8           next_acct_num; /* next account id whcih will be used to activate the bearer */
#endif /* __CBM_BEARER_SWITCH__ */

#if defined(__APP_STATIC_QOS_PROFILE_SUPPORT__) || defined(__APP_DYNAMIC_QOS_PROFILE_SUPPORT__)
	UINT8                               qos_profile_idx; /* qos profile index */
	nvram_ef_abm_ps_qos_profile_struct      *qos_profile;
#endif /* defined(__APP_STATIC_QOS_PROFILE_SUPPORT__) || defined(__APP_DYNAMIC_QOS_PROFILE_SUPPORT__) */
} abm_app_proc_struct;

typedef struct
{
	UINT8                           ref_count;
	UINT16                          msg_len;
	UINT32                          ori_acct_id;
	UINT32                          sel_acct_id;
	mmi_abm_bearer_event_result_enum    user_result;
	mmi_abm_bearer_event_enum           event;
} mmi_abm_bearer_event_connect_req_struct;

typedef struct
{
	UINT8                           ref_count;
	UINT16                          msg_len;
	UINT32                          ori_acct_id;
	BOOL                            result;
	mmi_abm_bearer_event_enum           event;
} mmi_abm_bearer_event_connect_cnf_struct;

/* Application has to return response once it receive the APP_CBM_BEARER_EVENT_NOTIFY_IND message.
 */
typedef struct
{
    UINT8                 ref_count;
    UINT16                msg_len;
    UINT32                account_id; /* network account id contains multiple info. 
                                             For example, app_id is encoded in the account id */
    BOOL                  result;     /* TRUE: accept bearer event. FALSE: reject bearer event */
    cbm_app_bearer_event_enum event;      /* notification event. This describe which event has occured.
                                             For example: if the primitive is sent out due to the occurance
                                             of bearer fallback level two event, 
                                             the value put here would be CBM_APP_BEARER_EVT_FB_L2_WITH_NEW_CONN */
    
} app_cbm_bearer_event_notify_rsp_struct;

typedef struct
{
	UINT32  ipsec_spi;            /* IPSec */
	UINT32  flow_label;           /* IPv6 */

	UINT16  remote_start_port;    /* inclusive */
	UINT16  remote_end_port;      /* inclusive */
	UINT16  local_start_port;
	UINT16  local_end_port;

	UINT8   pfi;                  /* filter id */
	UINT8   epi;                  /* evaluation precedence */
	UINT8   protocol;             /* TCP, UDP, ... */
	UINT8   remote_addr[4]; 
	UINT8   remote_addr_prefix_len;     /* 0..32 */
	UINT8   tos_mask;             /* set to 0 if not required to exam ToS */
	UINT8   tos;
} soc_abm_dl_filter_struct  ;

typedef struct
{
	UINT8   ref_count;
	UINT16  msg_len;	
	UINT32  account_id; /* 0xff: all bearers */
	UINT8   qos_profile_id;
} app_soc_deactivate_req_struct;

typedef struct {
	UINT8 ref_count;
	UINT16 msg_len;

	UINT32 nw_acc_id; /* Network account ID */    
	UINT16 bandwidth;
	UINT16 cause;

	soc_abm_dl_filter_struct  dl_filter[8];

	UINT8  qos_profile_id;      /* >0: implies qos_required */
	UINT8  app_type;       /* app type to call encode function */
	UINT8  dl_filter_num;
} soc_abm_activate_req_struct;

typedef struct {
	UINT8 ref_count;
	UINT16 msg_len;

	UINT32 nw_acc_id;
	UINT32 activate_acct_id;

	UINT16 err_cause;
	UINT16  cause;

	BOOL result;
	BOOL   qos_fulfilled;

	cbm_bearer_enum bearer;
	UINT8  qos_profile_id;
	UINT8  bearer_ctx_id;
	UINT8 ip_addr[4];
	UINT8 pri_dns_addr[4];
	UINT8 sec_dns_addr[4];
	UINT8 gateway[4];
	UINT8 netmask[4];
	UINT8 mac_addr[6];
	UINT16 mtu;
	UINT8 dhcp_state;
} soc_abm_activate_cnf_struct;

typedef struct {
	UINT8 ref_count;
	UINT16 msg_len;

	UINT32 nw_acc_id; /* Network account ID */ 
	UINT16 cause;
	UINT8  qos_profile_id;
	UINT8  bearer_ctx_id;
} soc_abm_deactivate_req_struct;

typedef struct {
	UINT8 ref_count;
	UINT16 msg_len;

	UINT32 nw_acc_id;
	UINT16 err_cause;
	UINT16  cause;
	BOOL result;
	UINT8  qos_profile_id;
	UINT8  bearer_ctx_id;
} soc_abm_deactivate_cnf_struct;

#ifndef __NWK_APP_UT__

INT8 cbm_hold_bearer(UINT8 app_id);

INT8 cbm_release_bearer(UINT8 app_id);

INT8 cbm_get_bearer_type(UINT32 account_id, cbm_bearer_enum *bearer_type);

INT32 cbm_get_bearer_status(UINT32 account_id);

#endif /* __NWK_APP_UT__ */

#endif //__W32_ABM__