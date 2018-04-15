/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    mthl.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          MTHL.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#if !defined(__MTHL_H__)
#define __MTHL_H__

#include "platform_uart.h"
#include "platform_rtos.h"
#include "w32_socket.h"

#include "Winsock2.h"

#define LUA_SOCKET_THREAD_STACK_SIZE (10*1024)

#define LUA_MAX_SOCKET_SUPPORT 10

#define LUA_SOCKET_RX_BUF_SIZE (8*1024+1) //8KB

#define LUA_INVALID_SOKCET_ID (0xffffffff)

#define LUA_INVALID_SOCKET_INDEX (0xffffffff)

#define LUA_INVALID_PARAM  (-1)

#define LUA_SOCKET_ACCT_ID (CBM_MRE_ACCT_ID)

#define MAX_GET_HOST_BY_NAME_NUM   5

#define MTHL_MAX_IP_SOCKET_NUM      (MAX_IP_SOCKET_NUM -1)

#define MAX_CTRL_PKT_SIZE                    256

typedef enum
{
     IPV4_ADDR_TYPE = 0x21,
     IPV6_ADDR_TYPE = 0x57,
     PPP_ADDR_TYPE  = 0x01,
     OSP_IHOSS_ADDR_TYPE = 0x02,
     NULL_PDP_ADDR_TYPE = 0x03 /* This is added incase if no pdpaddrtype is 
                                  specified */
} pdp_addr_type_enum;

/* Data Account GRPS profile authentication type enum */
typedef enum
{
	CUSTOM_DTCNT_PROF_GPRS_AUTH_TYPE_NORMAL,       /* auth. type : normal */
	CUSTOM_DTCNT_PROF_GPRS_AUTH_TYPE_SECURE,       /* auth. type : secure */
	CUSTOM_DTCNT_PROF_GPRS_AUTH_TYPE_TOTAL    
} custom_dtcnt_prof_gprs_auth_type_enum;

typedef enum
{
	PDP_NONE,
	PDP_ACTIVATE_REQ,
	PDP_ACTIVATE_CNF,
	PDP_DEACTIVATE_IND,
	PDP_DEACTIVATE_REQ,
	PDP_DEACTIVATE_CNF,
}pdp_bear_op_enum;

typedef enum
{
	SUCCESS,
	PARAM_INVALID,
	LIMITED_RES,	
} mthl_create_pdp_param_result;

typedef enum
{
   TRANSPARENT_ESC_DETECT,
   TRANSPARENT_PLUGOUT_DETECT,
   TRANSPARENT_NETWORK_ERROR,
   TRANSPARENT_ERROR_CAUSE_ENUM_SIZE
} transparent_error_cause_enum;

typedef struct
{    
	custom_dtcnt_prof_gprs_auth_type_enum  AuthType;   /* GPRS Authentication Type */
	UINT8 UserName[CUSTOM_DTCNT_PROF_MAX_USER_LEN+1];     /* User string of a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_USER_LEN) */
	UINT8 Passwd[CUSTOM_DTCNT_PROF_MAX_PW_LEN+1];         /* Password for a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_PW_LEN) */
} mthl_custom_dtcnt_prof_auth_struct;

typedef struct
{
	UINT32              account_id;
	pdp_addr_type_enum      pdp_type;        /*pdp_type_enum*/
	UINT8               apn_length;
	UINT8               apn [ MAX_APN_LEN ];
	UINT16              proxy_port;       
	UINT8               addr_length;
	UINT8               proxy_addr[ MAX_PDP_ADDR_LEN ];
	UINT32              user_data;
} mthl_create_pdp_param_req_struct;

typedef struct
{
	BOOL       result;
	UINT32     error_cause;
	UINT32     account_id;
	UINT32     user_data;
} mthl_activate_pdp_cnf_struct;

typedef struct
{    
	custom_dtcnt_prof_gprs_auth_type_enum  AuthType;   /* GPRS Authentication Type */
	UINT8 UserName[CUSTOM_DTCNT_PROF_MAX_USER_LEN+1];     /* User string of a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_USER_LEN) */
	UINT8 Passwd[CUSTOM_DTCNT_PROF_MAX_PW_LEN+1];         /* Password for a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_PW_LEN) */
} custom_dtcnt_prof_auth_struct;

typedef struct
{
	UINT32     account_id;
	custom_dtcnt_prof_auth_struct Auth_info;       /* authentication info */
	UINT32     user_data;
} mthl_activate_pdp_req_struct;

typedef struct
{
	UINT32     account_id;
	BOOL       result;
	UINT32     error_cause;
	UINT32     user_data;
} mthl_deactivate_pdp_cnf_struct;

typedef struct
{
	UINT32     account_id;
	UINT32     user_data;
} mthl_deactivate_pdp_req_struct;

typedef struct
{ 
	UINT32 account_id;
	cbm_result_error_enum error;
	UINT32 error_cause;
} mthl_deactivate_pdp_ind_struct;

typedef struct
{ 
	UINT8 domain_len;
	char domain_name[128+1];
	UINT32    user_data;
} mthl_get_host_by_name_req_struct;

typedef struct
{ 
	UINT16 addr_len; /* address length */
	UINT8 addr[MAX_SOCK_ADDR_LEN];
	UINT32 user_data;
} mthl_get_host_by_addr_req_struct;

typedef struct
{  
	BOOL     result;//ÊÇ·ñ³É¹¦£»(-2)
	UINT8    domain_len;
	CHAR     domain_name[128+1];
	INT16	addr_len; /* address length */
	UINT8	addr[MAX_SOCK_ADDR_LEN];
	UINT32   user_data;
	UINT32   account_id;
} mthl_get_host_by_name_cnf_struct, mthl_get_host_by_addr_cnf_struct, mthl_get_host_by_name_ind_struct, mthl_get_host_by_addr_ind_struct;

typedef struct
{
	UINT32 account_id;
	sockaddr_struct addr; //include socket_type
	UINT32 user_data;
} mthl_create_conn_req_struct;

typedef struct
{
	INT8 sock_id;
	sockaddr_struct   addr;
	UINT32    user_data;
} sock_account_info_struct;

typedef struct
{
   INT8 sock_id;
   UINT64 total_send_size;
   UINT64 Ack_value;
   UINT64 total_recv_size;
}mthl_send_recv_soc_save_struct;

typedef struct
{
  UINT32 account_id;
  UINT8 sock_num;

  sock_account_info_struct socket_info[MTHL_MAX_IP_SOCKET_NUM]; //type, id, ip, port
  mthl_send_recv_soc_save_struct socket_statistics_info[MTHL_MAX_IP_SOCKET_NUM];
  UINT32    user_data;
} mthl_create_conn_soc_save_struct;

typedef struct
{
	UINT32 account_id;
	BOOL result;
	UINT8 sock_id;
	UINT32    user_data;
} mthl_create_conn_cnf_struct;

typedef struct
{
	UINT32 account_id;
	UINT8 sock_id;
	UINT32    user_data;
} mthl_create_sock_ind_struct;

typedef struct
{
	mthl_create_pdp_param_result    result;
	UINT32     account_id;
	UINT32     user_data;
} mthl_create_pdp_param_cnf_struct;

typedef struct
{
	UINT8  sock_id;
	UINT8* data;
	INT32 size;
	UINT8 flags;
	sockaddr_struct addr;
	UINT32    user_data;
} mthl_send_data_req_struct;

typedef struct
{
	UINT8 sock_id;
	BOOL result;
	INT32 ret_val;
	UINT32    user_data;
} mthl_send_data_cnf_struct, mthl_send_data_ind_struct;

typedef struct
{ 
	UINT8 sock_id;
	UINT8 flags;
	UINT32    user_data;
} mthl_recv_data_req_struct;

typedef struct
{ 
	UINT8 sock_id;
	BOOL result;
	UINT8* buff;
	INT32 ret_val;
	sockaddr_struct addr;
	UINT32    user_data;
} mthl_recv_data_cnf_struct, mthl_recv_data_ind_struct;

typedef struct
{ 
	BOOL result;
	UINT8 sock_id;
	UINT32    user_data;
} mthl_close_sock_ind_struct;

typedef struct
{
	BOOL result;
	UINT8 sock_id;
	UINT32    user_data;
} mthl_close_sock_cnf_struct;

typedef struct
{ 
	UINT8 sock_id;
	UINT32 user_data;
} mthl_close_sock_req_struct;

//TCP only
typedef struct
{
	UINT32  account_id;
	UINT8	addr_len; /* address length */
	UINT8	addr[MAX_SOCK_ADDR_LEN];
	UINT16  port;
	UINT32   user_data;
} mthl_server_launch_req_struct;

typedef struct
{
	UINT8 result;
	INT8 sock_id;
	UINT32 user_data;
} mthl_server_listen_cnf_struct;

typedef struct
{
	UINT8 conn_sock_id;
	sockaddr_struct addr;
} mthl_server_accept_ind_struct;

typedef struct
{
	UINT32 account_id;
	UINT8 pdp_type;
	UINT8 apn_length;
	UINT8 apn[ MAX_APN_LEN ];
	UINT16 proxy_port;
	UINT8 proxy_len;
	UINT8 proxy_addr[ MAX_PDP_ADDR_LEN ];
} mthl_pdp_param_info_struct;

typedef struct
{
	UINT8 result;
	UINT8 pdp_param_num;
	mthl_pdp_param_info_struct pdp_param_info[3];
	UINT32 user_data;
} mthl_query_pdp_param_cnf_struct;

typedef struct
{
	UINT32 user_data;
} mthl_query_pdp_param_req_struct;

typedef struct
{
	UINT8   result;
	UINT8   num;
	UINT8   state_list[3][2];
	UINT32  user_data;
} mthl_query_pdp_state_cnf_struct;

typedef struct
{
	UINT32 user_data;
} mthl_query_pdp_state_req_struct;

typedef struct
{
	UINT32              account_id;
	UINT32              nwk_acct_id;
	pdp_addr_type_enum      pdp_type;        /*pdp_type_enum*/
	UINT8               apn_length;
	UINT8               apn [ MAX_APN_LEN ];
	UINT16              proxy_port;       
	UINT8               proxy_addr[ MAX_PDP_ADDR_LEN ];
	UINT8               proxy_flags;
	custom_dtcnt_prof_auth_struct Auth_info;       /* authentication info */
	pdp_bear_op_enum                 bear_op;
	UINT32              user_data;
}mthl_create_pdp_param_save_struct;

typedef struct
{
	UINT32	account_id;
	UINT32	user_data;
} mthl_query_sock_account_info_req_struct;

typedef struct
{
	BOOL result;
	UINT32 account_id;
	UINT8 sock_num;
	sock_account_info_struct socket_info[MTHL_MAX_IP_SOCKET_NUM]; //type, id, ip, port
	UINT32    user_data;
} mthl_query_sock_account_info_cnf_struct;

typedef struct
{ 
	INT8 sock_id;
	UINT32 user_data;
} mthl_query_sock_info_req_struct;

typedef struct
{ 
	INT8 sock_id;
	BOOL result;
	UINT64 total_send_size;
	UINT64 Ack_value;
	UINT64 total_recv_size;
	UINT32 user_data;
} mthl_query_sock_info_cnf_struct;

typedef enum
{
	TRA_SUCCESS           = 0,     /* success */
	TRA_INVALID_SOCKET    = -1,    /* invalid socket */
	TRA_INVALID_PARAM     = -2,    /* invalid para */
	TRA_GET_PORT_FAIL     = -3,    /* get uart port fail */
} transparent_rsp_error_enum;

typedef struct 
{    
    UINT16 size;
    UINT16 used;
    UINT8 *data_ptr;
    UINT8 *cur_ptr;
    UINT8 *end_ptr;    
} mthl_transport_buff_head_struct;

typedef struct
{ 
	UINT8 port;
	INT8 sock_id;
	sockaddr_struct addr;
	UINT32   user_data;
} mthl_transparent_trans_req_struct;

typedef struct
{ 
	INT8 result;
	INT8 sock_id;
	UINT8  port; //add 
	UINT32   user_data;
} mthl_transparent_trans_cnf_struct;

typedef struct
{ 
	transparent_error_cause_enum cause;
	INT8 sock_id;
	INT8 port;//modify
} mthl_transparent_trans_end_ind_struct;

typedef struct  
{
    BOOL is_read_suspend;
    BOOL is_write_suspend;
    /*
     * FLC locks
     */
    BOOL GPRS_FLC_UL_LOCKED;                /* The FLC UL buffer is not available)*/
    BOOL GPRS_FLC_DL_LOCKED;                /* While uart writed len < data len */

	INT8 sock_id;
	socket_type_enum sock_type;
	sockaddr_struct addr;
   
    UINT8  port;                            /* Keep for L4C UART port used */
   
    /* 
     * UL Datapath parameters
     */
    mthl_transport_buff_head_struct * ibuff_head_ptr;
   
    /* 
     * DL Datapath parameters
     * if GPRS_FLC_DL_LOCKED we should enqueue the FLC peer buff from TCM
     */
    mthl_transport_buff_head_struct * outbuff_head_ptr;
} mthl_transparent_mod_struct;

typedef enum
{
	MMI_ABM_PROFILE_ADD,
	MMI_ABM_PROFILE_MODIFY,
	MMI_ABM_PROFILE_DEL,
	MMI_ABM_PROFILE_ACT_TOTAL
} mmi_abm_profile_update_act_enum;

typedef struct
{
	UINT8 ref_count;
	UINT16 msg_len;
	UINT32 account_id;
	mmi_abm_profile_update_act_enum action;
	cbm_bearer_enum bearer_type;
} mmi_abm_update_account_info_req_struct;

typedef struct  
{       
	UINT8 	ref_count;
	UINT16 	msg_len;			/* LOCAL_PARA_HDR */	
	UINT16 	port;
} uart_escape_detected_ind_struct;

typedef struct  
{       
	UINT8 	ref_count;
	UINT16 	msg_len;			/* LOCAL_PARA_HDR */	
	UINT16 	port;
}uart_ready_to_read_ind_struct; 

 typedef struct  
{       
	UINT8 	ref_count;
	UINT16 	msg_len;			/* LOCAL_PARA_HDR */	
	UINT16 	port;
}uart_plugout_ind_struct;  

BOOL mthl_init(void);
void mthl_main_thread(LPVOID lparam);

void mthl_send_create_pdp_param_req_hdlr(MSG* msg);
void mthl_activate_pdp_req_hdlr(MSG* msg);
void mthl_create_and_conn_req_hdlr(MSG *msg);
void mthl_get_host_by_name_req_hdlr(MSG *msg);
void mthl_send_req_hdlr(MSG *msg);
void mthl_send_create_pdp_param_cnf(mthl_create_pdp_param_cnf_struct *param_cnf);
void mthl_send_activate_pdp_cnf(mthl_activate_pdp_cnf_struct* pdp_cnf);
void mthl_get_host_by_name_soc_cnf(mthl_get_host_by_name_cnf_struct *name_cnf);
void mthl_send_create_conn_cnf(mthl_create_conn_cnf_struct *conn_cnf);
void mthl_send_create_sock_ind(mthl_create_sock_ind_struct *mthl_create_sock_ind);
void mthl_send_sock_cnf(mthl_send_data_cnf_struct *mthl_send_data_cnf);
void mthl_get_host_by_name_soc_cnf_mod_id(mthl_get_host_by_name_cnf_struct *mthl_get_host_by_name_cnf);
void mthl_send_msg_to_abm(UINT16 msg_id, void *req);

#endif //__MTHL_H__

