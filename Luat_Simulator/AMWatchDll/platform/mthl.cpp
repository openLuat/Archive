/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    mthl.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          MTHL.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "platform.h"
#include "platform_rtos.h"
#include "platform_socket.h"

#include "mthl.h"

#ifndef DEBUG_INFO_M2M_MTHL_TASK_HANDLE_ONLY_FOR_IT
#define DEBUG_INFO_M2M_MTHL_TASK_HANDLE_ONLY_FOR_IT   //把添加的assert包起来，仅供IT使用；
#endif

typedef struct
{
   UINT32    user_data;
   BOOL       bUsed;
}mthl_get_host_by_name_save_struct;

typedef struct
{
   UINT16 addr_len;
   UINT8 addr[128];
   UINT32    user_data;
} mthl_get_host_by_addr_save_struct;

typedef struct
{
	UINT32 server_account_id;
	INT8   listen_sock_id;
} mthl_server_info;

typedef enum
{
    SRV_DTCNT_DB_TRANS_NONE,
    SRV_DTCNT_DB_TRANS_CREATE,
    SRV_DTCNT_DB_TRANS_FIRST_BOOTUP,
    SRV_DTCNT_DB_TRANS_UPDATE,
    SRV_DTCNT_DB_TRANS_ADD,
    SRV_DTCNT_DB_TRANS_DELETE,
    SRV_DTCNT_DB_TRANS_TOTAL
} srv_dtcnt_db_trans_type_enum;

mthl_create_pdp_param_save_struct g_mthl_create_pdp_param_save[MTHL_GPRS_MAX_PDP_NUM];

mthl_create_conn_soc_save_struct  g_mthl_create_conn_soc_save[MTHL_GPRS_MAX_PDP_NUM];

mthl_get_host_by_name_save_struct g_get_host_by_name_save[MAX_GET_HOST_BY_NAME_NUM];
mthl_get_host_by_addr_save_struct g_get_host_by_addr_save;



BOOL g_mthl_task_init_done = FALSE;

UINT8 g_mthl_atci_app_id = CBM_INVALID_APP_ID;
UINT32 g_mthl_wifi_nwk_acct_id = CBM_INVALID_NWK_ACCT_ID;

INT32 g_request_id_for_gethostbyname = 0;
INT32 g_request_id_for_gethostbyaddr = 0;

mthl_server_info g_server_info;

mthl_transparent_mod_struct mthl_transparent_mode_context;
BOOL is_mthl_in_transparent_mode = FALSE;

void mthl_send_query_pdp_param_cnf(mthl_query_pdp_param_cnf_struct *mthl_query_pdp_param_cnf);
void mthl_send_deactivate_pdp_ind(mthl_deactivate_pdp_ind_struct * mthl_deactivate_pdp_ind);
void mthl_send_deactivate_pdp_cnf(mthl_deactivate_pdp_cnf_struct * mthl_deactivate_pdp_cnf);
void mthl_send_query_pdp_state_cnf(mthl_query_pdp_state_cnf_struct *mthl_query_pdp_state_cnf);

void mthl_query_sock_num_cnf(mthl_query_sock_account_info_cnf_struct *mthl_query_sock_account_info_cnf);
void mthl_close_sock_cnf(mthl_close_sock_cnf_struct *mthl_close_cnf);
void mthl_close_sock_ind(mthl_close_sock_ind_struct *mthl_close_ind);
void mthl_send_sock_cnf(mthl_send_data_cnf_struct *mthl_send_data_cnf);
void mthl_send_sock_ind(mthl_send_data_ind_struct *mthl_send_data_ind);
void mthl_recv_sock_cnf(mthl_recv_data_cnf_struct *mthl_recv_data_cnf);
void mthl_query_info_sock_cnf(mthl_query_sock_info_cnf_struct *mthl_query_sock_info_cnf);
void mthl_get_host_by_addr_soc_cnf(mthl_get_host_by_addr_cnf_struct *mthl_get_host_by_addr_cnf);
void mthl_tcp_server_listen_cnf(mthl_server_listen_cnf_struct *mthl_server_listen_cnf);
void mthl_tcp_server_accept_ind(mthl_server_accept_ind_struct *mthl_server_accept_ind);


INT8 mthl_cbm_release_bearer(UINT32 ori_acct_id);
void mthl_send_abm_acc_info(srv_dtcnt_db_trans_type_enum opcode, UINT32 account_id, cbm_bearer_enum bearer_type);
void mthl_soc_notify(MSG *inMsg);
void mthl_soc_get_host_by_name_ind_hdlr(MSG *msg);
void mthl_soc_get_host_by_addr_ind_hdlr(MSG *msg);


static void mthl_transparent_reveive_data(void);
static void mthl_transparent_uart_ready_to_read_ind_hdlr(void);
mthl_transport_buff_head_struct* mthl_transparent_mem_alloc(void);
static void mthl_transparent_send_end_ind(INT8 port, transparent_error_cause_enum cause);
void mthl_transparent_ready_to_write_uart_ind_hdlr(void);

BOOL mthl_init(void)
{
    int i,j;

    memset(g_mthl_create_pdp_param_save, 0x00, sizeof(mthl_create_pdp_param_save_struct)*MTHL_GPRS_MAX_PDP_NUM);
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        g_mthl_create_pdp_param_save[i].account_id = CBM_INVALID_NWK_ACCT_ID;
    }

    memset(g_mthl_create_conn_soc_save, 0x00, sizeof(mthl_create_conn_soc_save_struct)*MTHL_GPRS_MAX_PDP_NUM);
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        g_mthl_create_conn_soc_save[i].account_id = CBM_INVALID_NWK_ACCT_ID;

        for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
        {
            g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;
            g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;
        }
    }

    memset(&g_get_host_by_name_save, 0x00, sizeof(g_get_host_by_name_save));
    memset(&g_get_host_by_addr_save, 0x00, sizeof(mthl_get_host_by_addr_save_struct));
    g_server_info.server_account_id = CBM_INVALID_NWK_ACCT_ID;
    g_server_info.listen_sock_id    = SOC_ERROR;

    g_mthl_task_init_done = TRUE;

    return TRUE;
}

//不存在关机需要保存的数据；
void mthl_send_create_pdp_param_req_hdlr(MSG *msg)
{
    int i;
    INT8 result;
    mthl_create_pdp_param_req_struct   *mthl_create_pdp_param_req;
    mthl_create_pdp_param_cnf_struct   mthl_create_pdp_param_cnf;
    UINT32 acc_id;

    mthl_create_pdp_param_req = (mthl_create_pdp_param_req_struct*)msg->wParam;
    memset(&mthl_create_pdp_param_cnf, 0x00, sizeof(mthl_create_pdp_param_cnf_struct));

    if(mthl_create_pdp_param_req->account_id < 0
        || mthl_create_pdp_param_req->account_id == CBM_INVALID_NWK_ACCT_ID
        || mthl_create_pdp_param_req->apn_length == 0
        || mthl_create_pdp_param_req->pdp_type == NULL_PDP_ADDR_TYPE
        || (mthl_create_pdp_param_req->proxy_addr[0] == 0 && mthl_create_pdp_param_req->proxy_port > 0)
        || (mthl_create_pdp_param_req->proxy_addr[0] != 0 && mthl_create_pdp_param_req->proxy_port == 0)
        )
    {
        mthl_create_pdp_param_cnf.result = PARAM_INVALID;
        mthl_create_pdp_param_cnf.account_id = mthl_create_pdp_param_req->account_id;
        mthl_create_pdp_param_cnf.user_data  = mthl_create_pdp_param_req->user_data;//原封不动返回;

        //返回cnf;
        mthl_send_create_pdp_param_cnf(&mthl_create_pdp_param_cnf);
        return;
    }


    if(g_mthl_task_init_done == TRUE ||
        (g_mthl_task_init_done == FALSE && g_mthl_atci_app_id != CBM_INVALID_APP_ID))
    {
        cbm_app_info_struct info;

		/**********************************************************
	      根据指定的APN进行网络连接的步骤:
		  1. 调用srv_dtcnt_get_acc_id_by_apn获取到account id
		  2. 调用cbm_register_app_id_with_app_info注册app id
		  3. 调用cbm_set_app_id设置app id 和account id
	    **********************************************************/
        srv_dtcnt_get_acc_id_by_apn((char*)mthl_create_pdp_param_req->apn, &acc_id );

		info.app_str_id = NULL;
		info.app_icon_id = NULL;
        info.app_type = DTCNT_APPTYPE_MRE_NET | DTCNT_APPTYPE_NO_PX;

        if(g_mthl_atci_app_id != CBM_INVALID_APP_ID)
        {
            cbm_deregister_app_id(g_mthl_atci_app_id);
        }

        result = cbm_register_app_id_with_app_info(&info, &g_mthl_atci_app_id);

        if(result ==CBM_OK)
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr  g_mthl_atci_app_id = %d acc_id=%d.", g_mthl_atci_app_id, acc_id);
        else
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_atci_app_id error, result = %d.", result);

        for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
        {
            //Update param;
            if(g_mthl_create_pdp_param_save[i].account_id == mthl_create_pdp_param_req->account_id)
            {
                g_mthl_create_pdp_param_save[i].nwk_acct_id= cbm_set_app_id(acc_id, g_mthl_atci_app_id);
            }
        }

        g_mthl_task_init_done = FALSE;
    }

    //先判断是否为更新account_id相关参数的操作；
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        //Update param;
        if(g_mthl_create_pdp_param_save[i].account_id == mthl_create_pdp_param_req->account_id)
        {
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr match acct_id, update param case, param:");

            //通过ori_acct_id保存当前acct_id对应的上网参数（MSG带过来的）；
            g_mthl_create_pdp_param_save[i].account_id = mthl_create_pdp_param_req->account_id;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_create_pdp_param_save[%d].account_id = %d.", i,g_mthl_create_pdp_param_save[i].account_id);

            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_create_pdp_param_save[%d].nwk_acct_id = %d.", i,g_mthl_create_pdp_param_save[i].nwk_acct_id);

            g_mthl_create_pdp_param_save[i].apn_length = mthl_create_pdp_param_req->apn_length;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_create_pdp_param_save[%d].apn_length = %d.", i,g_mthl_create_pdp_param_save[i].apn_length);

            strncpy((char*)g_mthl_create_pdp_param_save[i].apn, (char*)mthl_create_pdp_param_req->apn, g_mthl_create_pdp_param_save[i].apn_length);
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_create_pdp_param_save[%d].apn = %s.", i,g_mthl_create_pdp_param_save[i].apn);

            g_mthl_create_pdp_param_save[i].pdp_type = mthl_create_pdp_param_req->pdp_type;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr g_mthl_create_pdp_param_save[%d].pdp_type = %d.", i,g_mthl_create_pdp_param_save[i].pdp_type);

            if(mthl_create_pdp_param_req->proxy_port != 0)
            {
                g_mthl_create_pdp_param_save[i].proxy_flags = 1;
                g_mthl_create_pdp_param_save[i].proxy_port = mthl_create_pdp_param_req->proxy_port;
                g_mthl_create_pdp_param_save[i].proxy_addr[0] = mthl_create_pdp_param_req->proxy_addr[0];
                g_mthl_create_pdp_param_save[i].proxy_addr[1] = mthl_create_pdp_param_req->proxy_addr[1];
                g_mthl_create_pdp_param_save[i].proxy_addr[2] = mthl_create_pdp_param_req->proxy_addr[2];
                g_mthl_create_pdp_param_save[i].proxy_addr[3] = mthl_create_pdp_param_req->proxy_addr[3];
                LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr use proxy.");
            }
            else
            {
                g_mthl_create_pdp_param_save[i].proxy_flags = 0;
                LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr no use proxy.");
            }

            break;
        }
        else
            continue;
    }

    //match acct_id, update param已完成;
    if(i < MTHL_GPRS_MAX_PDP_NUM)
    {
        //返回cnf SUCCESS之前，还要update acc_info;
        //mthl_send_abm_acc_info(SRV_DTCNT_DB_TRANS_UPDATE, mthl_create_pdp_param_req->account_id, CBM_PS);
        mthl_create_pdp_param_cnf.result     = SUCCESS;
        mthl_create_pdp_param_cnf.account_id = mthl_create_pdp_param_req->account_id;
        mthl_create_pdp_param_cnf.user_data  = mthl_create_pdp_param_req->user_data;//原封不动返回;
        mthl_send_create_pdp_param_cnf(&mthl_create_pdp_param_cnf);

        LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr match acct_id, update param done.");
        return;
    }

    //not match acct_id, 则:新添加一组param;
    //mthl task save param;
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        //Add param;
        if(g_mthl_create_pdp_param_save[i].account_id == CBM_INVALID_NWK_ACCT_ID)
        {
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): Add param case, param:");

            //通过ori_acct_id保存当前acct_id对应的上网参数（MSG带过来的）；
            g_mthl_create_pdp_param_save[i].account_id = mthl_create_pdp_param_req->account_id;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d.", i,g_mthl_create_pdp_param_save[i].account_id);

            g_mthl_create_pdp_param_save[i].nwk_acct_id= cbm_set_app_id(acc_id, g_mthl_atci_app_id);

            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].nwk_acct_id = %d.", i,g_mthl_create_pdp_param_save[i].nwk_acct_id);

            g_mthl_create_pdp_param_save[i].apn_length = mthl_create_pdp_param_req->apn_length;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].apn_length = %d.", i,g_mthl_create_pdp_param_save[i].apn_length);

            strncpy((char*)g_mthl_create_pdp_param_save[i].apn, (char*)mthl_create_pdp_param_req->apn, g_mthl_create_pdp_param_save[i].apn_length);
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].apn = %s.", i,g_mthl_create_pdp_param_save[i].apn);

            g_mthl_create_pdp_param_save[i].pdp_type = mthl_create_pdp_param_req->pdp_type;
            LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].pdp_type = %d.", i,g_mthl_create_pdp_param_save[i].pdp_type);

            if(mthl_create_pdp_param_req->proxy_port != 0)
            {
                g_mthl_create_pdp_param_save[i].proxy_flags = 1;
                g_mthl_create_pdp_param_save[i].proxy_port = mthl_create_pdp_param_req->proxy_port;
                g_mthl_create_pdp_param_save[i].proxy_addr[0] = mthl_create_pdp_param_req->proxy_addr[0];
                g_mthl_create_pdp_param_save[i].proxy_addr[1] = mthl_create_pdp_param_req->proxy_addr[1];
                g_mthl_create_pdp_param_save[i].proxy_addr[2] = mthl_create_pdp_param_req->proxy_addr[2];
                g_mthl_create_pdp_param_save[i].proxy_addr[3] = mthl_create_pdp_param_req->proxy_addr[3];
                LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): use proxy.");
            }
            else
            {
                g_mthl_create_pdp_param_save[i].proxy_flags = 0;
                LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): no use proxy.");
            }

            break;
        }
        else
        continue;
    }

    if(i == MTHL_GPRS_MAX_PDP_NUM)
    {
        mthl_create_pdp_param_cnf.result = LIMITED_RES;
        mthl_create_pdp_param_cnf.account_id = mthl_create_pdp_param_req->account_id;
        mthl_create_pdp_param_cnf.user_data  = mthl_create_pdp_param_req->user_data;//原封不动返回;

        //返回cnf;
        mthl_send_create_pdp_param_cnf(&mthl_create_pdp_param_cnf);
        LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): LIMITED_RES case.");
        return;
    }

    //返回cnf SUCCESS之前，还要update acc_info;
    mthl_send_abm_acc_info(SRV_DTCNT_DB_TRANS_ADD, mthl_create_pdp_param_req->account_id, CBM_PS);
    mthl_create_pdp_param_cnf.result     = SUCCESS;
    mthl_create_pdp_param_cnf.account_id = mthl_create_pdp_param_req->account_id;
    mthl_create_pdp_param_cnf.user_data  = mthl_create_pdp_param_req->user_data;//原封不动返回;
    mthl_send_create_pdp_param_cnf(&mthl_create_pdp_param_cnf);
    LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): SUCCESS case.");

    return;

}

void mthl_get_pdp_param_req_hdlr(MSG *msg)
{
            int i;
	 		mthl_query_pdp_param_req_struct   *mthl_query_pdp_param_req;
			mthl_query_pdp_param_cnf_struct    mthl_query_pdp_param_cnf;

			mthl_query_pdp_param_req = (mthl_query_pdp_param_req_struct*)msg->wParam;
			memset(&mthl_query_pdp_param_cnf, 0x00, sizeof(mthl_query_pdp_param_cnf_struct));

			//query the param of mthl task save ;
			for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
			{
			   //find valid account_id;
			   if(g_mthl_create_pdp_param_save[i].account_id != CBM_INVALID_NWK_ACCT_ID)
			   {
			        LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): query param OK case, param:");

			        //查询到一组account_id有效，对应的参数，就是成功；
					mthl_query_pdp_param_cnf.result = TRUE;
					mthl_query_pdp_param_cnf.pdp_param_num++;

					mthl_query_pdp_param_cnf.pdp_param_info[i].account_id = g_mthl_create_pdp_param_save[i].account_id;
					LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d.", i,g_mthl_create_pdp_param_save[i].account_id);

					//g_mthl_create_pdp_param_save[i].nwk_acct_id;
					//LogWriter::LOGX("[MTHL] mthl_send_create_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].nwk_acct_id = %d.", i,g_mthl_create_pdp_param_save[i].nwk_acct_id);

					mthl_query_pdp_param_cnf.pdp_param_info[i].apn_length = g_mthl_create_pdp_param_save[i].apn_length;
					LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].apn_length = %d.", i,g_mthl_create_pdp_param_save[i].apn_length);

					strncpy((char*)mthl_query_pdp_param_cnf.pdp_param_info[i].apn, (char*)g_mthl_create_pdp_param_save[i].apn, g_mthl_create_pdp_param_save[i].apn_length);
					LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].apn = %s.", i,g_mthl_create_pdp_param_save[i].apn);

					mthl_query_pdp_param_cnf.pdp_param_info[i].pdp_type = g_mthl_create_pdp_param_save[i].pdp_type;
                    LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): g_mthl_create_pdp_param_save[%d].pdp_type = %d.", i,g_mthl_create_pdp_param_save[i].pdp_type);

					if(g_mthl_create_pdp_param_save[i].proxy_port != 0)
					{
						mthl_query_pdp_param_cnf.pdp_param_info[i].proxy_port = g_mthl_create_pdp_param_save[i].proxy_port;
                        mthl_query_pdp_param_cnf.pdp_param_info[i].proxy_addr[0] = g_mthl_create_pdp_param_save[i].proxy_addr[0];
						mthl_query_pdp_param_cnf.pdp_param_info[i].proxy_addr[1] = g_mthl_create_pdp_param_save[i].proxy_addr[1];
						mthl_query_pdp_param_cnf.pdp_param_info[i].proxy_addr[2] = g_mthl_create_pdp_param_save[i].proxy_addr[2];
						mthl_query_pdp_param_cnf.pdp_param_info[i].proxy_addr[3] = g_mthl_create_pdp_param_save[i].proxy_addr[3];
						LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): use proxy case.");
					}
					else
					{
					    LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): no use proxy case.");
					}
                    //查询到一组有效的，仍要继续查询；
					continue;
			   }
			   else
			   	   continue;
			}

			//遍历之后，not find;
			if(mthl_query_pdp_param_cnf.pdp_param_num == 0)
			{
			    mthl_query_pdp_param_cnf.result = FALSE;
				//mthl_query_pdp_param_cnf.pdp_param_num++; //0
				mthl_query_pdp_param_cnf.user_data  = mthl_query_pdp_param_req->user_data;//原封不动返回;

				//返回cnf;
				mthl_send_query_pdp_param_cnf(&mthl_query_pdp_param_cnf);
				LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): query param fail case.");
				return;
			}

            //find valid account_id;
			mthl_query_pdp_param_cnf.user_data  = mthl_query_pdp_param_req->user_data;//原封不动返回;
			mthl_send_query_pdp_param_cnf(&mthl_query_pdp_param_cnf);
            LogWriter::LOGX("[MTHL] mthl_get_pdp_param_req_hdlr(): SUCCESS case.");

            return;
}
void mthl_send_activate_pdp_req_hdlr(MSG *msg)
{
    int i;
    cbm_result_error_enum ret;
    INT32 return_val;

    mthl_activate_pdp_req_struct   *mthl_activate_pdp_req;
    mthl_activate_pdp_cnf_struct   mthl_activate_pdp_cnf;

    mthl_activate_pdp_req = (mthl_activate_pdp_req_struct*)msg->wParam;
    memset(&mthl_activate_pdp_cnf, 0x00, sizeof(mthl_activate_pdp_cnf_struct));

    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        if(mthl_activate_pdp_req->account_id == g_mthl_create_pdp_param_save[i].account_id)
        {
            LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): match acct_id, param:");

            //ori_acct_id匹配成功，保存auth info;
            g_mthl_create_pdp_param_save[i].Auth_info.AuthType = mthl_activate_pdp_req->Auth_info.AuthType;
            LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): g_mthl_create_pdp_param_save[%d].Auth_info.AuthType = %d", i,g_mthl_create_pdp_param_save[i].Auth_info.AuthType);

            memcpy(g_mthl_create_pdp_param_save[i].Auth_info.UserName, mthl_activate_pdp_req->Auth_info.UserName, CUSTOM_DTCNT_PROF_MAX_USER_LEN);
            LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): g_mthl_create_pdp_param_save[%d].Auth_info.UserName = %s", i,g_mthl_create_pdp_param_save[i].Auth_info.UserName);

            memcpy(g_mthl_create_pdp_param_save[i].Auth_info.Passwd, mthl_activate_pdp_req->Auth_info.Passwd, CUSTOM_DTCNT_PROF_MAX_USER_LEN);
            LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): g_mthl_create_pdp_param_save[%d].Auth_info.Passwd = %s", i,g_mthl_create_pdp_param_save[i].Auth_info.Passwd);

            break;
        }
        else
            continue;
    }

    //account_id匹配失败；
    if(i == MTHL_GPRS_MAX_PDP_NUM)
    {
        mthl_activate_pdp_cnf.account_id = mthl_activate_pdp_req->account_id;
        mthl_activate_pdp_cnf.result = FALSE;
        mthl_activate_pdp_cnf.error_cause = 0;
        mthl_activate_pdp_cnf.user_data = mthl_activate_pdp_req->user_data;

        //返回cnf;
        mthl_send_activate_pdp_cnf(&mthl_activate_pdp_cnf);
        LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): not match acct_id.");
        //g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_CNF;
        g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE; //激活失败；
        return;
    }

    //通过ori_acct_id找到g_nwk_acct_id；
    //仿照srv_cbm_register_bearer_info()的做法;
    /*srv_cbm_register_bearer_info(
    app_id,
    SRV_CBM_ACTIVATING | SRV_CBM_ACTIVATED | SRV_CBM_DEACTIVATED,
    SRV_CBM_BEARER_CSD | SRV_CBM_BEARER_PS,
    srv_nwk_bearer_info_ind,
    NULL);
    */

    return_val = cbm_register_bearer_info_by_app_id(
        g_mthl_atci_app_id,
        SRV_CBM_BEARER_CSD | SRV_CBM_BEARER_PS,
        SRV_CBM_ACTIVATING | SRV_CBM_ACTIVATED | SRV_CBM_DEACTIVATED);

    //register_bearer_info IND处理失败；
    if(CBM_OK != return_val)
    {
        mthl_activate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;
        mthl_activate_pdp_cnf.result = FALSE;
        mthl_activate_pdp_cnf.error_cause = 0;
        mthl_activate_pdp_cnf.user_data = mthl_activate_pdp_req->user_data;

        //返回cnf;
        mthl_send_activate_pdp_cnf(&mthl_activate_pdp_cnf);
        LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): register_bearer_info fail.");

        g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE;//激活失败；
        return;
    }

    ret = (cbm_result_error_enum)cbm_open_bearer(g_mthl_create_pdp_param_save[i].nwk_acct_id);

    //记录当前account_id上是激活操作；
    g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_REQ;

    //立即返回成功或失败；
    if(ret != CBM_WOULDBLOCK)
    {
        mthl_activate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;
        if(ret == CBM_OK)
        {
            cbm_hold_bearer(g_mthl_atci_app_id);
            mthl_activate_pdp_cnf.result = TRUE;
            g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_CNF;//激活成功；
        }
        else
        {
            mthl_activate_pdp_cnf.result = FALSE;
            g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE;//激活失败；
        }
        mthl_activate_pdp_cnf.error_cause = 0;
        mthl_activate_pdp_cnf.user_data = mthl_activate_pdp_req->user_data;

        //返回cnf;
        mthl_send_activate_pdp_cnf(&mthl_activate_pdp_cnf);
        LogWriter::LOGX("[MTHL] mthl_send_activate_pdp_req_hdlr(): open_bearer = %d.", ret);

        //g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_CNF;
        return;
    }

    //WOULDBLOCK: IND上报后，可知bearer是否激活成功；
    //具体处理ABM的INIT/QUERY消息;把apn/ auth/usrname/passwd带下去；
    LogWriter::LOGX("[MTHL] activate_pdp: WOULDBLOCK case.");
    g_mthl_create_pdp_param_save[i].user_data = mthl_activate_pdp_req->user_data;
    return;
}


void mthl_send_deactivate_pdp_req_hdlr(MSG *msg)
{
            mthl_deactivate_pdp_req_struct   *mthl_deactivate_pdp_req;
			mthl_deactivate_pdp_cnf_struct   mthl_deactivate_pdp_cnf;
			INT8 ret;
			int i=0;

			mthl_deactivate_pdp_req = (mthl_deactivate_pdp_req_struct*)msg->wParam;
			memset(&mthl_deactivate_pdp_cnf, 0x00, sizeof(mthl_deactivate_pdp_cnf_struct));

			for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
			{
			    if(mthl_deactivate_pdp_req->account_id == g_mthl_create_pdp_param_save[i].account_id)
			    {
			        //ori_acct_id匹配成功;
			        LogWriter::LOGX("[MTHL] mthl_send_deactivate_pdp_req_hdlr(): match acct_id case.");
					break;
			    }
				else
					continue;
			}

            //account_id匹配失败；
			if(i == MTHL_GPRS_MAX_PDP_NUM)
			{
			    mthl_deactivate_pdp_cnf.account_id = mthl_deactivate_pdp_req->account_id;
				mthl_deactivate_pdp_cnf.result = FALSE;
				mthl_deactivate_pdp_cnf.error_cause = 0;
				mthl_deactivate_pdp_cnf.user_data = mthl_deactivate_pdp_req->user_data;

				//返回cnf;
				mthl_send_deactivate_pdp_cnf(&mthl_deactivate_pdp_cnf);
				LogWriter::LOGX("[MTHL] mthl_send_deactivate_pdp_req_hdlr(): not match acct_id.");

				g_mthl_create_pdp_param_save[i].bear_op = PDP_DEACTIVATE_CNF;
				return;
			}

			ret = mthl_cbm_release_bearer(mthl_deactivate_pdp_req->account_id);
			g_mthl_create_pdp_param_save[i].bear_op = PDP_DEACTIVATE_REQ;

			//立即返回成功或失败；
			if(ret != CBM_WOULDBLOCK)
			{
			    mthl_deactivate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;

				if(ret == CBM_OK)
				   mthl_deactivate_pdp_cnf.result = TRUE;
				else
				   mthl_deactivate_pdp_cnf.result = FALSE;

				mthl_deactivate_pdp_cnf.error_cause = 0;
				mthl_deactivate_pdp_cnf.user_data = mthl_deactivate_pdp_req->user_data;

				//返回cnf;
				mthl_send_deactivate_pdp_cnf(&mthl_deactivate_pdp_cnf);
				LogWriter::LOGX("[MTHL] mthl_send_deactivate_pdp_req_hdlr(): release_bearer = %d.", ret);

				g_mthl_create_pdp_param_save[i].bear_op = PDP_DEACTIVATE_CNF;
				return;
			}

			//IND上报后，可知bearer是否去激活成功；
			LogWriter::LOGX("[MTHL] deactivate_pdp: WOULDBLOCK case.");

}

void mthl_get_pdp_state_req_hdlr(MSG *msg)
{
			int i;
			INT32 bearer_status;
	 		mthl_query_pdp_state_req_struct   *mthl_query_pdp_state_req;
			mthl_query_pdp_state_cnf_struct    mthl_query_pdp_state_cnf;

			mthl_query_pdp_state_req = (mthl_query_pdp_state_req_struct*)msg->wParam;
			memset(&mthl_query_pdp_state_cnf, 0x00, sizeof(mthl_query_pdp_state_cnf_struct));

            //query the param of mthl task save ;
			for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
			{
			   //find valid account_id;
			   if(g_mthl_create_pdp_param_save[i].account_id != CBM_INVALID_NWK_ACCT_ID)
			   {
			        LogWriter::LOGX("[MTHL] mthl_get_pdp_state_req_hdlr(): query state OK case, param:");

			        //查询到一组account_id有效，对应的参数，就是成功；
					mthl_query_pdp_state_cnf.result = TRUE;
					mthl_query_pdp_state_cnf.num++;

					mthl_query_pdp_state_cnf.state_list[i][0] = (UINT8)(g_mthl_create_pdp_param_save[i].account_id);
                    LogWriter::LOGX("[MTHL] mthl_get_pdp_state_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d.", i,g_mthl_create_pdp_param_save[i].account_id);

					if(g_mthl_create_pdp_param_save[i].bear_op == PDP_ACTIVATE_CNF)
					   mthl_query_pdp_state_cnf.state_list[i][1] = TRUE;
					//激活后，deactivate失败，此时可能也还是ACTIVATED;
					else if(g_mthl_create_pdp_param_save[i].bear_op == PDP_NONE)
					{
					   bearer_status = abm_get_bearer_available_status(CBM_BEARER_PS, CBM_SIM_ID_SIM1);

					   if(bearer_status == CBM_ACTIVATED)
					   	 mthl_query_pdp_state_cnf.state_list[i][1] = TRUE;
					   else
					   	 mthl_query_pdp_state_cnf.state_list[i][1] = FALSE;
					}
					else
					   mthl_query_pdp_state_cnf.state_list[i][1] = FALSE;

                    //查询到一组有效的，仍要继续查询；
					continue;
			   }
			   else
			   	   continue;
			}

			//遍历之后，not find;
			if(mthl_query_pdp_state_cnf.num == 0)
			{
			    mthl_query_pdp_state_cnf.result = FALSE;
				//mthl_query_pdp_state_cnf.num; //0
				mthl_query_pdp_state_cnf.user_data  = mthl_query_pdp_state_req->user_data;//原封不动返回;

				//返回cnf;
				mthl_send_query_pdp_state_cnf(&mthl_query_pdp_state_cnf);
				LogWriter::LOGX("[MTHL] mthl_get_pdp_state_req_hdlr(): query state fail case.");
				return;
			}

            //find valid account_id;
			mthl_query_pdp_state_cnf.user_data  = mthl_query_pdp_state_req->user_data;//原封不动返回;
			mthl_send_query_pdp_state_cnf(&mthl_query_pdp_state_cnf);
            LogWriter::LOGX("[MTHL] mthl_get_pdp_state_req_hdlr(): SUCCESS case.");

            return;
}


/*******************************************socket handle*************************************************/
void mthl_create_and_conn_req_hdlr(MSG *msg)
{
    mthl_create_conn_req_struct   *mthl_create_conn_req;
    mthl_create_conn_cnf_struct    mthl_create_conn_cnf;
    int i=0,j = 0;
    INT8 error_soc_create= 0;
    INT8 ret_conn = 0;
    UINT8 val;

    mthl_create_conn_req = (mthl_create_conn_req_struct*)msg->wParam;
    memset(&mthl_create_conn_cnf, 0x00, sizeof(mthl_create_conn_cnf_struct));

    //通过ori_acct_id找到g_nwk_acct_id；
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        if(mthl_create_conn_req->account_id == g_mthl_create_pdp_param_save[i].account_id)
        {
            //ori_acct_id匹配成功;
            LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): match acct_id case.");
            LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d", i,g_mthl_create_pdp_param_save[i].account_id);
            break;
        }
        else
            continue;
    }

    //ori_acct_id匹配失败或创建的socket_type不支持；
    if(i == MTHL_GPRS_MAX_PDP_NUM || mthl_create_conn_req->addr.sock_type == SOC_SOCK_SMS || mthl_create_conn_req->addr.sock_type == SOC_SOCK_RAW)
    {
        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
        mthl_create_conn_cnf.result     = FALSE;
        mthl_create_conn_cnf.sock_id    = SOC_ERROR;
        mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

        //返回cnf;
        mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): not match acct_id or not support sock_type.");
        return;
    }

    #if defined(__MTK_TARGET__)
        #ifndef __TCPIP_LOOP_BACK__
        //真机上，loopback is disable,  check 127.0.0.1 is invalid;
        if(mthl_create_conn_req->addr.sock_type == SOC_SOCK_STREAM &&
            mthl_create_conn_req->addr.addr[0]==127 &&
            mthl_create_conn_req->addr.addr[1]==0 &&
            mthl_create_conn_req->addr.addr[2]==0 &&
            mthl_create_conn_req->addr.addr[3]==1)
        {
            mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
            mthl_create_conn_cnf.result     = FALSE;
            mthl_create_conn_cnf.sock_id    = SOC_ERROR;
            mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

            //返回cnf;
            mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
            LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP Case & loopback is disable.");
            return;
        }
        #endif
    #endif/*__MTK_TARGET__*/

    //记录sock_id与socket_type对应关系；
    //每次返回一个RSP成功的，根据ori_acct_id分类计数；
    error_soc_create = soc_create(SOC_PF_INET, mthl_create_conn_req->addr.sock_type, 0, g_mthl_create_pdp_param_save[i].nwk_acct_id);

    //只要socket创建成功，就设置异步、非阻塞；
 #if defined(__MTK_TARGET__)
    if(error_soc_create >= 0)
    {
    	    mthl_create_sock_ind_struct    mthl_create_sock_ind;

    	   val = TRUE;
	    soc_setsockopt(error_soc_create, SOC_NBIO, &val, sizeof(val));
	    val = SOC_READ | SOC_WRITE | SOC_CONNECT | SOC_CLOSE;
	    soc_setsockopt(error_soc_create, SOC_ASYNC, &val, sizeof(val));

    	    LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): error_soc_create = %d.", error_soc_create);

            mthl_create_sock_ind.account_id = mthl_create_conn_req->account_id;
            mthl_create_sock_ind.sock_id      = error_soc_create;
            mthl_create_sock_ind.user_data   =  mthl_create_conn_req->user_data;

    	    mthl_send_create_sock_ind(&mthl_create_sock_ind);
    }
 #else
    if(error_soc_create >= 0)
    {
        mthl_create_sock_ind_struct    mthl_create_sock_ind;

        val = TRUE;
        soc_setsockopt(error_soc_create, SOC_NBIO, &val, sizeof(val));
        val = SOC_READ | SOC_WRITE | SOC_CONNECT | SOC_CLOSE;
        soc_setsockopt(error_soc_create, SOC_ASYNC, &val, sizeof(val));

        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): error_soc_create = %d.", error_soc_create);

        mthl_create_sock_ind.account_id = mthl_create_conn_req->account_id;
        mthl_create_sock_ind.sock_id      = error_soc_create;
        mthl_create_sock_ind.user_data   =  mthl_create_conn_req->user_data;

        mthl_send_create_sock_ind(&mthl_create_sock_ind);
    }
 #endif

  /****************************************************UDP*************************************************************/
	//如果是基于UDP的，则：无连接的，直接返回RSP；
	//UDP success;
    if(mthl_create_conn_req->addr.sock_type == SOC_SOCK_DGRAM && error_soc_create >= 0)
    {
        g_mthl_create_conn_soc_save[i].account_id               = g_mthl_create_pdp_param_save[i].account_id;
		//g_mthl_create_conn_soc_save[i].sock_num++;

		//找到一个可用的socket_info[j]，再保存信息；
		for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
		{
		   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id == SOC_ERROR)
		   {
		        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, param:");

				g_mthl_create_conn_soc_save[i].socket_info[j].sock_id      = error_soc_create;
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);

				g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type  = SOC_SOCK_DGRAM;
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): UDP case.");

	            //UDP也要保存req带过来的默认发送地址；
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0] = mthl_create_conn_req->addr.addr[0];
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1] = mthl_create_conn_req->addr.addr[1];
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2] = mthl_create_conn_req->addr.addr[2];
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3] = mthl_create_conn_req->addr.addr[3];
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr_len= mthl_create_conn_req->addr.addr_len;
				g_mthl_create_conn_soc_save[i].socket_info[j].addr.port    = mthl_create_conn_req->addr.port;
				//保存信息ok，再计数；
				g_mthl_create_conn_soc_save[i].sock_num++;
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].sock_num = %d",i,g_mthl_create_conn_soc_save[i].sock_num);

				//为流量统计再保存一份；
				g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = error_soc_create;
				break;
		   }
		   else
		   	continue;
		}

		//创建成功，保存失败；
		if(j == MTHL_MAX_IP_SOCKET_NUM)
		{
		        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
				mthl_create_conn_cnf.result     = FALSE;
				mthl_create_conn_cnf.sock_id    = SOC_ERROR;
				mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

				//返回cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): not find free sock_info.");
				//close socket;
				soc_close(error_soc_create);
				return;
		}
		//创建成功，保存成功；
		else
		{
		        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
				mthl_create_conn_cnf.result     = TRUE;
				mthl_create_conn_cnf.sock_id    = error_soc_create;
				mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;
			    //返回cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, done.");
				return;
		}

    }
	//UDP 创建 fail;
	if(mthl_create_conn_req->addr.sock_type == SOC_SOCK_DGRAM && error_soc_create < 0)
	{
	            mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
				mthl_create_conn_cnf.result     = FALSE;
				mthl_create_conn_cnf.sock_id    = SOC_ERROR;
				mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

				//返回cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): UDP error_soc_create fail = %d.", error_soc_create);
				return;
	}

/********************************************************************************************************************/



	//如果是基于TCP的，则：soc_connect(…);根据连接结果再返回RSP；
	//TCP 创建 success;
    if(mthl_create_conn_req->addr.sock_type == SOC_SOCK_STREAM && error_soc_create >= 0)
    {
         g_mthl_create_conn_soc_save[i].account_id               = g_mthl_create_pdp_param_save[i].account_id;

         ret_conn = soc_connect(error_soc_create, &(mthl_create_conn_req->addr));
		 //立即返回连接成功或失败；
		 if(ret_conn !=SOC_WOULDBLOCK)
		 {
		     if(ret_conn == SOC_SUCCESS)//立即返回连接成功；
		     {
		            LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, success.");

					//找到一个可用的socket_info[j]，再保存信息；
					for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
					{
					   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id == SOC_ERROR)
					   {
					        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, param:");

							g_mthl_create_conn_soc_save[i].socket_info[j].sock_id      = error_soc_create;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);

							g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type  = SOC_SOCK_STREAM;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP case.");

				            //TCP 不用保存req带过来的要连接的Server地址；
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0] = mthl_create_conn_req->addr.addr[0];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1] = mthl_create_conn_req->addr.addr[1];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2] = mthl_create_conn_req->addr.addr[2];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3] = mthl_create_conn_req->addr.addr[3];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr_len= mthl_create_conn_req->addr.addr_len;
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.port    = mthl_create_conn_req->addr.port;

							//保存信息ok，再计数；
							g_mthl_create_conn_soc_save[i].sock_num++;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].sock_num = %d",i,g_mthl_create_conn_soc_save[i].sock_num);

							//为流量统计再保存一份；
							g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = error_soc_create;
							break;
					   }
					   else
					   	continue;
					}
                    //创建、连接成功，保存失败；
					if(j == MTHL_MAX_IP_SOCKET_NUM)
					{
					        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
							mthl_create_conn_cnf.result     = FALSE;
							mthl_create_conn_cnf.sock_id    = SOC_LIMIT_RESOURCE;
							mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

							//返回cnf;
							mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, success, not find free sock_info.");
							//close socket;
							soc_close(error_soc_create);
							return;
					}
					//创建、连接成功，保存成功；
					else
					{
					        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
							mthl_create_conn_cnf.result     = TRUE;
							mthl_create_conn_cnf.sock_id    = error_soc_create;
							mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;
						    //返回cnf;
							mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, success, find free sock_info.");
							return;
					}
		     }
			 else//立即返回连接失败；
			 {
			    mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
				mthl_create_conn_cnf.result     = FALSE;
				mthl_create_conn_cnf.sock_id    = SOC_ERROR;
				mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

				//返回cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, fail = %d.", ret_conn);

				//close socket;
				soc_close(error_soc_create);
				return;
			 }
		 }
		 //后续返回连接结果；
		 else
		 {
		        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, wouldblock.");

					//找到一个可用的socket_info[j]，再保存信息；
					for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
					{
					   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id == SOC_ERROR)
					   {
					        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, param:");

							g_mthl_create_conn_soc_save[i].socket_info[j].sock_id      = error_soc_create;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);

							g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type  = SOC_SOCK_STREAM;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP case.");

				            //TCP 不用保存req带过来的要连接的Server地址；
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0] = mthl_create_conn_req->addr.addr[0];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1] = mthl_create_conn_req->addr.addr[1];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2] = mthl_create_conn_req->addr.addr[2];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3] = mthl_create_conn_req->addr.addr[3];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr_len= mthl_create_conn_req->addr.addr_len;
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.port    = mthl_create_conn_req->addr.port;

							//保存信息ok，再计数；
							g_mthl_create_conn_soc_save[i].sock_num++;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].sock_num = %d",i,g_mthl_create_conn_soc_save[i].sock_num);

							//为流量统计再保存一份；
							g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = error_soc_create;
							break;
					   }
					   else
					   	continue;
					}
                    //创建、连接SOC_WOULDBLOCK，保存失败；
					if(j == MTHL_MAX_IP_SOCKET_NUM)
					{
					        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
							mthl_create_conn_cnf.result     = FALSE;
							mthl_create_conn_cnf.sock_id    = SOC_LIMIT_RESOURCE;
							mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

							//返回cnf;
							mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, success, not find free sock_info.");
							//close socket;
							soc_close(error_soc_create);
							return;
					}
					//创建、连接SOC_WOULDBLOCK，保存成功；
					else
					{
					        mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
							mthl_create_conn_cnf.result     = FALSE;
							mthl_create_conn_cnf.sock_id    = SOC_WOULDBLOCK;
							mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;
							g_mthl_create_conn_soc_save[i].user_data = mthl_create_conn_req->user_data;
                            g_mthl_create_conn_soc_save[i].socket_info[j].user_data = mthl_create_conn_req->user_data;

						    //返回cnf;
							//mthl_send_create_conn_cnf(&mthl_create_conn_cnf);没有对应的MSG，就不返回wouldblock;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): soc_connect, success, find free sock_info.");
							return;
					}
		 }
    }
    //TCP 创建 fail;
	if(mthl_create_conn_req->addr.sock_type == SOC_SOCK_STREAM && error_soc_create < 0)
	{
	            mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
				mthl_create_conn_cnf.result     = FALSE;
				mthl_create_conn_cnf.sock_id    = SOC_ERROR;
				mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

				//返回cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
				LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP error_soc_create fail = %d.", error_soc_create);
				return;
	}


	//非TCP/UDP的异常Case；
	LogWriter::LOGX("[MTHL] create_and_conn: no TCP & UDP case.");
    mthl_create_conn_cnf.account_id = mthl_create_conn_req->account_id;
	mthl_create_conn_cnf.result     = FALSE;
	mthl_create_conn_cnf.sock_id    = SOC_ERROR;
	mthl_create_conn_cnf.user_data  = mthl_create_conn_req->user_data;

	//返回cnf;
	mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
	return;

}


void mthl_query_sock_num_req_hdlr(MSG *msg)
{
    mthl_query_sock_account_info_req_struct   *mthl_query_sock_account_info_req;
    mthl_query_sock_account_info_cnf_struct    mthl_query_sock_account_info_cnf;
    int i =0;
    int j = 0;
    INT8 error_soc_create= 0;
    INT8 ret_conn = 0;

    mthl_query_sock_account_info_req = (mthl_query_sock_account_info_req_struct*)msg->wParam;
    memset(&mthl_query_sock_account_info_cnf, 0x00, sizeof(mthl_query_sock_account_info_cnf_struct));

	for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
	{
		if(mthl_query_sock_account_info_req->account_id == g_mthl_create_pdp_param_save[i].account_id)
		{
			//ori_acct_id匹配成功;
			LogWriter::LOGX("[MTHL] mthl_query_sock_num_req_hdlr(): match acct_id case.");
			LogWriter::LOGX("[MTHL] mthl_query_sock_num_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d", i,g_mthl_create_pdp_param_save[i].account_id);
			break;
		}
		else
			continue;
	}

	//ori_acct_id匹配失败；
	if(i == MTHL_GPRS_MAX_PDP_NUM)
	{
		mthl_query_sock_account_info_cnf.account_id = mthl_query_sock_account_info_req->account_id;
		mthl_query_sock_account_info_cnf.result     = FALSE;
		//mthl_query_sock_account_info_cnf.socket_info查询失败，无查询结果，都是default;
		//mthl_query_sock_account_info_cnf.sock_num查询失败，无查询结果，都是default;
		mthl_query_sock_account_info_cnf.user_data  = mthl_query_sock_account_info_req->user_data;

		//返回cnf;
		mthl_query_sock_num_cnf(&mthl_query_sock_account_info_cnf);
		LogWriter::LOGX("[MTHL] mthl_query_sock_num_req_hdlr(): not match acct_id case.");
		return;
	}

	//Check之前的分类计数结果，即可得到结果；
	mthl_query_sock_account_info_cnf.account_id = mthl_query_sock_account_info_req->account_id;
	LogWriter::LOGX("[MTHL] mthl_query_sock_num_req_hdlr():mthl_query_sock_account_info_req->account_id = %d", mthl_query_sock_account_info_req->account_id);

	mthl_query_sock_account_info_cnf.result     = TRUE;
	memcpy(mthl_query_sock_account_info_cnf.socket_info, g_mthl_create_conn_soc_save[i].socket_info, sizeof(sock_account_info_struct)*MTHL_MAX_IP_SOCKET_NUM);

	mthl_query_sock_account_info_cnf.sock_num   = g_mthl_create_conn_soc_save[i].sock_num;
	LogWriter::LOGX("[MTHL] mthl_query_sock_num_req_hdlr():g_mthl_create_conn_soc_save[%d].sock_num = %d", i, g_mthl_create_conn_soc_save[i].sock_num);

	mthl_query_sock_account_info_cnf.user_data  = mthl_query_sock_account_info_req->user_data;

	//返回cnf;
	mthl_query_sock_num_cnf(&mthl_query_sock_account_info_cnf);

	return;
}

void mthl_close_req_hdlr(MSG *msg)
{
	mthl_close_sock_req_struct   *mthl_close_req;
	mthl_close_sock_cnf_struct    mthl_close_cnf;
    INT8 ret;
	int i,j;

	mthl_close_req = (mthl_close_sock_req_struct*)msg->wParam;
	memset(&mthl_close_cnf, 0x00, sizeof(mthl_close_sock_cnf_struct));

	ret = soc_close(mthl_close_req->sock_id);

	if(ret == SOC_INVALID_SOCKET)
	{
	    mthl_close_cnf.result    = FALSE;
		mthl_close_cnf.sock_id   = mthl_close_req->sock_id;
		mthl_close_cnf.user_data = mthl_close_req->user_data;

	    //返回fail cnf;
		mthl_close_sock_cnf(&mthl_close_cnf);
		LogWriter::LOGX("[MTHL] mthl_close_req_hdlr() fail: soc_close ret = %d.", ret);
	}
	else if(ret == SOC_SUCCESS)
	{
	    mthl_close_cnf.result    = TRUE;
		mthl_close_cnf.sock_id   = mthl_close_req->sock_id;
		mthl_close_cnf.user_data = mthl_close_req->user_data;

	    //返回success cnf;
		mthl_close_sock_cnf(&mthl_close_cnf);
		LogWriter::LOGX("[MTHL] mthl_close_req_hdlr() success: soc_close ret = %d.", ret);

		//关闭server listen_sock;
        if(mthl_close_cnf.sock_id == g_server_info.listen_sock_id)
        {
           g_server_info.listen_sock_id = SOC_ERROR;
		   g_server_info.server_account_id = CBM_INVALID_NWK_ACCT_ID;
        }
		else
		{
		        //通过sock_id匹配，把close的socket id，包括statistic的部分，都清除为init value;
                for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
                {
                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
                    {
                       if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== mthl_close_req->sock_id)
                       {
                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr() match sock_id.");

                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
                         g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;

						 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_statistics_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id);
						 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;

						 g_mthl_create_conn_soc_save[i].sock_num--;
						 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, SOC_ERROR, done.");
					   	 break;
                       }
					   else
					   	 continue;
                    }
					if(j<MTHL_MAX_IP_SOCKET_NUM)
					  break;
					else
					  continue;
                }

				//清除失败；
				if(i == MTHL_GPRS_MAX_PDP_NUM)
				{
					LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, fail, not match sock_id..");
					LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): mthl_close_req->sock_id = %d.", mthl_close_req->sock_id);
				}
		 }
	}
	else
	{
	    //soc_close()的返回值只有以上两种，此Case 是异常的；
	    LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): error case.");
	  #ifdef DEBUG_INFO_M2M_MTHL_TASK_HANDLE_ONLY_FOR_IT
		assert(0);
	  #endif/*DEBUG_INFO_M2M_MTHL_TASK_HANDLE_ONLY_FOR_IT*/
	}

	return;
}

void mthl_send_req_hdlr(MSG *msg)
{
    mthl_send_data_req_struct   *mthl_send_data_req;
    mthl_send_data_cnf_struct    mthl_send_data_cnf;
    mthl_close_sock_ind_struct   mthl_close_ind;
    INT32 ret = SOC_ERROR;
    INT32 close_ret = SOC_ERROR;
    int i=0,j = 0;

    mthl_send_data_req = (mthl_send_data_req_struct*)msg->wParam;
    memset(&mthl_send_data_cnf, 0x00, sizeof(mthl_send_data_cnf_struct));
    memset(&mthl_close_ind, 0x00, sizeof(mthl_close_sock_ind_struct));

    // 根据sock_id 查询对应的socket_type，Check是否带了接收端地址；
    LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): sock_id = %d", mthl_send_data_req->sock_id);

    //通过sock_id反过来匹配到account_id;
    for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
    {
        for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
        {
            if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== mthl_send_data_req->sock_id)
            {
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): match sock_id.");
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", \
                     i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
                break;
            }
            else
                continue;
        }
        
        if(j<MTHL_MAX_IP_SOCKET_NUM)
        {
            LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): j= %d.", j);
            break;
        }
        else
            continue;
    }

    //sock_id匹配失败；
    if(i == MTHL_GPRS_MAX_PDP_NUM || mthl_send_data_req->data == NULL)
    {
        mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
        mthl_send_data_cnf.result    = FALSE;
        mthl_send_data_cnf.ret_val   = SOC_INVALID_SOCKET;
        mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

        //返回cnf;
        mthl_send_sock_cnf(&mthl_send_data_cnf);
        LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): not match sock_id, or mthl_send_data_req->data == NULL.");
        if(mthl_send_data_req->data != NULL)
        {
            WinUtil::L_FREE(mthl_send_data_req->data);
            mthl_send_data_req->data = NULL;
            LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): free_ctrl_buffer().");
        }
        return;
    }

    //跑到此处没有return，说明已经是不为NULL；
    LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): mthl_send_data_req->data != NULL.");

	 //TCP case;
	 if(g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type == SOC_SOCK_STREAM)
	 {
	    // soc_send()	for TCP
	    LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): TCP send case.");
	    ret = soc_send(mthl_send_data_req->sock_id, mthl_send_data_req->data, mthl_send_data_req->size, mthl_send_data_req->flags);

        //发送动作call完，就释放buff，后续user会重发；
        WinUtil::L_FREE(mthl_send_data_req->data);
		mthl_send_data_req->data = NULL;

		//立即发送成功；
		if(ret > 0)
		{
		     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
			 mthl_send_data_cnf.result    = TRUE;

			 mthl_send_data_cnf.ret_val   = ret;
			 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): TCP send ok, ret = %d.", ret);

			 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_send_size += ret;
			 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

			 //返回cnf;
			 mthl_send_sock_cnf(&mthl_send_data_cnf);
		 	 return;
		}
		//后续指示发送结果；
		else if(ret == SOC_WOULDBLOCK)
		{
		     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
			 mthl_send_data_cnf.result    = FALSE;
			 mthl_send_data_cnf.ret_val   = ret;
			 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

			 //返回cnf;
			 mthl_send_sock_cnf(&mthl_send_data_cnf);
			 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): TCP send WOULDBLOCK case.");

		     return;
		}
		//立即发送失败；
		else
		{
		     //对于TCP 立即发送失败的Case:由于对端强制关闭、或网络、bearer异常导致TCP连接出错，
		     //一般ret为-1, 释放socket;
		     if(ret == SOC_ERROR || ret == SOC_CONNABORTED || ret == SOC_BEARER_FAIL || ret == SOC_CONNRESET)
		     {
		         close_ret = soc_close(mthl_send_data_req->sock_id);

				 if(close_ret == SOC_INVALID_SOCKET)
				 	LogWriter::LOGX("[MTHL] send data fail: soc_close ret = SOC_INVALID_SOCKET");
				 else if(close_ret == SOC_SUCCESS)
				 {
				    //通过sock_id匹配，把close的socket id，包括statistic的部分，都清除为init value;
	                for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
	                {
	                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
	                    {
	                       if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== mthl_send_data_req->sock_id)
	                       {
	                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr() match sock_id.");

	                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
	                         g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;

							 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_statistics_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id);
							 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;
							 g_mthl_create_conn_soc_save[i].sock_num--;

							 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, SOC_ERROR, done.");
						   	 break;
	                       }
						   else
						   	 continue;
	                    }
						if(j<MTHL_MAX_IP_SOCKET_NUM)
						  break;
						else
						  continue;
	                }

					//清除失败；
					if(i == MTHL_GPRS_MAX_PDP_NUM)
					{
						LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, fail, not match sock_id..");
						LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): mthl_close_req->sock_id = %d.", mthl_send_data_req->sock_id);
					}
					//清除成功；
					else
					{
					    LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, success.");
						LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): mthl_close_req->sock_id = %d.", mthl_send_data_req->sock_id);

						mthl_close_ind.result    = TRUE;
						mthl_close_ind.sock_id   = mthl_send_data_req->sock_id;
						mthl_close_ind.user_data = 0;

					    //返回success IND;
						mthl_close_sock_ind(&mthl_close_ind);
					}
				 }
				 else
				 	LogWriter::LOGX("[MTHL] send data fail: soc_close ret = %d.",close_ret);
		     }
		     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
			 mthl_send_data_cnf.result    = FALSE;
			 mthl_send_data_cnf.ret_val   = ret;
			 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): TCP send fail, ret = %d.", ret);

			 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

			 //返回cnf;
			 mthl_send_sock_cnf(&mthl_send_data_cnf);
		     return;
		}
	 }

	 //UDP case;
	 if(g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type == SOC_SOCK_DGRAM)
	 {
	    //soc_sendto() for UDP
	    LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send case.");

	    //发送时，UDP当前指定接收地址；
	    if(mthl_send_data_req->addr.port !=0 && mthl_send_data_req->addr.addr[0] != 0)
	    {
	       LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, current set addr:");
		   LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, port = %d, addr = %d.%d.%d.%d",
		   	                mthl_send_data_req->addr.port, mthl_send_data_req->addr.addr[0], mthl_send_data_req->addr.addr[1],mthl_send_data_req->addr.addr[2],mthl_send_data_req->addr.addr[3]);

		   ret = soc_sendto(mthl_send_data_req->sock_id, mthl_send_data_req->data, mthl_send_data_req->size, mthl_send_data_req->flags, &(mthl_send_data_req->addr));
			//发送动作call完，就释放buff，后续user会重发；
	        WinUtil::L_FREE(mthl_send_data_req->data);
			mthl_send_data_req->data = NULL;

			//立即发送成功；
			if(ret > 0)
			{
			     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
				 mthl_send_data_cnf.result    = TRUE;
				 mthl_send_data_cnf.ret_val   = ret;
				 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send ok 1, ret = %d.", ret);
               /*
                 butterfly.pan 160704
                 ret=23
                 ret=25
               */

				 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_send_size += ret;
				 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

				 //返回cnf;
				 mthl_send_sock_cnf(&mthl_send_data_cnf);
			 	 return;
			}
			//后续指示发送结果；
			else if(ret == SOC_WOULDBLOCK)
			{
			     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
				 mthl_send_data_cnf.result    = FALSE;
				 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send WOULDBLOCK case.");
				 mthl_send_data_cnf.ret_val   = ret;
				 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

				 //返回cnf;
				 mthl_send_sock_cnf(&mthl_send_data_cnf);
			     return;
			}
			//立即发送失败；
			else
			{
			     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
				 mthl_send_data_cnf.result    = FALSE;
				 mthl_send_data_cnf.ret_val   = ret;
				 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send fail, ret = %d.", ret);

				 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

				 //返回cnf;
				 mthl_send_sock_cnf(&mthl_send_data_cnf);
			     return;
			}
	    }
        //发送时，UDP当前指定接收地址，但是传入的地址不合法；
		if((mthl_send_data_req->addr.port ==0 && mthl_send_data_req->addr.addr[0] != 0)||(mthl_send_data_req->addr.port !=0 && mthl_send_data_req->addr.addr[0] == 0))
		{
            LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, current set error addr:");
            LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, port = %d, addr = %d.%d.%d.%d",
                        mthl_send_data_req->addr.port, mthl_send_data_req->addr.addr[0], mthl_send_data_req->addr.addr[1],mthl_send_data_req->addr.addr[2],mthl_send_data_req->addr.addr[3]);

            mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
            mthl_send_data_cnf.result    = FALSE;
            mthl_send_data_cnf.ret_val   = SOC_INVAL;
            mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

            //返回cnf;
            mthl_send_sock_cnf(&mthl_send_data_cnf);
        		WinUtil::L_FREE(mthl_send_data_req->data);
				mthl_send_data_req->data = NULL;
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): free_ctrl_buffer().");
            return;
		}

		//发送时，UDP当前未指定接收地址，则:使用之前指定的默认地址；
		if(mthl_send_data_req->addr.port ==0 && mthl_send_data_req->addr.addr[0] == 0)
		{
		   //之前指定了默认地址；
		   if(g_mthl_create_conn_soc_save[i].socket_info[j].addr.port != 0 && g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0]!=0)
		   {
		        LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, default addr:");
				LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, port = %d, addr = %d.%d.%d.%d",
		   	                g_mthl_create_conn_soc_save[i].socket_info[j].addr.port,
		   	                g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0],
		   	                g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1],
		   	                g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2],
		   	                g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3]);

		        ret = soc_sendto(mthl_send_data_req->sock_id, mthl_send_data_req->data, mthl_send_data_req->size, mthl_send_data_req->flags, &(g_mthl_create_conn_soc_save[i].socket_info[j].addr));
				//发送动作call完，就释放buff，后续user会重发；
		        WinUtil::L_FREE(mthl_send_data_req->data);
				mthl_send_data_req->data = NULL;

				//立即发送成功；
				if(ret > 0)
				{
				     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
					 mthl_send_data_cnf.result    = TRUE;
					 mthl_send_data_cnf.ret_val   = ret;
					 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send ok 2, ret = %d.", ret);

					 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_send_size += ret;
					 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

					 //返回cnf;
					 mthl_send_sock_cnf(&mthl_send_data_cnf);
				 	 return;
				}
				//后续指示发送结果；
				else if(ret == SOC_WOULDBLOCK)
				{
				     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
					 mthl_send_data_cnf.result    = FALSE;
					 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send WOULDBLOCK case.");

					 mthl_send_data_cnf.ret_val   = ret;
					 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

					 //返回cnf;
					 mthl_send_sock_cnf(&mthl_send_data_cnf);
				     return;
				}
				//立即发送失败；
				else
				{
				     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
					 mthl_send_data_cnf.result    = FALSE;
					 mthl_send_data_cnf.ret_val   = ret;
					 LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send fail, ret = %d.", ret);

					 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

					 //返回cnf;
					 mthl_send_sock_cnf(&mthl_send_data_cnf);
				     return;
				}
		   }
		   //之前未指定有效的默认地址；
            else
            {
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, error default addr:");
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): UDP send, port = %d, addr = %d.%d.%d.%d",
                            g_mthl_create_conn_soc_save[i].socket_info[j].addr.port,
                            g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0],
                            g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1],
                            g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2],
                            g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3]);

                mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
                mthl_send_data_cnf.result    = FALSE;
                mthl_send_data_cnf.ret_val   = SOC_INVAL;
                mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

                //返回cnf;
                mthl_send_sock_cnf(&mthl_send_data_cnf);
				WinUtil::L_FREE(mthl_send_data_req->data);
				mthl_send_data_req->data = NULL;
                LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): free_ctrl_buffer().");
                return;
            }
		}
	 }

	 //非TCP/UDP的异常case；
	 LogWriter::LOGX("[MTHL] send data: no TCP & UDP case.");
     mthl_send_data_cnf.sock_id   = mthl_send_data_req->sock_id;
	 mthl_send_data_cnf.result    = FALSE;
	 mthl_send_data_cnf.ret_val   = SOC_OPNOTSUPP;
	 mthl_send_data_cnf.user_data = mthl_send_data_req->user_data;

	 //返回cnf;
	 mthl_send_sock_cnf(&mthl_send_data_cnf);
	 //异常返回前，释放buff，后续user会重发；
     WinUtil::L_FREE(mthl_send_data_req->data);
	 mthl_send_data_req->data = NULL;
     LogWriter::LOGX("[MTHL] mthl_send_req_hdlr(): L_FREE.");
	 return;
}

void mthl_recv_req_hdlr(MSG *msg)
{
    mthl_recv_data_req_struct   *mthl_recv_data_req;
	mthl_recv_data_cnf_struct    mthl_recv_data_cnf;
	mthl_close_sock_ind_struct   mthl_close_ind;
    INT32 ret = 0;
	INT32 close_ret = 0;
	int i=0,j=0;
	UINT8 *buff = NULL;

	buff = (UINT8*)WinUtil::L_MALLOC(512);
	memset(buff, 0x00, 512);

	mthl_recv_data_req = (mthl_recv_data_req_struct*)msg->wParam;
	memset(&mthl_recv_data_cnf, 0x00, sizeof(mthl_recv_data_cnf_struct));

	// 根据sock_id 查询对应的socket_type；
                for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
                {
                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
                    {
                       if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== mthl_recv_data_req->sock_id)
                       {
                         LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): match sock_id.");
                         LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
					   	 break;
                       	}
					   else
					   	 continue;
                    }
					if(j<MTHL_MAX_IP_SOCKET_NUM)
					{
					  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): j= %d.", j);
					  break;
					}
					else
					  continue;
                }

     //sock_id匹配失败；
	 if(i == MTHL_GPRS_MAX_PDP_NUM)
	 {
                 mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
				 mthl_recv_data_cnf.result    = FALSE;
				 mthl_recv_data_cnf.ret_val   = SOC_INVALID_SOCKET;
				 mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

				 //返回cnf;
				 mthl_recv_sock_cnf(&mthl_recv_data_cnf);
				 LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): not match sock_id.");

				 //接收异常返回，MTHL直接自己free memory;
				 WinUtil::L_FREE(buff);
				 buff = NULL;

			     return;
	 }

	// 根据sock_id 查询对应的socket_type ；
	//TCP case;
	 if(g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type == SOC_SOCK_STREAM)
	 {
	     // soc_recv() for TCP;
	     LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): TCP recv case.");

	     ret = soc_recv(mthl_recv_data_req->sock_id, buff, 512, mthl_recv_data_req->flags);

		                 //立即接收成功；
						 if(ret > 0)
						 {
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = TRUE;

							  //buff指针传回，以便ATCI free memory;
							  mthl_recv_data_cnf.buff      = buff;
							  mthl_recv_data_cnf.ret_val   = ret;

							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): TCP recv ok, ret = %d.", ret);

							  g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_recv_size += ret;
							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
						 //后续指示接收结果；
						 else if(ret == SOC_WOULDBLOCK)
						 {
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = FALSE;

							  //mthl_recv_data_cnf.buff      = buff;
							  mthl_recv_data_cnf.ret_val   = ret;

							  //接收异常返回，MTHL直接自己free memory;
							 WinUtil::L_FREE(buff);
							 buff = NULL;

							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): TCP recv WOULDBLOCK case.");

							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
						 //立即接收失败；
						 else
						 {
						     //对于TCP 立即接收失败的Case:由于对端强制关闭、或网络、bearer异常导致TCP连接出错，
						     //ret为-1，对方强行关闭连接无close sock FIN包，释放socket;
						     //[注]: ret为0，对方正常关闭连接，即: close sock FIN包，后续会有CLOSE_IND；
						     if(ret == SOC_ERROR || ret == SOC_CONNABORTED || ret == SOC_BEARER_FAIL || ret == SOC_CONNRESET)
						     {
						         close_ret = soc_close(mthl_recv_data_req->sock_id);

								 if(close_ret == SOC_INVALID_SOCKET)
								 	LogWriter::LOGX("[MTHL] recv data fail: soc_close ret = SOC_INVALID_SOCKET");
								 else if(close_ret == SOC_SUCCESS)
								 {
								    //通过sock_id匹配，把close的socket id，包括statistic的部分，都清除为init value;
					                for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
					                {
					                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
					                    {
					                       if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== mthl_recv_data_req->sock_id)
					                       {
					                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr() match sock_id.");

					                         LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
					                         g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;

											 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear ,g_mthl_create_conn_soc_save[%d].socket_statistics_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id);
											 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;
											 g_mthl_create_conn_soc_save[i].sock_num--;

											 LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, SOC_ERROR, done.");
										   	 break;
					                       }
										   else
										   	 continue;
					                    }
										if(j<MTHL_MAX_IP_SOCKET_NUM)
										  break;
										else
										  continue;
					                }

									//清除失败；
									if(i == MTHL_GPRS_MAX_PDP_NUM)
									{
										LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, fail, not match sock_id..");
										LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): mthl_close_req->sock_id = %d.", mthl_recv_data_req->sock_id);
									}
									else
									{
									    LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): clear init value, success.");
										LogWriter::LOGX("[MTHL] mthl_close_req_hdlr(): mthl_close_req->sock_id = %d.", mthl_recv_data_req->sock_id);

										mthl_close_ind.result    = TRUE;
										mthl_close_ind.sock_id   = mthl_recv_data_req->sock_id;
										mthl_close_ind.user_data = 0;

									    //返回success IND;
										mthl_close_sock_ind(&mthl_close_ind);
									}
								 }
								 else
								 	LogWriter::LOGX("[MTHL] send data fail: soc_close ret = %d.",close_ret);
						     }
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = FALSE;
							  //mthl_recv_data_cnf.buff      = buff;
							  mthl_recv_data_cnf.ret_val   = ret;
							  //接收异常返回，MTHL直接自己free memory;
							  WinUtil::L_FREE(buff);
							  buff = NULL;

							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): TCP recv fail, ret = %d.", ret);
							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
	 }

	//UDP case;
	if(g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type == SOC_SOCK_DGRAM)
	{
	    //soc_recvfrom ()  for UDP
	    LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): UDP recv case.");

	    ret = soc_recvfrom(mthl_recv_data_req->sock_id, buff, 512, mthl_recv_data_req->flags,&(mthl_recv_data_cnf.addr));

                         //立即接收成功；
						 if(ret > 0)
						 {
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = TRUE;

							  //buff指针传回，以便ATCI free memory;
							  mthl_recv_data_cnf.buff      = buff;
							  mthl_recv_data_cnf.ret_val   = ret;
							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): UDP recv ok, ret = %d.", ret);

							  g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_recv_size += ret;
							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
						 //后续指示接收结果；
						 else if(ret == SOC_WOULDBLOCK)
						 {
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = FALSE;
							  //mthl_recv_data_cnf.buff      = buff;
							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): UDP recv WOULDBLOCK case.");

							  mthl_recv_data_cnf.ret_val   = ret;
							  //接收异常返回，MTHL直接自己free memory;
							 WinUtil::L_FREE(buff);
							 buff = NULL;

							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
						 //立即接收失败；
						 else
						 {
							  mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
							  mthl_recv_data_cnf.result    = FALSE;
							  //mthl_recv_data_cnf.buff      = buff;
							  mthl_recv_data_cnf.ret_val   = ret;
							  //接收异常返回，MTHL直接自己free memory;
							 WinUtil::L_FREE(buff);
							 buff = NULL;

							  LogWriter::LOGX("[MTHL] mthl_recv_req_hdlr(): UDP recv fail, ret = %d.", ret);

							  mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

							  //返回cnf;
							  mthl_recv_sock_cnf(&mthl_recv_data_cnf);
							  return;
						 }
	}

     //非TCP/UDP的异常case；
     LogWriter::LOGX("[MTHL] recv data: no TCP & UDP case.");
     mthl_recv_data_cnf.sock_id   = mthl_recv_data_req->sock_id;
	 mthl_recv_data_cnf.result    = FALSE;
	 //mthl_recv_data_cnf.buff      = buff;
	 mthl_recv_data_cnf.ret_val   = SOC_OPNOTSUPP;
	 //接收异常返回，MTHL直接自己free memory;
	 WinUtil::L_FREE(buff);
	 buff = NULL;

	 mthl_recv_data_cnf.user_data = mthl_recv_data_req->user_data;

	  //返回cnf;
	 mthl_recv_sock_cnf(&mthl_recv_data_cnf);
	 return;

}

void mthl_query_info_req_hdlr(MSG *msg)
{
    mthl_query_sock_info_req_struct   *mthl_query_sock_info_req;
    mthl_query_sock_info_cnf_struct    mthl_query_sock_info_cnf;
    int i,j;

    mthl_query_sock_info_req = (mthl_query_sock_info_req_struct*)msg->wParam;
    memset(&mthl_query_sock_info_cnf, 0x00, sizeof(mthl_query_sock_info_cnf_struct));

	            // 根据sock_id 查询计数结果；
	            for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
                {
                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
                    {
                       if(g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id== mthl_query_sock_info_req->sock_id)
                       {
                         LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): match sock_id.");
                         LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_statistics_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id);
					   	 break;
                       }
					   else
					   	 continue;
                    }
					if(j<MTHL_MAX_IP_SOCKET_NUM)
					  break;
					else
					  continue;
                }

			     //sock_id匹配失败；
				 if(i == MTHL_GPRS_MAX_PDP_NUM)
				 {
			                 mthl_query_sock_info_cnf.sock_id   = mthl_query_sock_info_req->sock_id;
							 mthl_query_sock_info_cnf.result    = FALSE;
							 //mthl_send_data_cnf.ret_val   = ret;
							 mthl_query_sock_info_cnf.user_data = mthl_query_sock_info_req->user_data;

							 //返回cnf;
							 mthl_query_info_sock_cnf(&mthl_query_sock_info_cnf);
                             LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): not match sock_id.");
						     return;
				 }

	//已经接收的Data Size
	// ACKED的值，即：发送到对端并被确认的ack值；通过sockopt获取；
	// 发送到对端并被确认的数据的Size；
	LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): match sock_id, query info:");

	mthl_query_sock_info_cnf.sock_id         = mthl_query_sock_info_req->sock_id;
	LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): mthl_query_sock_info_req->sock_id = %d", mthl_query_sock_info_req->sock_id);

	mthl_query_sock_info_cnf.result          = TRUE;

	mthl_query_sock_info_cnf.total_send_size = g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_send_size;
	LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): mthl_query_sock_info_cnf.total_send_size = %ld", mthl_query_sock_info_cnf.total_send_size);

	mthl_query_sock_info_cnf.total_recv_size = g_mthl_create_conn_soc_save[i].socket_statistics_info[j].total_recv_size;
	LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): mthl_query_sock_info_cnf.total_recv_size = %ld", mthl_query_sock_info_cnf.total_recv_size);

	soc_getsockopt(mthl_query_sock_info_req->sock_id, SOC_TCP_ACKED_DATA, &(mthl_query_sock_info_cnf.Ack_value), sizeof(mthl_query_sock_info_cnf.Ack_value));
	LogWriter::LOGX("[MTHL] mthl_query_info_req_hdlr(): mthl_query_sock_info_cnf.Ack_value = %ld", mthl_query_sock_info_cnf.Ack_value);
	mthl_query_sock_info_cnf.user_data       = mthl_query_sock_info_req->user_data;

	//返回cnf;
	mthl_query_info_sock_cnf(&mthl_query_sock_info_cnf);
    return;
}

void mthl_get_host_by_name_req_hdlr(MSG* msg)
{
    mthl_get_host_by_name_req_struct   *mthl_get_host_by_name_req;
    mthl_get_host_by_name_cnf_struct    mthl_get_host_by_name_cnf;
    INT8 ret;
    int i;
    UINT32 param_index;
    mthl_get_host_by_name_req = (mthl_get_host_by_name_req_struct*)msg->wParam;
    memset(&mthl_get_host_by_name_cnf, 0x00, sizeof(mthl_get_host_by_name_cnf_struct));


	//维护request_id;//req/rsp成对出现，可以从0,1,2…;

	//nwk_account_id;//encode了ori_acct_id\sim_id\app_id；
	        //查找一个可用的account_id；
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        if(g_mthl_create_pdp_param_save[i].account_id != CBM_INVALID_NWK_ACCT_ID)
        {
            //ori_acct_id查找成功;
            LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): match account_id.");
            LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d", i,g_mthl_create_pdp_param_save[i].account_id);
            param_index = i;
            break;
        }
        else
        continue;
    }

    //account_id匹配失败或domain_name为0无效参数；
    if(i == MTHL_GPRS_MAX_PDP_NUM || strlen(mthl_get_host_by_name_req->domain_name)==0)
    {
        mthl_get_host_by_name_cnf.result = FALSE;
        mthl_get_host_by_name_cnf.user_data = mthl_get_host_by_name_req->user_data;

        //返回失败，还要把req带过来的domain name信息返回；
        mthl_get_host_by_name_cnf.domain_len = strlen(mthl_get_host_by_name_req->domain_name);
        strncpy(mthl_get_host_by_name_cnf.domain_name, mthl_get_host_by_name_req->domain_name, mthl_get_host_by_name_cnf.domain_len);

        //返回cnf;
        mthl_get_host_by_name_soc_cnf_mod_id(&mthl_get_host_by_name_cnf);
        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): not match account_id!");
        return;
    }

    mthl_get_host_by_name_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;

    //跑到此处说明domain_name有效，大于0；
    LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): domain_name > 0.");

    for(i = 0; i < MAX_GET_HOST_BY_NAME_NUM; i ++)
    {
        if(!g_get_host_by_name_save[i].bUsed)
            break;
    }

    //[Notes]: check之前的DNS query的异步case，是否有返回RSP，否则不能开始下一次查询；
    if(i == MAX_GET_HOST_BY_NAME_NUM)
    {
        //没有被清空，说明之前的DNS query没有返回RSP；
        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): DNS query async case: not RSP!");
        mthl_get_host_by_name_cnf.result = FALSE;
        mthl_get_host_by_name_cnf.user_data = mthl_get_host_by_name_req->user_data;

        //返回失败，还要把新一次req带过来的domain name信息返回；
        mthl_get_host_by_name_cnf.domain_len = strlen(mthl_get_host_by_name_req->domain_name);
        strncpy(mthl_get_host_by_name_cnf.domain_name, mthl_get_host_by_name_req->domain_name, mthl_get_host_by_name_cnf.domain_len);

        //返回cnf;
        mthl_get_host_by_name_soc_cnf(&mthl_get_host_by_name_cnf);
        return;
    }

    ret = soc_gethostbyname(FALSE,
                            i,
                            mthl_get_host_by_name_req->domain_name,
                            mthl_get_host_by_name_cnf.addr,
                            (UINT8 *)(&(mthl_get_host_by_name_cnf.addr_len)),
                            0,
                            g_mthl_create_pdp_param_save[param_index].nwk_acct_id);

    if(ret == SOC_SUCCESS)  /* 马上从DNS Cache中，得到结果*/
    {
        mthl_get_host_by_name_cnf.result = TRUE;
        mthl_get_host_by_name_cnf.domain_len = strlen(mthl_get_host_by_name_req->domain_name);
        strncpy(mthl_get_host_by_name_cnf.domain_name, mthl_get_host_by_name_req->domain_name, mthl_get_host_by_name_cnf.domain_len);

        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): DNS query from cache, Success: IP addr:");
        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): mthl_get_host_by_name_cnf.addr = %d.%d.%d.%d",
                     mthl_get_host_by_name_cnf.addr[0], mthl_get_host_by_name_cnf.addr[1],mthl_get_host_by_name_cnf.addr[2],mthl_get_host_by_name_cnf.addr[3]);
        mthl_get_host_by_name_cnf.user_data = mthl_get_host_by_name_req->user_data;

        //返回cnf;
        mthl_get_host_by_name_soc_cnf(&mthl_get_host_by_name_cnf);
        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): DNS query from cache, Success!");
    }
    else if(ret == SOC_WOULDBLOCK)  /* 后续返回结果*/
    {
        //保存请求带过来的domain name信息，用于后续返回；
        g_get_host_by_name_save[i].user_data = mthl_get_host_by_name_req->user_data;
        g_get_host_by_name_save[i].bUsed       = TRUE;
        LogWriter::LOGX("[MTHL] get_host_by_name: WOULDBLOCK case.");
    }
    //立即得到fail;
    else
    {
        mthl_get_host_by_name_cnf.result = FALSE;
        mthl_get_host_by_name_cnf.user_data = mthl_get_host_by_name_req->user_data;

        //返回失败，还要把req带过来的domain name信息返回；
        mthl_get_host_by_name_cnf.domain_len = strlen(mthl_get_host_by_name_req->domain_name);
        strncpy(mthl_get_host_by_name_cnf.domain_name, mthl_get_host_by_name_req->domain_name, mthl_get_host_by_name_cnf.domain_len);

        //返回cnf;
        mthl_get_host_by_name_soc_cnf(&mthl_get_host_by_name_cnf);
        LogWriter::LOGX("[MTHL] mthl_get_host_by_name_req_hdlr(): DNS query fail!");
    }

    return;

	//处理SOC上报的IND；// 查询结果，nwk_account_id， request_id进行区分；
}

void mthl_get_host_by_addr_req_hdlr(MSG *msg)
{
	mthl_get_host_by_addr_req_struct   *mthl_get_host_by_addr_req;
	mthl_get_host_by_addr_cnf_struct    mthl_get_host_by_addr_cnf;
    INT8 ret;
	int i,j = 0;
	UINT32 nwk_account_id;

	mthl_get_host_by_addr_req = (mthl_get_host_by_addr_req_struct*)msg->wParam;
	memset(&mthl_get_host_by_addr_cnf, 0x00, sizeof(mthl_get_host_by_addr_cnf_struct));


	//维护request_id;//req/rsp成对出现，可以从0,1,2…;

	//nwk_account_id;//encode了ori_acct_id\sim_id\app_id；
	        //查找一个可用的account_id；
            for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
			{
			    if(g_mthl_create_pdp_param_save[i].account_id != CBM_INVALID_NWK_ACCT_ID)
			    {
			        //ori_acct_id查找成功;
			        LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): match account_id.");
					LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): g_mthl_create_pdp_param_save[%d].account_id = %d", i,g_mthl_create_pdp_param_save[i].account_id);
			        nwk_account_id = cbm_encode_data_account_id(g_mthl_create_pdp_param_save[i].account_id, CBM_SIM_ID_SIM1 , g_mthl_atci_app_id, FALSE);
					break;
			    }
				else
					continue;
			}

            //account_id匹配失败或addr为0无效参数；
			if(i == MTHL_GPRS_MAX_PDP_NUM || (mthl_get_host_by_addr_req->addr[0]==0))
			{
			    mthl_get_host_by_addr_cnf.result = FALSE;
				mthl_get_host_by_addr_cnf.user_data = mthl_get_host_by_addr_req->user_data;

				//返回失败，还要把req带过来的addr信息返回；
				mthl_get_host_by_addr_cnf.addr_len = strlen((char*)mthl_get_host_by_addr_req->addr);
		        memcpy(mthl_get_host_by_addr_cnf.addr, mthl_get_host_by_addr_req->addr, mthl_get_host_by_addr_req->addr_len);

				//返回cnf;
				mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);
				LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): not match account_id!");
				return;
			}

            //跑到此处说明addr有效，不为0；
			LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): addr not 0.");
	  //[Notes]: check之前的IP Addr query的异步case，是否有返回RSP，否则不能开始下一次查询；
			if(g_get_host_by_addr_save.addr_len!= 0 || strlen((char*)g_get_host_by_addr_save.addr)>0)
			{
			    //没有被清空，说明之前的IP Addr query没有返回RSP；
			    LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): IP Addr query async case: not RSP!");
			    mthl_get_host_by_addr_cnf.result = FALSE;
				mthl_get_host_by_addr_cnf.user_data = mthl_get_host_by_addr_req->user_data;

				//返回失败，还要把新一次req带过来的domain name信息返回；
				mthl_get_host_by_addr_cnf.addr_len = strlen((char*)mthl_get_host_by_addr_req->addr);
		        memcpy(mthl_get_host_by_addr_cnf.addr, mthl_get_host_by_addr_req->addr, mthl_get_host_by_addr_req->addr_len);

				//返回cnf;
				mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);
				return;
			}

	 mthl_get_host_by_addr_cnf.domain_len = 128;
	 ret = soc_gethostbyaddr(FALSE,
                           g_request_id_for_gethostbyaddr,
                           mthl_get_host_by_addr_cnf.domain_name,
                           (UINT32 *)&mthl_get_host_by_addr_cnf.domain_len,
                           mthl_get_host_by_addr_req->addr,
                           (UINT8)mthl_get_host_by_addr_req->addr_len,
                           0,
                           nwk_account_id);

    //马上从DNS Cache中，得到结果；
	if(ret == SOC_SUCCESS)
	{
         mthl_get_host_by_addr_cnf.result = TRUE;
		 mthl_get_host_by_addr_cnf.addr_len = strlen((char*)mthl_get_host_by_addr_req->addr);
		 memcpy(mthl_get_host_by_addr_cnf.addr, mthl_get_host_by_addr_req->addr, mthl_get_host_by_addr_req->addr_len);
         mthl_get_host_by_addr_cnf.user_data = mthl_get_host_by_addr_req->user_data;

		 //返回cnf;
		 mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);
		 LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): IP Addr query from cache, Success: domain_name:");
		 LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): domain_name = %s", mthl_get_host_by_addr_cnf.domain_name);
		 //return;
	}
	//后续返回结果；
	else if(ret == SOC_WOULDBLOCK)
	{
	      //保存请求带过来的IP Addr 信息，用于后续返回；
	      g_get_host_by_addr_save.addr_len= mthl_get_host_by_addr_req->addr_len;
		  memcpy(g_get_host_by_addr_save.addr, mthl_get_host_by_addr_req->addr, g_get_host_by_addr_save.addr_len);
		  g_get_host_by_addr_save.user_data = mthl_get_host_by_addr_req->user_data;
	      LogWriter::LOGX("[MTHL] get_host_by_addr: WOULDBLOCK case.");
	}
	//立即得到fail;
	else
	{
                mthl_get_host_by_addr_cnf.result = FALSE;
				mthl_get_host_by_addr_cnf.user_data = mthl_get_host_by_addr_req->user_data;

				//返回失败，还要把req带过来的IP Addr 信息返回；
				mthl_get_host_by_addr_cnf.addr_len = strlen((char*)mthl_get_host_by_addr_req->addr);
		        memcpy(mthl_get_host_by_addr_cnf.addr, mthl_get_host_by_addr_req->addr, mthl_get_host_by_addr_req->addr_len);

				//返回cnf;
				mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);
				LogWriter::LOGX("[MTHL] mthl_get_host_by_addr_req_hdlr(): IP Addr query fail.");
				//return;
	}

	return;

	//处理SOC上报的IND；
}

void mthl_tcp_server_listen_and_accept_req_hdlr(MSG *msg)
{
    mthl_server_launch_req_struct   *mthl_server_launch_req;
    mthl_server_listen_cnf_struct    mthl_server_listen_cnf;
    mthl_server_accept_ind_struct    mthl_server_accept_ind;
    INT8 ret;
    INT8 error_soc_create= 0;
    int i=0;
    int j=0;
    UINT8 val;
    sockaddr_struct server_addr;
    sockaddr_struct conn_addr;

    mthl_server_launch_req = (mthl_server_launch_req_struct*)msg->wParam;
    memset(&mthl_server_listen_cnf, 0x00, sizeof(mthl_server_listen_cnf_struct));
    memset(&mthl_server_accept_ind, 0x00, sizeof(mthl_server_accept_ind_struct));

    //通过ori_acct_id找到g_nwk_acct_id；
    for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
    {
        if(mthl_server_launch_req->account_id == g_mthl_create_pdp_param_save[i].account_id)
        {
            //ori_acct_id匹配成功;
            break;
        }
        else
            continue;
    }

    //ori_acct_id匹配失败；
    if(i == MTHL_GPRS_MAX_PDP_NUM)
    {
        //mthl_server_listen_cnf.account_id = mthl_server_launch_req->account_id;
        mthl_server_listen_cnf.result     = FALSE;
        mthl_server_listen_cnf.sock_id    = SOC_ERROR;
        mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

        //返回cnf;
        mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
        return;
    }

    //目前只支持同时launch 1个Server；check之前是否launch Server；
    if(g_server_info.listen_sock_id != SOC_ERROR)
    {
        mthl_server_listen_cnf.result     = FALSE;
        mthl_server_listen_cnf.sock_id    = SOC_ERROR;
        mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

        //返回cnf;
        mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
        return;
    }
    //IP需要程序自己指定，只校验port即可；
    if(mthl_server_launch_req->port == 0)
    {
        mthl_server_listen_cnf.result     = FALSE;
        mthl_server_listen_cnf.sock_id    = SOC_ERROR;
        mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

        //返回cnf;
        mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
        return;
    }

    //TCP only;
    error_soc_create = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, g_mthl_create_pdp_param_save[i].nwk_acct_id);
    //TCP 创建 fail;
    if(error_soc_create < 0)
    {
        mthl_server_listen_cnf.result     = FALSE;
        mthl_server_listen_cnf.sock_id    = SOC_ERROR;
        mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

        //返回cnf;
        mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
        return;
    }

    //TCP 创建 success，即:Server listen_sock，设置为阻塞的；
    val = FALSE;
    soc_setsockopt(error_soc_create, SOC_NBIO, &val, sizeof(val));
    val = SOC_READ | SOC_WRITE | SOC_CONNECT | SOC_CLOSE | SOC_ACCEPT;
    soc_setsockopt(error_soc_create, SOC_ASYNC, &val, sizeof(val));

    memset(&server_addr, 0x00, sizeof(sockaddr_struct));
    server_addr.sock_type = SOC_SOCK_STREAM;

    #if !defined(__MTK_TARGET__)
    if(mthl_server_launch_req->port == 0)
        mthl_server_launch_req->port = 8888;//TCP Server port;
    /*if(mthl_server_launch_req->addr[0] ==0)
    {
    mthl_server_launch_req->addr[0]=10;
    mthl_server_launch_req->addr[1]=16;
    mthl_server_launch_req->addr[2]=3;
    mthl_server_launch_req->addr[3]=11;
    mthl_server_launch_req->addr_len = 4;
    }*/
    #endif
    server_addr.port      = mthl_server_launch_req->port;

    if(mthl_server_launch_req->addr_len != 0)
        server_addr.addr_len  = mthl_server_launch_req->addr_len;
    else
        server_addr.addr_len  = 4; //now,only support IPv4;

    #if !defined(__MTK_TARGET__)
    memcpy(server_addr.addr, mthl_server_launch_req->addr, mthl_server_launch_req->addr_len);
    #else
    LogWriter::LOGX("[MTHL] mthl_server_launch_req->addr:%d.%d.%d.%d",
        mthl_server_launch_req->addr[0],
        mthl_server_launch_req->addr[1],
        mthl_server_launch_req->addr[2],
        mthl_server_launch_req->addr[3]);
    //#define INADDR_ANY          (u_int32_t)0x00000000
    //memset(server_addr.addr, 0x00, mthl_server_launch_req->addr_len);
    //server_addr.addr_len = 0;
    //INADDR_ANY 还是connect fail;
    if(mthl_server_launch_req->addr[0] ==0)
    {
        LogWriter::LOGX("[MTHL] mthl_server_launch_req->addr[0] ==0 case.");
            mthl_server_launch_req->addr[0]=127;
            mthl_server_launch_req->addr[1]=0;
            mthl_server_launch_req->addr[2]=0;
            mthl_server_launch_req->addr[3]=1;
    }
    memcpy(server_addr.addr, mthl_server_launch_req->addr, server_addr.addr_len);
    #endif

    ret = soc_bind(error_soc_create, &server_addr);//wait event;

    //非异步；
    //success;
    if(ret == SOC_SUCCESS)
    {
        LogWriter::LOGX("[MTHL] soc_bind(): SOC_SUCCESS case.");
    }
    //各种case fail;
    else
    {
        mthl_server_listen_cnf.result     = FALSE;
        mthl_server_listen_cnf.sock_id    = error_soc_create;
        mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

        //返回cnf;
        mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
        //close socket;
        soc_close(error_soc_create);
        return;
    }

	ret = soc_listen(error_soc_create, 5);

	if(ret != SOC_WOULDBLOCK)
	{
	    //立即返回成功；
	    if(ret == SOC_SUCCESS)
	    {
	       		LogWriter::LOGX("[MTHL] soc_listen(): SOC_SUCCESS case.");
		        mthl_server_listen_cnf.result     = TRUE;
				mthl_server_listen_cnf.sock_id    = error_soc_create;
				mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

				//返回cnf;
				mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);

				//侦听成功后，保存listen_sock_id，再继续accept;
				g_server_info.listen_sock_id = error_soc_create;
	    }
		//立即返回失败；
		else
		{
		        mthl_server_listen_cnf.result     = FALSE;
				mthl_server_listen_cnf.sock_id    = error_soc_create;
				mthl_server_listen_cnf.user_data  = mthl_server_launch_req->user_data;

				//返回侦听cnf;
				mthl_tcp_server_listen_cnf(&mthl_server_listen_cnf);
				//close socket;
				soc_close(error_soc_create);
				return;
		}
	}
	else
	{
	    //Server listen_sock，默认是阻塞的，所以不会返回SOC_WOULDBLOCK；
	    LogWriter::LOGX("[MTHL] soc_listen(): SOC_WOULDBLOCK error case.");
	}

    //accept之前，再设置为异步非阻塞的；
    val = TRUE;
    soc_setsockopt(error_soc_create, SOC_NBIO, &val, sizeof(val));
    val = SOC_READ | SOC_WRITE | SOC_CONNECT | SOC_CLOSE | SOC_ACCEPT;
    soc_setsockopt(error_soc_create, SOC_ASYNC, &val, sizeof(val));
	//Server主程序，只需主动soc_accept()一次；
	ret = soc_accept(error_soc_create, &conn_addr);

	//立即返回结果；
	if(ret != SOC_WOULDBLOCK)
	{
	    //fail;
	    if(ret < 0)
	    {
	            mthl_server_accept_ind.conn_sock_id   = SOC_ERROR;
				//mthl_server_accept_ind.addr为初始值0；

				//返回cnf;
				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
				return;
	    }
		//success conn_sock_id;
		else
		{
		      mthl_server_accept_ind.conn_sock_id = ret;
			  //找到一个可用的socket_info[j]，再保存信息；
					for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
					{
					   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id == SOC_ERROR)
					   {
					        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, param:");

							g_mthl_create_conn_soc_save[i].socket_info[j].sock_id      = mthl_server_accept_ind.conn_sock_id;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);

							g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type  = SOC_SOCK_STREAM;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP case.");

				            //TCP 不用保存req带过来的要连接的Server地址；
							/*g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0] = mthl_create_conn_req->addr.addr[0];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1] = mthl_create_conn_req->addr.addr[1];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2] = mthl_create_conn_req->addr.addr[2];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3] = mthl_create_conn_req->addr.addr[3];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.port    = mthl_create_conn_req->addr.port;*/

							//保存信息ok，再计数；
							g_mthl_create_conn_soc_save[i].sock_num++;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].sock_num = %d",i,g_mthl_create_conn_soc_save[i].sock_num);

							//为流量统计再保存一份；
							g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = mthl_server_accept_ind.conn_sock_id;
							break;
					   }
					   else
					   	continue;
					}
                    //接收连接成功，保存失败；
					if(j == MTHL_MAX_IP_SOCKET_NUM)
					{
							mthl_server_accept_ind.conn_sock_id = SOC_LIMIT_RESOURCE;
							//memcpy(&(mthl_server_accept_ind.addr), &conn_addr, sizeof(sockaddr_struct));//保持初值0即可；

							LogWriter::LOGX("[MTHL] soc_accept(): success ,but save conn_sock_id fail case.");

							 //返回cnf;
			  				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
							//close socket;
							soc_close(mthl_server_accept_ind.conn_sock_id);
							return;
					}
					//接收连接成功，保存成功；
					else
					{
					        memcpy(&(mthl_server_accept_ind.addr), &conn_addr, sizeof(sockaddr_struct));
							LogWriter::LOGX("[MTHL] soc_accept(): success conn_sock_id case.");

						    //返回cnf;
			  				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
							return;
					}
			  //memcpy(&(mthl_server_accept_ind.addr), &conn_addr, sizeof(sockaddr_struct));
			  //LogWriter::LOGX("[MTHL] soc_accept(): success conn_sock_id case.");

			  //返回cnf;
			  //mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
		}
	}
	//后续异步通知，//处理SOC上报的IND；
	else
	{
	    //异步通知时，先保存一份account_id，以便后续匹配使用；
	    g_server_info.server_account_id = mthl_server_launch_req->account_id;
	    LogWriter::LOGX("[MTHL] soc_accept(): SOC_WOULDBLOCK case.");
	}

	return;
}

void mthl_send_create_pdp_param_cnf(mthl_create_pdp_param_cnf_struct *mthl_create_pdp_param_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_create_pdp_param_cnf_struct *create_pdp_param_cnf = NULL;
    create_pdp_param_cnf = (mthl_create_pdp_param_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_create_pdp_param_cnf_struct));

    create_pdp_param_cnf->account_id = mthl_create_pdp_param_cnf->account_id;
    create_pdp_param_cnf->result     = mthl_create_pdp_param_cnf->result;
    create_pdp_param_cnf->user_data  = mthl_create_pdp_param_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_CREATE_PDP_PARAM_CNF;
    pMsg->local_para_ptr = (local_para_struct*)create_pdp_param_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = create_pdp_param_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_query_pdp_param_cnf(mthl_query_pdp_param_cnf_struct *mthl_query_pdp_param_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_query_pdp_param_cnf_struct *query_pdp_param_cnf = NULL;
    query_pdp_param_cnf = (mthl_query_pdp_param_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_query_pdp_param_cnf_struct));

    query_pdp_param_cnf->result   = mthl_query_pdp_param_cnf->result;
    query_pdp_param_cnf->pdp_param_num = mthl_query_pdp_param_cnf->pdp_param_num;
    memcpy(query_pdp_param_cnf->pdp_param_info, mthl_query_pdp_param_cnf->pdp_param_info, sizeof(mthl_pdp_param_info_struct)*MTHL_GPRS_MAX_PDP_NUM);
    query_pdp_param_cnf->user_data = mthl_query_pdp_param_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_GET_PDP_PARAM_CNF;
    pMsg->local_para_ptr = (local_para_struct*)query_pdp_param_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = query_pdp_param_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_activate_pdp_cnf(mthl_activate_pdp_cnf_struct * mthl_activate_pdp_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_activate_pdp_cnf_struct *activate_pdp_cnf = NULL;
    activate_pdp_cnf = (mthl_activate_pdp_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_activate_pdp_cnf_struct));

    activate_pdp_cnf->account_id  = mthl_activate_pdp_cnf->account_id;
    activate_pdp_cnf->result      = mthl_activate_pdp_cnf->result;
    activate_pdp_cnf->error_cause = mthl_activate_pdp_cnf->error_cause;
    activate_pdp_cnf->user_data   = mthl_activate_pdp_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_ACTIVATE_PDP_CNF;
    pMsg->local_para_ptr = (local_para_struct*)activate_pdp_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = activate_pdp_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}


void mthl_send_deactivate_pdp_ind(mthl_deactivate_pdp_ind_struct * mthl_deactivate_pdp_ind)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_deactivate_pdp_ind_struct *deactivate_pdp_ind = NULL;
    deactivate_pdp_ind = (mthl_deactivate_pdp_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_deactivate_pdp_ind_struct));

    deactivate_pdp_ind->account_id   =  mthl_deactivate_pdp_ind->account_id;
    deactivate_pdp_ind->error        =  mthl_deactivate_pdp_ind->error;
    deactivate_pdp_ind->error_cause  =  mthl_deactivate_pdp_ind->error_cause;

    pMsg->id = MSG_ID_APP_MTHL_DEACTIVATED_PDP_IND;
    pMsg->local_para_ptr = (local_para_struct*)deactivate_pdp_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = -1;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_deactivate_pdp_cnf(mthl_deactivate_pdp_cnf_struct* mthl_deactivate_pdp_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_deactivate_pdp_cnf_struct *deactivate_pdp_cnf = NULL;
    deactivate_pdp_cnf = (mthl_deactivate_pdp_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_deactivate_pdp_cnf_struct));

    deactivate_pdp_cnf->account_id    =  mthl_deactivate_pdp_cnf->account_id;
    deactivate_pdp_cnf->result        =  mthl_deactivate_pdp_cnf->result;
    deactivate_pdp_cnf->error_cause   =  mthl_deactivate_pdp_cnf->error_cause;
    deactivate_pdp_cnf->user_data     =  mthl_deactivate_pdp_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_DEACTIVATE_PDP_CNF;
    pMsg->local_para_ptr = (local_para_struct*)deactivate_pdp_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = deactivate_pdp_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_query_pdp_state_cnf(mthl_query_pdp_state_cnf_struct *mthl_query_pdp_state_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_query_pdp_state_cnf_struct *query_pdp_state_cnf = NULL;
    query_pdp_state_cnf = (mthl_query_pdp_state_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_query_pdp_state_cnf_struct));

    query_pdp_state_cnf->result   = mthl_query_pdp_state_cnf->result;
    query_pdp_state_cnf->num      = mthl_query_pdp_state_cnf->num;
    memcpy(query_pdp_state_cnf->state_list, mthl_query_pdp_state_cnf->state_list, sizeof(UINT8)*3*2);
    query_pdp_state_cnf->user_data = mthl_query_pdp_state_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_GET_PDP_STATE_CNF;
    pMsg->local_para_ptr = (local_para_struct*)query_pdp_state_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = query_pdp_state_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}


void mthl_send_create_conn_cnf(mthl_create_conn_cnf_struct *mthl_create_conn_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_create_conn_cnf_struct *create_conn_cnf = NULL;
    create_conn_cnf = (mthl_create_conn_cnf_struct*)WinUtil::L_MALLOC(sizeof(mthl_create_conn_cnf_struct));

    create_conn_cnf->account_id   = mthl_create_conn_cnf->account_id;
    create_conn_cnf->result       = mthl_create_conn_cnf->result;
    create_conn_cnf->sock_id      = mthl_create_conn_cnf->sock_id;
    create_conn_cnf->user_data    = mthl_create_conn_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_CREATE_CONN_CNF;
    pMsg->local_para_ptr = (local_para_struct*)create_conn_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = create_conn_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}


void mthl_send_create_sock_ind(mthl_create_sock_ind_struct *mthl_create_sock_ind)
{
	MSG msg;
	PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_create_sock_ind_struct *create_sock_ind = NULL;
    create_sock_ind = (mthl_create_sock_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_create_sock_ind_struct));

    create_sock_ind->account_id = mthl_create_sock_ind->account_id;
    create_sock_ind->sock_id      = mthl_create_sock_ind->sock_id;
    create_sock_ind->user_data  = mthl_create_sock_ind->user_data;

	pMsg->id = MSG_ID_APP_MTHL_CREATE_SOCK_IND;
	pMsg->local_para_ptr = (local_para_struct*)create_sock_ind;

	msg.message = SIMU_RTOS_MSG_ID;
	msg.wParam = (WPARAM)pMsg;
	msg.lParam = create_sock_ind->user_data;

	SendToLoadLuaMessage(&msg);
}

void mthl_query_sock_num_cnf(mthl_query_sock_account_info_cnf_struct *mthl_query_sock_account_info_cnf)
{
	MSG msg;
	PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_query_sock_account_info_cnf_struct *query_sock_account_info_cnf = NULL;
    query_sock_account_info_cnf = (mthl_query_sock_account_info_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_query_sock_account_info_cnf_struct));

    query_sock_account_info_cnf->account_id = mthl_query_sock_account_info_cnf->account_id;
	query_sock_account_info_cnf->result     = mthl_query_sock_account_info_cnf->result;

	if(query_sock_account_info_cnf->result == TRUE)
	{
		memcpy(query_sock_account_info_cnf->socket_info, mthl_query_sock_account_info_cnf->socket_info, sizeof(sock_account_info_struct)*MTHL_MAX_IP_SOCKET_NUM);
		query_sock_account_info_cnf->sock_num   = mthl_query_sock_account_info_cnf->sock_num;
	}

	query_sock_account_info_cnf->user_data  = mthl_query_sock_account_info_cnf->user_data;

	pMsg->id = MSG_ID_APP_MTHL_QUERY_SOCK_NUM_CNF;
	pMsg->local_para_ptr = (local_para_struct*)query_sock_account_info_cnf;

	msg.message = SIMU_RTOS_MSG_ID;
	msg.wParam = (WPARAM)pMsg;
	msg.lParam = query_sock_account_info_cnf->user_data;

	SendToLoadLuaMessage(&msg);
}

void mthl_close_sock_cnf(mthl_close_sock_cnf_struct *mthl_close_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_close_sock_cnf_struct *close_cnf = NULL;
    close_cnf = (mthl_close_sock_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_close_sock_cnf_struct));

    close_cnf->result    = mthl_close_cnf->result;
    close_cnf->sock_id   = mthl_close_cnf->sock_id;
    close_cnf->user_data = mthl_close_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_CLOSE_CNF;
    pMsg->local_para_ptr = (local_para_struct*)close_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = close_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_close_sock_ind(mthl_close_sock_ind_struct *mthl_close_ind)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_close_sock_ind_struct *close_ind = NULL;
    close_ind = (mthl_close_sock_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_close_sock_ind_struct));

    close_ind->result    = mthl_close_ind->result;
    close_ind->sock_id   = mthl_close_ind->sock_id;
    close_ind->user_data = mthl_close_ind->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_CLOSE_IND;
    pMsg->local_para_ptr = (local_para_struct*)close_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = close_ind->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_sock_cnf(mthl_send_data_cnf_struct *mthl_send_data_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_send_data_cnf_struct *send_data_cnf = NULL;
    send_data_cnf = (mthl_send_data_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_send_data_cnf_struct));

    send_data_cnf->result    = mthl_send_data_cnf->result;
    send_data_cnf->ret_val   = mthl_send_data_cnf->ret_val;
    send_data_cnf->sock_id   = mthl_send_data_cnf->sock_id;
    send_data_cnf->user_data = mthl_send_data_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_SEND_CNF;
    pMsg->local_para_ptr = (local_para_struct*)send_data_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = send_data_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_sock_ind(mthl_send_data_ind_struct *mthl_send_data_ind)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_send_data_ind_struct *send_data_ind = NULL;
    send_data_ind = (mthl_send_data_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_send_data_ind_struct));

    send_data_ind->result    = mthl_send_data_ind->result;
    send_data_ind->sock_id   = mthl_send_data_ind->sock_id;
    send_data_ind->user_data = mthl_send_data_ind->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_SEND_IND;
    pMsg->local_para_ptr = (local_para_struct*)send_data_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = send_data_ind->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_recv_sock_cnf(mthl_recv_data_cnf_struct *mthl_recv_data_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_recv_data_cnf_struct *recv_data_cnf = NULL;
    recv_data_cnf = (mthl_recv_data_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_recv_data_cnf_struct));

    recv_data_cnf->sock_id = mthl_recv_data_cnf->sock_id;
    recv_data_cnf->result  = mthl_recv_data_cnf->result;
    recv_data_cnf->buff    = mthl_recv_data_cnf->buff;
    recv_data_cnf->ret_val = mthl_recv_data_cnf->ret_val;
    recv_data_cnf->addr    = mthl_recv_data_cnf->addr;
    recv_data_cnf->user_data = mthl_recv_data_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_RECV_CNF;
    pMsg->local_para_ptr = (local_para_struct*)recv_data_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = recv_data_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_recv_sock_ind(mthl_recv_data_ind_struct *mthl_recv_data_ind)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_recv_data_ind_struct *recv_data_ind = NULL;
    recv_data_ind = (mthl_recv_data_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_recv_data_ind_struct));

    recv_data_ind->sock_id = mthl_recv_data_ind->sock_id;
    recv_data_ind->result  = mthl_recv_data_ind->result;
    recv_data_ind->buff    = mthl_recv_data_ind->buff;
    recv_data_ind->ret_val = mthl_recv_data_ind->ret_val;
    recv_data_ind->addr    = mthl_recv_data_ind->addr;
    recv_data_ind->user_data = mthl_recv_data_ind->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SOCK_RECV_IND;
    pMsg->local_para_ptr = (local_para_struct*)recv_data_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = recv_data_ind->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_query_info_sock_cnf(mthl_query_sock_info_cnf_struct *mthl_query_sock_info_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_query_sock_info_cnf_struct *query_sock_info_cnf = NULL;
    query_sock_info_cnf = (mthl_query_sock_info_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_query_sock_info_cnf_struct));

    query_sock_info_cnf->sock_id   =   mthl_query_sock_info_cnf->sock_id;
    query_sock_info_cnf->result    =   mthl_query_sock_info_cnf->result;
    query_sock_info_cnf->total_send_size = mthl_query_sock_info_cnf->total_send_size;
    query_sock_info_cnf->total_recv_size = mthl_query_sock_info_cnf->total_recv_size;
    query_sock_info_cnf->Ack_value       = mthl_query_sock_info_cnf->Ack_value;
    query_sock_info_cnf->user_data       = mthl_query_sock_info_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_QUERY_SOCK_INFO_CNF;
    pMsg->local_para_ptr = (local_para_struct*)query_sock_info_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = query_sock_info_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}


void mthl_get_host_by_name_soc_cnf_mod_id(mthl_get_host_by_name_cnf_struct *mthl_get_host_by_name_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_get_host_by_name_cnf_struct *get_host_by_name_cnf = NULL;
    get_host_by_name_cnf = (mthl_get_host_by_name_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_get_host_by_name_cnf_struct));

    get_host_by_name_cnf->result     = mthl_get_host_by_name_cnf->result;

    get_host_by_name_cnf->domain_len = mthl_get_host_by_name_cnf->domain_len;
    if(get_host_by_name_cnf->domain_len > 0)
        strncpy(get_host_by_name_cnf->domain_name, mthl_get_host_by_name_cnf->domain_name, get_host_by_name_cnf->domain_len);

    get_host_by_name_cnf->addr_len   = mthl_get_host_by_name_cnf->addr_len;
    if(get_host_by_name_cnf->addr_len > 0)
        strncpy((char*)get_host_by_name_cnf->addr, (char*)mthl_get_host_by_name_cnf->addr, get_host_by_name_cnf->addr_len);

    get_host_by_name_cnf->user_data  = mthl_get_host_by_name_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_GET_HOST_BY_NAME_CNF;
    pMsg->local_para_ptr = (local_para_struct*)get_host_by_name_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = get_host_by_name_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_get_host_by_name_soc_cnf(mthl_get_host_by_name_cnf_struct *mthl_get_host_by_name_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_get_host_by_name_cnf_struct *get_host_by_name_cnf = NULL;
    get_host_by_name_cnf = (mthl_get_host_by_name_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_get_host_by_name_cnf_struct));

    get_host_by_name_cnf->result     = mthl_get_host_by_name_cnf->result;

    get_host_by_name_cnf->domain_len = mthl_get_host_by_name_cnf->domain_len;
    if(get_host_by_name_cnf->domain_len > 0)
        strncpy(get_host_by_name_cnf->domain_name, mthl_get_host_by_name_cnf->domain_name, get_host_by_name_cnf->domain_len);

    get_host_by_name_cnf->addr_len   = mthl_get_host_by_name_cnf->addr_len;
    if(get_host_by_name_cnf->addr_len > 0)
        strncpy((char*)get_host_by_name_cnf->addr, (char*)mthl_get_host_by_name_cnf->addr, get_host_by_name_cnf->addr_len);

    get_host_by_name_cnf->user_data  = mthl_get_host_by_name_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_GET_HOST_BY_NAME_CNF;
    pMsg->local_para_ptr = (local_para_struct*)get_host_by_name_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = get_host_by_name_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_get_host_by_addr_soc_cnf(mthl_get_host_by_addr_cnf_struct *mthl_get_host_by_addr_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_get_host_by_addr_cnf_struct *get_host_by_addr_cnf = NULL;
    get_host_by_addr_cnf = (mthl_get_host_by_addr_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_get_host_by_addr_cnf_struct));

    get_host_by_addr_cnf->result     = mthl_get_host_by_addr_cnf->result;

    get_host_by_addr_cnf->domain_len = mthl_get_host_by_addr_cnf->domain_len;
    if(get_host_by_addr_cnf->domain_len > 0)
        strncpy(get_host_by_addr_cnf->domain_name, mthl_get_host_by_addr_cnf->domain_name, get_host_by_addr_cnf->domain_len);

    get_host_by_addr_cnf->addr_len   = mthl_get_host_by_addr_cnf->addr_len;
    if(get_host_by_addr_cnf->addr_len > 0)
        strncpy((char*)get_host_by_addr_cnf->addr, (char*)mthl_get_host_by_addr_cnf->addr, get_host_by_addr_cnf->addr_len);

    get_host_by_addr_cnf->user_data  = mthl_get_host_by_addr_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_GET_HOST_BY_ADDR_CNF;
    pMsg->local_para_ptr = (local_para_struct*)get_host_by_addr_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = get_host_by_addr_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_tcp_server_listen_cnf(mthl_server_listen_cnf_struct *mthl_server_listen_cnf)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_server_listen_cnf_struct *server_listen_cnf = NULL;
    server_listen_cnf = (mthl_server_listen_cnf_struct *)WinUtil::L_MALLOC(sizeof(mthl_server_listen_cnf_struct));

    server_listen_cnf->result  	 = mthl_server_listen_cnf->result;
    server_listen_cnf->sock_id   = mthl_server_listen_cnf->sock_id;
    server_listen_cnf->user_data = mthl_server_listen_cnf->user_data;

    pMsg->id = MSG_ID_APP_MTHL_SERVER_LISTEN_CNF;
    pMsg->local_para_ptr = (local_para_struct*)server_listen_cnf;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = server_listen_cnf->user_data;

    SendToLoadLuaMessage(&msg);
}

void mthl_tcp_server_accept_ind(mthl_server_accept_ind_struct *mthl_server_accept_ind)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_server_accept_ind_struct *server_accept_ind = NULL;
    server_accept_ind = (mthl_server_accept_ind_struct *)WinUtil::L_MALLOC(sizeof(mthl_server_accept_ind_struct));

    server_accept_ind->conn_sock_id = mthl_server_accept_ind->conn_sock_id;
    memcpy(&(server_accept_ind->addr), &(mthl_server_accept_ind->addr), sizeof(sockaddr_struct));

    pMsg->id = MSG_ID_APP_MTHL_SERVER_ACCEPT_IND;
    pMsg->local_para_ptr = (local_para_struct*)server_accept_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = -1;

    SendToLoadLuaMessage(&msg);
}

void mthl_send_abm_acc_info(srv_dtcnt_db_trans_type_enum opcode, UINT32 account_id, cbm_bearer_enum bearer_type)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    mmi_abm_update_account_info_req_struct *info_req =
		(mmi_abm_update_account_info_req_struct *)WinUtil::L_MALLOC(sizeof(mmi_abm_update_account_info_req_struct));

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (opcode == SRV_DTCNT_DB_TRANS_ADD)
    {
        info_req->action = MMI_ABM_PROFILE_ADD;
        info_req->account_id = account_id;
        info_req->bearer_type = bearer_type;
    }
    else /* DELETE */
    {
		/* APP_INFO_MODIFY */
		if((bearer_type & CBM_CSD) || (bearer_type & CBM_GPRS) || (bearer_type & CBM_EDGE) || (bearer_type & CBM_UMTS))
		{
	        info_req->action = MMI_ABM_PROFILE_DEL;
	        info_req->account_id = account_id;
	        info_req->bearer_type = bearer_type;
		}
        else
        {
			WinUtil::L_FREE((local_para_struct *)info_req);
            return;
        }
    }
	
    mthl_send_msg_to_abm(MSG_ID_MMI_ABM_UPDATE_ACCOUNT_INFO_REQ, (void *)info_req);
}

void mthl_send_msg_to_abm(UINT16 msg_id, void *req)
{

}

//add for transparent transport mode begin

static void mthl_transparent_mode_uart_set_owner_id(UINT16 port, UINT16 ownerid)
{

}

static UINT16 mthl_transparent_mode_uart_get_owner_id(UINT16 port)
{
	return 0;
}

static UINT16 mthl_transparent_mode_uart_put_bytes(UINT16 port, UINT8 *Buffaddr, UINT16 Length, UINT16 ownerid)
{
	return 0;
}

static UINT16 mthl_transparent_mode_uart_get_bytes(UINT16 port, UINT8 *Buffaddr, UINT16 Length, UINT8 *status, UINT16 ownerid)
{
    return 0;
}


BOOL mthl_get_transparent_mode_status()
{

	LogWriter::LOGX("[MTHL] mthl_get_transparent_mode_status = %d",is_mthl_in_transparent_mode);
    return is_mthl_in_transparent_mode;
}

void mthl_set_transparent_mode_status(BOOL status)
{
    is_mthl_in_transparent_mode = status;
}

void mthl_transparent_req_get_udp_address(int i,int j)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	if(mthl_transparent_mode_context.addr.addr_len ==0)
	{
		mthl_transparent_mode_context.addr.addr[0] = g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0];
		mthl_transparent_mode_context.addr.addr[1] = g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1];
		mthl_transparent_mode_context.addr.addr[2] = g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2];
		mthl_transparent_mode_context.addr.addr[3] = g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3];
		mthl_transparent_mode_context.addr.addr_len = g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr_len;
		mthl_transparent_mode_context.addr.port = g_mthl_create_conn_soc_save[i].socket_info[j].addr.port;
	}
}

socket_type_enum mthl_get_socket_type_by_socket_id(INT8 socket_id)
{
    int i,j;

	// 根据sock_id 查询对应的socket_type；
	for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
	{
		for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
		{
			if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== socket_id)
				break;
			else
				continue;
		}
		if(j<MTHL_MAX_IP_SOCKET_NUM && i<MTHL_GPRS_MAX_PDP_NUM)
		{
		    mthl_transparent_req_get_udp_address(i,j);
			LogWriter::LOGX("[MTHL] mthl_get_socket_type_by_socket_id(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
			return g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type;
		}
		else
			continue;
	}

		 //sock_id匹配失败；
	if(i == MTHL_GPRS_MAX_PDP_NUM)
	{
		return SOC_SOCK_SMS;
	}

	return SOC_SOCK_SMS;
}

BOOL mthl_check_udp_address_valid(sockaddr_struct *sockaddr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

	if(sockaddr->port == 0 || sockaddr->addr_len == 0)
		return FALSE;
	if((sockaddr->addr[0]|sockaddr->addr[1]|sockaddr->addr[2]|sockaddr->addr[3])==0)
		return FALSE;

	return TRUE;
}

static void mthl_transparent_init_mem_size()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	if (mthl_transparent_mode_context.ibuff_head_ptr ==NULL)
    {
        mthl_transparent_mode_context.ibuff_head_ptr = mthl_transparent_mem_alloc();

        if (mthl_transparent_mode_context.ibuff_head_ptr ==NULL)
        {
            mthl_transparent_mode_context.GPRS_FLC_UL_LOCKED = TRUE;
            return;
        }
        else
        {
            mthl_transparent_mode_context.GPRS_FLC_UL_LOCKED = FALSE;
        }
    }
	if (mthl_transparent_mode_context.outbuff_head_ptr ==NULL)
    {
        mthl_transparent_mode_context.outbuff_head_ptr = mthl_transparent_mem_alloc();

        if (mthl_transparent_mode_context.outbuff_head_ptr ==NULL)
        {
            mthl_transparent_mode_context.GPRS_FLC_DL_LOCKED = TRUE;
            return;
        }
        else
        {
            mthl_transparent_mode_context.GPRS_FLC_DL_LOCKED = FALSE;
        }
    }
}

void mthl_transparent_trans_req_hdlr(mthl_transparent_trans_req_struct *trans_p)
{
	MSG msg;
	PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

	socket_type_enum sock_type;
	mthl_transparent_trans_cnf_struct *trans_cnf;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

	trans_cnf = (mthl_transparent_trans_cnf_struct*)WinUtil::L_MALLOC(sizeof(mthl_transparent_trans_cnf_struct));
	trans_cnf->sock_id = trans_p->sock_id;
	trans_cnf->port = trans_p->port;
	trans_cnf->user_data = trans_p->user_data;
	mthl_transparent_mode_context.port = trans_p->port;

	sock_type = mthl_get_socket_type_by_socket_id(trans_p->sock_id);

	if(sock_type==SOC_SOCK_SMS)
	{
		trans_cnf->result = TRA_INVALID_SOCKET;
		goto send_cnf;
	}

	if(sock_type==SOC_SOCK_DGRAM)
	{
	    if(mthl_check_udp_address_valid(&trans_p->addr) == FALSE)
	    {
		if(mthl_check_udp_address_valid(&mthl_transparent_mode_context.addr)== FALSE)
		{
				trans_cnf->result = TRA_INVALID_PARAM;
			goto send_cnf;
		}
	}
		else
		{
			mthl_transparent_mode_context.addr.addr[0] = trans_p->addr.addr[0];
			mthl_transparent_mode_context.addr.addr[1] = trans_p->addr.addr[1];
			mthl_transparent_mode_context.addr.addr[2] = trans_p->addr.addr[2];
			mthl_transparent_mode_context.addr.addr[3] = trans_p->addr.addr[3];
			mthl_transparent_mode_context.addr.addr_len = trans_p->addr.addr_len;
			mthl_transparent_mode_context.addr.port = trans_p->addr.port;
		}

	}

	#ifdef __MTK_TARGET__

	//ret= set uart owner
	if(mthl_transparent_mode_uart_get_owner_id(trans_p->port)!= -1)
    {
        //UART_SetOwner (trans_p->port, MOD_MTHL);
		mthl_transparent_mode_uart_set_owner_id(trans_p->port, -1);
    }
	if(mthl_transparent_mode_uart_get_owner_id(trans_p->port)!= -1)
	{
		trans_cnf->result = TRA_GET_PORT_FAIL;
		goto send_cnf;
	}
	#endif

	mthl_set_transparent_mode_status(TRUE);

	trans_cnf->result = TRA_SUCCESS;
	mthl_transparent_mode_context.sock_id = trans_p->sock_id;
	mthl_transparent_mode_context.sock_type = sock_type;

	pMsg->id = MSG_ID_APP_MTHL_TRANSPARENT_TRANS_CNF;
	pMsg->local_para_ptr = (local_para_struct*)trans_cnf;

	msg.message = SIMU_RTOS_MSG_ID;
	msg.wParam = (WPARAM)pMsg;
	msg.lParam = trans_cnf->user_data;

	SendToLoadLuaMessage(&msg);

	mthl_transparent_init_mem_size();
	mthl_transparent_uart_ready_to_read_ind_hdlr(); /* Read UART */

	return;

	send_cnf:



	pMsg->id = MSG_ID_APP_MTHL_TRANSPARENT_TRANS_CNF;
	pMsg->local_para_ptr = (local_para_struct*)trans_cnf;

	msg.message = SIMU_RTOS_MSG_ID;
	msg.wParam = (WPARAM)pMsg;
	msg.lParam = trans_cnf->user_data;

	SendToLoadLuaMessage(&msg);

}


/*******************************************************************************
 * FUNCTION
 *    mthl_transparent_mem_alloc
 * DESCRIPTION
 *    This function is used to prepare the decode buff for the PPP framer.
 *
 * PARAMETERS
 *     IN/OUT    **flc_buff
 *     IN        is_data_mode
 *         IN        type
 * RETURNS
 *    ppp_buff_head_struct *
 * GLOBALS AFFECTED
 *    context of ppp framer
 *******************************************************************************/
mthl_transport_buff_head_struct* mthl_transparent_mem_alloc()
{
    mthl_transport_buff_head_struct * new_buff_ptr = NULL;

    /* Construct PPP input buffer header */
	new_buff_ptr = (mthl_transport_buff_head_struct *)WinUtil::L_MALLOC(sizeof(mthl_transport_buff_head_struct));
    memset((UINT8 *) new_buff_ptr, 0, sizeof(mthl_transport_buff_head_struct));


     new_buff_ptr->data_ptr = (UINT8 *)WinUtil::L_MALLOC(MAX_CTRL_PKT_SIZE);

     memset((UINT8 *)new_buff_ptr->data_ptr,0, MAX_CTRL_PKT_SIZE);
     new_buff_ptr->size = MAX_CTRL_PKT_SIZE;
     new_buff_ptr->end_ptr = new_buff_ptr->cur_ptr = new_buff_ptr->data_ptr;

    return new_buff_ptr;
}

void mthl_transparent_outbuffer_mem_dealloc ()
{
	/*----------------------------------------------------------------*/
	/* Local Variables												  */
	/*----------------------------------------------------------------*/

	/*----------------------------------------------------------------*/
	/* Code Body													  */
	/*----------------------------------------------------------------*/
	if(mthl_transparent_mode_context.outbuff_head_ptr!=NULL)
	{
		if (mthl_transparent_mode_context.outbuff_head_ptr->data_ptr !=NULL)
		{
			WinUtil::L_FREE(mthl_transparent_mode_context.outbuff_head_ptr->data_ptr);
			mthl_transparent_mode_context.outbuff_head_ptr->data_ptr = NULL;
		}
	}
	if (mthl_transparent_mode_context.outbuff_head_ptr !=NULL)
	{
		WinUtil::L_FREE(mthl_transparent_mode_context.outbuff_head_ptr);
		mthl_transparent_mode_context.outbuff_head_ptr = NULL;
	}
}

void mthl_transparent_inbuffer_mem_dealloc ()
{
	/*----------------------------------------------------------------*/
	/* Local Variables												  */
	/*----------------------------------------------------------------*/

	/*----------------------------------------------------------------*/
	/* Code Body													  */
	/*----------------------------------------------------------------*/
	if(mthl_transparent_mode_context.ibuff_head_ptr!=NULL)
	{
		if(mthl_transparent_mode_context.ibuff_head_ptr->data_ptr !=NULL)
		{
			WinUtil::L_FREE(mthl_transparent_mode_context.ibuff_head_ptr->data_ptr);
			mthl_transparent_mode_context.ibuff_head_ptr->data_ptr = NULL;
		}
	}
	if (mthl_transparent_mode_context.ibuff_head_ptr !=NULL)
	{
		WinUtil::L_FREE(mthl_transparent_mode_context.ibuff_head_ptr);
		mthl_transparent_mode_context.ibuff_head_ptr = NULL;
	}
}


void mthl_transparent_mem_dealloc ()
{
	/*----------------------------------------------------------------*/
	/* Local Variables												  */
	/*----------------------------------------------------------------*/

	/*----------------------------------------------------------------*/
	/* Code Body													  */
	/*----------------------------------------------------------------*/
	mthl_transparent_inbuffer_mem_dealloc();
	mthl_transparent_outbuffer_mem_dealloc();
		mthl_transparent_mode_context.GPRS_FLC_UL_LOCKED = TRUE;
		mthl_transparent_mode_context.GPRS_FLC_DL_LOCKED = TRUE;
}


static void mthl_transparent_send_data()
{
    INT32 ret;
    mthl_transport_buff_head_struct *ibuff_head_ptr = mthl_transparent_mode_context.ibuff_head_ptr;

	if(mthl_get_transparent_mode_status()==FALSE ||(mthl_transparent_mode_uart_get_owner_id (mthl_transparent_mode_context.port)!= -1))
		return;
   //TCP case;
	 if(mthl_transparent_mode_context.sock_type== SOC_SOCK_STREAM)
	 {
	    // soc_send()	for TCP
	    ret = soc_send(mthl_transparent_mode_context.sock_id, ibuff_head_ptr->cur_ptr, ibuff_head_ptr->used,0);

        //发送动作call完，就释放buff，后续user会重发；
        //WinUtil::L_FREE(mthl_send_data_req->data);
		//mthl_send_data_req->data = NULL;

		//立即发送成功；
		if(ret > 0)
		{
		     ibuff_head_ptr->cur_ptr += ret;
			 ibuff_head_ptr->used -= ret;
			 if(ibuff_head_ptr->used!= 0)
			 {
			 	mthl_transparent_send_data();
			 }
			 else
			 {
			 	mthl_transparent_inbuffer_mem_dealloc();
				mthl_transparent_reveive_data();
				mthl_transparent_uart_ready_to_read_ind_hdlr();
			 }
		}
		//后续指示发送结果；
		else if(ret == SOC_WOULDBLOCK)
		{

		     return;
		}
		//立即发送失败；
		else
		{
		    //error
		    mthl_transparent_mem_dealloc();
		    mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
		     return;
		}
	 }

	 //UDP case;
	 if(mthl_transparent_mode_context.sock_type== SOC_SOCK_DGRAM)
	 {
	    //soc_sendto() for UDP
	    //发送时，UDP当前指定接收地址；
	    if(mthl_check_udp_address_valid(&mthl_transparent_mode_context.addr)==TRUE)
	    {
		   ret = soc_sendto(mthl_transparent_mode_context.sock_id, mthl_transparent_mode_context.ibuff_head_ptr->cur_ptr,
		   	mthl_transparent_mode_context.ibuff_head_ptr->used,0,&mthl_transparent_mode_context.addr);
			//发送动作call完，就释放buff，后续user会重发；
	        //WinUtil::L_FREE(mthl_send_data_req->data);
			//mthl_send_data_req->data = NULL;

			//立即发送成功；
			if(ret > 0)
			{
				mthl_transparent_mode_context.ibuff_head_ptr->cur_ptr += ret;
				mthl_transparent_mode_context.ibuff_head_ptr->used -= ret;
				if(mthl_transparent_mode_context.ibuff_head_ptr->used!= 0)
				{
					mthl_transparent_send_data();
				}
				else
				{
					mthl_transparent_inbuffer_mem_dealloc();
					mthl_transparent_reveive_data();
					mthl_transparent_uart_ready_to_read_ind_hdlr();
				}
			}
			//后续指示发送结果；
			else if(ret == SOC_WOULDBLOCK)
			{
			     return;
			}
			//立即发送失败；
			else
			{
			     mthl_transparent_mem_dealloc();
			     mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
			     return;
			}
	    }
		else
		{
        //发送时，UDP当前指定接收地址，但是传入的地址不合法；
                 mthl_transparent_mem_dealloc();
                 mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
			     return;

		}
	 	}

	 return;
}


static void mthl_transparent_reveive_data()
{
    INT32 ret;
    mthl_transport_buff_head_struct *pDec;
    BOOL is_frame_decode = FALSE;
    BOOL is_release_flc_buff = FALSE;

    /* Locals in stack */
    UINT8 EscDect = 0x00;
    UINT32 is_read_suspend = FALSE;

	if(mthl_get_transparent_mode_status()==FALSE ||(mthl_transparent_mode_uart_get_owner_id(mthl_transparent_mode_context.port)!= -1))
		return;

    /* The input buff is NULL for new incoming packet */
    if (mthl_transparent_mode_context.outbuff_head_ptr ==NULL)
    {
        mthl_transparent_mode_context.outbuff_head_ptr = mthl_transparent_mem_alloc();

        if (mthl_transparent_mode_context.outbuff_head_ptr ==NULL)
        {
            mthl_transparent_mode_context.GPRS_FLC_DL_LOCKED = TRUE;
            return;
        }
        else
        {
        }
    }

	pDec = mthl_transparent_mode_context.outbuff_head_ptr;

	//TCP case;
	 if(mthl_transparent_mode_context.sock_type== SOC_SOCK_STREAM)
	 {
	     // soc_recv() for TCP;
	     ret = soc_recv(mthl_transparent_mode_context.sock_id, pDec->cur_ptr, pDec->size,0);
		 LogWriter::LOGX("[MTHL] soc_recv: = %s",pDec->cur_ptr);
         //立即接收成功；
		 if(ret > 0)
		 {
		     pDec->end_ptr += ret;
			 pDec->used += ret;
			 mthl_transparent_ready_to_write_uart_ind_hdlr();
			 if(pDec->used==0)
			 {
			 	mthl_transparent_reveive_data();
				//mthl_transparent_uart_ready_to_read_ind_hdlr();
			 }
			 else
			 {
			 	//WinUtil::L_FREE(mthl_transparent_mode_context.inbuff_context.data_ptr);
				 mthl_transparent_ready_to_write_uart_ind_hdlr();
			 }
		}
		 //后续指示接收结果；
		 else if(ret == SOC_WOULDBLOCK)
		 {
			  //if(pDec->used>0 && mthl_transparent_mode_context.is_write_suspend == FALSE)
			  	//mthl_process_output();
			  	//mthl_transparent_ready_to_write_uart_ind_hdlr();
			  	mthl_transparent_uart_ready_to_read_ind_hdlr();
		 }
		 //立即接收失败；
		 else
		 {
		      mthl_transparent_mem_dealloc();
			  mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
			  return;
		 }
	 }

	//UDP case;
	if(mthl_transparent_mode_context.sock_type== SOC_SOCK_DGRAM)
	{
	    //soc_recvfrom ()  for UDP
	    ret = soc_recvfrom(mthl_transparent_mode_context.sock_id, pDec->cur_ptr, pDec->size,0,&mthl_transparent_mode_context.addr);
		LogWriter::LOGX("[MTHL] soc_recvfrom: = %s, ret = %d",pDec->cur_ptr, ret);
         //立即接收成功；
		 if(ret > 0)
		 {
		     pDec->end_ptr += ret;
			 pDec->used += ret;
			 mthl_transparent_ready_to_write_uart_ind_hdlr();
			 if(pDec->used==0)
			 {
			 	mthl_transparent_reveive_data();
				//mthl_transparent_uart_ready_to_read_ind_hdlr();
			 }
			 else
			 {
			 	//WinUtil::L_FREE(mthl_transparent_mode_context.inbuff_context.data_ptr);
				 mthl_transparent_ready_to_write_uart_ind_hdlr();
			 }
		}
		 //后续指示接收结果；
		 else if(ret == SOC_WOULDBLOCK)
		 {
			  //return;
			  mthl_transparent_uart_ready_to_read_ind_hdlr();
		 }
		 //立即接收失败；
		 else
		 {
		      mthl_transparent_mem_dealloc();
			  mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
			  return;
		 }
	}

	 return;

}


/*******************************************************************************
 * FUNCTION
 *   mthl_process_input
 * DESCRIPTION
 *   This function is to process data from UART.
 *
 * PARAMETERS
 *    IN/OUT
 * RETURNS
 *   void
 * GLOBALS AFFECTED
 *   context of mthl_transparent_mode_context
 *******************************************************************************/
static void mthl_process_input ()
{
    /* Locals in Registers */
    mthl_transport_buff_head_struct *pDec;
    BOOL is_frame_decode = FALSE;
    BOOL is_release_flc_buff = FALSE;

    /* Locals in stack */
    UINT8 EscDect = 0x00;
    UINT32 is_read_suspend = FALSE;

	#ifndef __MTK_TARGET__

	UINT32 ret_send=0;
    char CRLF[] = {0x0d,0x0a,0x00};
    char send_buf[80]={0};
    memset(send_buf,0,sizeof(send_buf));
    strcat(send_buf,"GET / HTTP/1.1");
    strcat(send_buf,CRLF);
    strcat(send_buf,"host: www.baidu.com");
    strcat(send_buf,CRLF);
    strcat(send_buf,"Connection: Closed");
    strcat(send_buf,CRLF);
    strcat(send_buf,CRLF);
	#endif

	LogWriter::LOGX("[MTHL] mthl_process_input.");

    /* The input buff is NULL for new incoming packet */
    if (mthl_transparent_mode_context.ibuff_head_ptr ==NULL)
    {
        mthl_transparent_mode_context.ibuff_head_ptr = mthl_transparent_mem_alloc();

        if (mthl_transparent_mode_context.ibuff_head_ptr ==NULL)
        {
            mthl_transparent_mode_context.GPRS_FLC_UL_LOCKED = TRUE;
            return;
        }
        else
        {
        }
    }

	pDec = mthl_transparent_mode_context.ibuff_head_ptr;

	#ifdef __MTK_TARGET__

    while ((mthl_transparent_mode_uart_get_owner_id(mthl_transparent_mode_context.port)==MOD_MTHL) &&is_read_suspend==FALSE)
    {
        if (0 == pDec->used)
        {
            pDec->used =
                mthl_transparent_mode_uart_get_bytes(
                    mthl_transparent_mode_context.port,
                    pDec->data_ptr,
                    pDec->size,
                    &EscDect,
                    MOD_MTHL);


		   LogWriter::LOGX("[MTHL] mthl_transparent_mode_uart_get_bytes:used=%d, data = %s",pDec->used,pDec->data_ptr);
		   if(pDec->used>0)
		   	{
            pDec->end_ptr = pDec->data_ptr + pDec->used;
            pDec->cur_ptr = pDec->data_ptr;

		        if (pDec->used <= pDec->size)
            {
               // kal_trace (TRACE_WARNING, WARNING_UART_SUSPEND, pDec->used);
                mthl_transparent_mode_context.is_read_suspend = TRUE;
                is_read_suspend = TRUE;
            }
            else
            {
                //kal_trace (TRACE_GROUP_1, WARNING_UART_GETBYTE, pDec->used);
            }
        }
		    else
		    {
		    	return;
		    }
        }

    }

	#endif

	#ifndef __MTK_TARGET__

	pDec->used = strlen(send_buf);
	strncpy((char*)pDec->data_ptr,send_buf, pDec->used);
	pDec->end_ptr = pDec->data_ptr + pDec->used;
    pDec->cur_ptr = pDec->data_ptr;
	LogWriter::LOGX("[MTHL] mthl_transparent_mode_uart_get_bytes: = %s",pDec->data_ptr);

	#endif

	mthl_transparent_send_data();


    return;
}


static void mthl_transparent_uart_ready_to_read_ind_hdlr()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

	if (mthl_transparent_mode_context.GPRS_FLC_UL_LOCKED == TRUE)
    {
        /* We have no buffer to put */
        return;
    }

	mthl_transparent_mode_context.is_read_suspend = FALSE;
    mthl_process_input();
}


/*******************************************************************************
 * FUNCTION
 *   pppg_uart_ready_to_write_ind_hdlr
 * DESCRIPTION
 *   This function is to handle UART write resume.
 *
 * PARAMETERS
 *    IN	            ilm_ptr
 *    IN/OUT         ppp_ptr
 * RETURNS
 *   void
 * GLOBALS AFFECTED
 *   N/A
 *******************************************************************************/
void mthl_transparent_ready_to_write_uart_ind_hdlr ()
{
    /* Locals in Registers */
    mthl_transport_buff_head_struct *pDec = mthl_transparent_mode_context.outbuff_head_ptr;

    /* Locals in Stack */
    UINT16 w_len = 0;

	LogWriter::LOGX("[MTHL] mthl_transparent_ready_to_write_uart_ind_hdlr");

	mthl_transparent_mode_context.is_write_suspend = FALSE ;


    /* We should write the remaining data to the uart first! */
    if (mthl_transparent_mode_uart_get_owner_id(mthl_transparent_mode_context.port)==0)
    {
        /*w_len = UART_PutBytes (
                mthl_transparent_mode_context.port,
                pDec->cur_ptr,
                pDec->used,
                MOD_MTHL);*/


		LogWriter::LOGX("[MTHL] mthl_transparent_ready_to_write_uart_ind_hdlr: = %s",pDec->cur_ptr);

		w_len = mthl_transparent_mode_uart_put_bytes(
                mthl_transparent_mode_context.port,
                pDec->cur_ptr,
                pDec->used,
                -1);

		#ifndef __MTK_TARGET__

		w_len = pDec->used;

		#endif

		LogWriter::LOGX("[MTHL] mthl_transparent_mode_uart_put_bytes: = %d/%d",w_len,pDec->used);

		pDec->used    -= w_len;
        pDec->cur_ptr += w_len;

        if (w_len < pDec->used)
        {
            /* We start from cur_ptr while receive UART_READY_TO_WRITE */
            //pDec->is_write_suspend = TRUE;
            mthl_transparent_ready_to_write_uart_ind_hdlr();
            return;
        }
        else
        {
            mthl_transparent_outbuffer_mem_dealloc();
        }
    }// ~ MAUI_02991936, mtk02285, 20111111

}



/*******************************************************************************
 * FUNCTION
 *   mthl_transparent_uart_plugout_ind_hdlr
 * DESCRIPTION
 *   This function is to handle UART PLUG OUT IND.
 *
 * PARAMETERS
 *    IN
 *    IN
 * RETURNS
 *   void
 * GLOBALS AFFECTED
 *   N/A
 *******************************************************************************/
static void mthl_transparent_send_end_ind(INT8 port , transparent_error_cause_enum cause)
{
    MSG msg;
    PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    mthl_transparent_trans_end_ind_struct *trans_end_ind;
    /*----------------------------------------------------------------*/
    /* Code Body													  */
    /*----------------------------------------------------------------*/

    mthl_set_transparent_mode_status(FALSE);
    mthl_transparent_mode_context.is_read_suspend = TRUE;
    mthl_transparent_mode_context.is_write_suspend = TRUE;

    mthl_transparent_mem_dealloc();

    trans_end_ind = (mthl_transparent_trans_end_ind_struct*)WinUtil::L_MALLOC(sizeof(mthl_transparent_trans_end_ind_struct));
    trans_end_ind->sock_id = mthl_transparent_mode_context.sock_id;
    trans_end_ind->port= port;
    trans_end_ind->cause = cause;

    LogWriter::LOGX("[MTHL] mthl_transparent_send_end_ind,cause =%d,sock_id=%d",cause,trans_end_ind->sock_id);

    pMsg->id = MSG_ID_APP_MTHL_TRANSPARENT_TRANS_END_IND;
    pMsg->local_para_ptr = (local_para_struct*)trans_end_ind;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    msg.lParam = -1;

    SendToLoadLuaMessage(&msg);
}


/*******************************************************************************
 * FUNCTION
 *   mthl_transparent_uart_plugout_ind_hdlr
 * DESCRIPTION
 *   This function is to handle UART PLUG OUT IND.
 *
 * PARAMETERS
 *    IN
 *    IN
 * RETURNS
 *   void
 * GLOBALS AFFECTED
 *   N/A
 *******************************************************************************/
void mthl_transparent_uart_plugout_ind_hdlr (uart_plugout_ind_struct *plugout_p)
{
	/*----------------------------------------------------------------*/
	/* Local Variables												  */
	/*----------------------------------------------------------------*/
	/*----------------------------------------------------------------*/
	/* Code Body													  */
	/*----------------------------------------------------------------*/

	//ret= set uart owner
	if(mthl_transparent_mode_uart_get_owner_id(plugout_p->port)!= -1)
	{
		return;
	}
	mthl_transparent_mem_dealloc();
	mthl_transparent_send_end_ind(plugout_p->port,TRANSPARENT_PLUGOUT_DETECT);
}

/*******************************************************************************
 * FUNCTION
 *   mthl_transparent_uart_escape_detected_ind_hdlr
 * DESCRIPTION
 *       UART/USB will send UART_ESCAPE_DETECTED_IND to UART/USB owner if
 *       it detect ESCAPE string.
 * PARAMETERS
 *    IN     ilm_ptr
 *    IN     ppp_ptr  
 * RETURNS
 *   void
 * GLOBALS AFFECTED
 *   context of ppp_gprs
 *******************************************************************************/
void mthl_transparent_uart_escape_detected_ind_hdlr(uart_escape_detected_ind_struct *escape_p)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

	//ret= set uart owner
	if(mthl_transparent_mode_uart_get_owner_id(escape_p->port)!= -1)
    {
        return;
    }
	mthl_transparent_mem_dealloc();
	mthl_transparent_send_end_ind(escape_p->port,TRANSPARENT_ESC_DETECT);	    
}

void mthl_main_thread(LPVOID lparam)
{
    MSG msg;
    unsigned result = -1;

    mthl_init();

	while(1)
	{
		result = GetMessage(&msg, (HWND)-1, 0, 0);

		if(result == -1)
		{
			//TO DO ...
			break;
		}

		if(msg.message == WM_QUIT)
		{
			break;
		}

		switch(msg.message)
		{
		case MSG_ID_SOC_ABM_ACTIVATE_REQ:
                     send_soc_app_activate_req_hdlr(&msg);
                     break;

		case MSG_ID_SOC_ABM_ACTIVATE_CNF:
			soc_abm_activate_cnf_hdlr(&msg);
			break;

		case MSG_ID_SOC_ABM_DEACTIVATE_CNF:
			soc_abm_deactivate_cnf_hdlr(&msg);
			break;

		// PDP定义: 保存后续ACTIVATE_PDP时，需要使用的参数。
		case MSG_ID_APP_MTHL_CREATE_PDP_PARAM_REQ:
			mthl_send_create_pdp_param_req_hdlr(&msg);
			break;

		//PDP参数保存后，提供查询功能；
		case MSG_ID_APP_MTHL_GET_PDP_PARAM_REQ:
			mthl_get_pdp_param_req_hdlr(&msg);
			break;

		//内部PDP激活。
		case MSG_ID_APP_MTHL_ACTIVATE_PDP_REQ:
			mthl_send_activate_pdp_req_hdlr(&msg);
			break;


		//内部PDP去激活。
		case MSG_ID_APP_MTHL_DEACTIVATE_PDP_REQ:
			mthl_send_deactivate_pdp_req_hdlr(&msg);
			break;

		//内部WIFI去激活。
		#if defined(__WIFI_TCPIP_AT_SUPPORT__)
		case MSG_ID_APP_MTHL_DEACTIVATE_WIFI_REQ:
			mthl_send_deactivate_wifi_req_hdlr(ilm_ptr);
			break;
		#endif //__WIFI_TCPIP_AT_SUPPORT__

		case MSG_ID_APP_CBM_BEARER_INFO_IND:
		{
		app_cbm_bearer_info_ind_struct *bearer_info;
		int i;
		mthl_activate_pdp_cnf_struct   mthl_activate_pdp_cnf;
		mthl_deactivate_pdp_ind_struct mthl_deactivate_pdp_ind;
		mthl_deactivate_pdp_cnf_struct   mthl_deactivate_pdp_cnf;

		bearer_info = (app_cbm_bearer_info_ind_struct*)msg.wParam;

		for(i=0;i<MTHL_GPRS_MAX_PDP_NUM;i++)
		{
			if(bearer_info->account_id == g_mthl_create_pdp_param_save[i].nwk_acct_id)
			break;
		}

		if(g_mthl_create_pdp_param_save[i].bear_op == PDP_ACTIVATE_REQ)
		{
			//正在激活，等待后续IND上报的结果，先不返回CNF;
			if(bearer_info->state == SRV_CBM_ACTIVATING)
			{
				LogWriter::LOGX("activate_pdp_ind_hdlr: ACTIVATING.");
				return;
			}

			//激活成功；
			if(bearer_info->state == SRV_CBM_ACTIVATED && bearer_info->error == SRV_CBM_OK)
			{
				cbm_hold_bearer(g_mthl_atci_app_id);
				mthl_activate_pdp_cnf.result = TRUE;
				g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_CNF;
			}
			else
			{
				mthl_activate_pdp_cnf.result = FALSE;
				g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE;//激活失败；
			}

			LogWriter::LOGX("activate_pdp_ind_hdlr: result %d.", mthl_activate_pdp_cnf.result);

			mthl_activate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;
			mthl_activate_pdp_cnf.error_cause = bearer_info->error_cause;
			mthl_activate_pdp_cnf.user_data = g_mthl_create_pdp_param_save[i].user_data;

			//异步返回activate cnf;
			mthl_send_activate_pdp_cnf(&mthl_activate_pdp_cnf);
			return;
		}


		if(g_mthl_create_pdp_param_save[i].bear_op == PDP_DEACTIVATE_REQ)
		{
		//正在去激活，等待后续IND上报的结果，先不返回CNF;
		if(bearer_info->state == SRV_CBM_DEACTIVATING)
		{
			LogWriter::LOGX("deactivate_pdp_ind_hdlr: DEACTIVATING.");
			return;
		}

		//去激活成功；
		if(bearer_info->state == SRV_CBM_DEACTIVATED && bearer_info->error == SRV_CBM_OK)
		{
			mthl_deactivate_pdp_cnf.result = TRUE;
			g_mthl_create_pdp_param_save[i].bear_op = PDP_DEACTIVATE_CNF;//去激活成功；
		}
		else
		{
			mthl_deactivate_pdp_cnf.result = FALSE;
			g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE;//去激活失败；
		}

		mthl_deactivate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;
		mthl_deactivate_pdp_cnf.error_cause = bearer_info->error_cause;
		mthl_deactivate_pdp_cnf.user_data = g_mthl_create_pdp_param_save[i].user_data;

		//异步返回deactivate cnf;
		mthl_send_deactivate_pdp_cnf(&mthl_deactivate_pdp_cnf);
		return;
		}


		//主动上报内部PDP去激活的通知，MSG_ID_APP_MTHL_DEACTIVATED_PDP_IND;
		//SRV_CBM_BEARER_FAIL(MSG_ID_L4C_PS_ABM_DISC_IND上报后对应的错误类型)；
		if( g_mthl_create_pdp_param_save[i].bear_op == PDP_ACTIVATE_CNF ||
			g_mthl_create_pdp_param_save[i].bear_op == PDP_NONE)
		{
			if(bearer_info->state == SRV_CBM_DEACTIVATED && bearer_info->error == SRV_CBM_BEARER_FAIL)
			{
				mthl_deactivate_pdp_ind.account_id  = g_mthl_create_pdp_param_save[i].account_id;
				mthl_deactivate_pdp_ind.error       = bearer_info->error;
				mthl_deactivate_pdp_ind.error_cause = bearer_info->error_cause;

				//异步上报deactivate ind;
				mthl_send_deactivate_pdp_ind(&mthl_deactivate_pdp_ind);

				g_mthl_create_pdp_param_save[i].bear_op = PDP_DEACTIVATE_IND;
			}
			else
			{
				//再次上报激活成功；
				if(bearer_info->state == SRV_CBM_ACTIVATED && bearer_info->error == SRV_CBM_OK)
				{
					cbm_hold_bearer(g_mthl_atci_app_id);
					mthl_activate_pdp_cnf.result = TRUE;
					g_mthl_create_pdp_param_save[i].bear_op = PDP_ACTIVATE_CNF;
				}
				else
				{
					mthl_activate_pdp_cnf.result = FALSE;
					g_mthl_create_pdp_param_save[i].bear_op = PDP_NONE;//激活失败；
				}

				mthl_activate_pdp_cnf.account_id = g_mthl_create_pdp_param_save[i].account_id;
				mthl_activate_pdp_cnf.error_cause = bearer_info->error_cause;
				mthl_activate_pdp_cnf.user_data = g_mthl_create_pdp_param_save[i].user_data;

				//异步返回activate cnf;  如果ACTIVATING，不用再上次上报IND ，只需更新bear_op即可；
				if(mthl_activate_pdp_cnf.result == TRUE)
					mthl_send_activate_pdp_cnf(&mthl_activate_pdp_cnf);
			}
			return;
		}
		break;
		}

		//内部PDP激活、去激活操作后，提供查询PDP state 功能；
		case MSG_ID_APP_MTHL_GET_PDP_STATE_REQ:
			mthl_get_pdp_state_req_hdlr(&msg);
			break;


		//op为：建立socket，并connect 。
		case MSG_ID_APP_MTHL_CREATE_CONN_REQ:
			mthl_create_and_conn_req_hdlr(&msg);
			break;

		//处理conn/send/recv/close/accept 的async case;
		case MSG_ID_APP_SOC_NOTIFY_IND:
			mthl_soc_notify(&msg);
			break;


		//op为：查询有多少个socket 。
		case MSG_ID_APP_MTHL_QUERY_SOCK_NUM_REQ:
			mthl_query_sock_num_req_hdlr(&msg);
			break;


		//op为：关闭socket 。
		case MSG_ID_APP_MTHL_SOCK_CLOSE_REQ:
			mthl_close_req_hdlr(&msg);
			break;


		//通过socket，发送数据。
		case MSG_ID_APP_MTHL_SOCK_SEND_REQ:
			mthl_send_req_hdlr(&msg);
			break;


		//通过socket，接收数据。
		case MSG_ID_APP_MTHL_SOCK_RECV_REQ:
			mthl_recv_req_hdlr(&msg);
			break;



		//查询socket信息,收到来自local APP的数据，已ack数据，已经接收peer数据。
		case MSG_ID_APP_MTHL_QUERY_SOCK_INFO_REQ:
			mthl_query_info_req_hdlr(&msg);
			break;


		//通过域名获取IP 。
		case MSG_ID_APP_MTHL_GET_HOST_BY_NAME_REQ:
			mthl_get_host_by_name_req_hdlr(&msg);
			break;

		case MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND:
			assert(msg.wParam != NULL);
			mthl_soc_get_host_by_name_ind_hdlr(&msg);
			break;

		//通过IP获取域名。
		case MSG_ID_APP_MTHL_GET_HOST_BY_ADDR_REQ:
			mthl_get_host_by_addr_req_hdlr(&msg);
			break;

		case MSG_ID_APP_SOC_GET_HOST_BY_ADDR_IND:
			mthl_soc_get_host_by_addr_ind_hdlr(&msg);
			break;


		case MSG_ID_APP_MTHL_SERVER_LAUNCH_REQ:
			mthl_tcp_server_listen_and_accept_req_hdlr(&msg);
			break;

		case MSG_ID_APP_MTHL_TRANSPARENT_TRANS_REQ:
		{
			mthl_transparent_trans_req_struct *p_ilm;
			p_ilm = (mthl_transparent_trans_req_struct*)(msg.wParam);
			mthl_transparent_trans_req_hdlr(p_ilm);
			break;
		}

		case MSG_ID_UART_ESCAPE_DETECTED_IND:
		{
			uart_escape_detected_ind_struct *p_ilm;
			p_ilm = (uart_escape_detected_ind_struct*)(msg.wParam);
			mthl_transparent_uart_escape_detected_ind_hdlr(p_ilm);
			break;
		}

		case MSG_ID_UART_READY_TO_READ_IND:
		{
			uart_ready_to_read_ind_struct *p_ilm;
			p_ilm = (uart_ready_to_read_ind_struct*)(msg.wParam);
			mthl_transparent_uart_ready_to_read_ind_hdlr();
			break;
		}

		case MSG_ID_UART_READY_TO_WRITE_IND:
			mthl_transparent_ready_to_write_uart_ind_hdlr();
			break;

		case MSG_ID_UART_PLUGOUT_IND:
		{
			uart_plugout_ind_struct *p_local;
			p_local = (uart_plugout_ind_struct*)(msg.wParam);
			mthl_transparent_uart_plugout_ind_hdlr(p_local);
		}
		break;

		default:
			break;
		}
	}
}


INT8 mthl_cbm_release_bearer(UINT32 ori_acct_id)
{
  return CBM_ERROR;
}

void mthl_soc_close_recv_close_ind(INT8 socket_id)
{
	int i=0;
	int j = 0;
	INT8 error;
	mthl_close_sock_ind_struct mthl_close_ind;

	error = soc_close(socket_id);
	 if(error == SOC_INVALID_SOCKET)
		LogWriter::LOGX("[MTHL] mthl_soc_notify(): SOC_CLOSE evt, soc_close ret = SOC_INVALID_SOCKET.");
	 else if(error == SOC_SUCCESS)
	 {
		LogWriter::LOGX("[MTHL] mthl_soc_notify(): SOC_CLOSE evt, soc_close ret = SOC_SUCCESS.");
		//通过sock_id匹配，把close的socket id，包括statistic的部分，都清除为init value;
		for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
		{
			for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
			{
			   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== socket_id)
			   {
				 LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind() match sock_id.");

				 LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): clear ,g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);
				 g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;

				 LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): clear ,g_mthl_create_conn_soc_save[%d].socket_statistics_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id);
				 g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;
				 g_mthl_create_conn_soc_save[i].sock_num--;

				 LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): clear init value, SOC_ERROR, done.");

				 break;
			   }
			   else
				 continue;
			}
			if(j<MTHL_MAX_IP_SOCKET_NUM)
			  break;
			else
			  continue;
		}

		//清除失败；
		if(i == MTHL_GPRS_MAX_PDP_NUM)
		{
			LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): clear init value, fail, not match sock_id..");
			LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): ind_ptr->sock_id = %d.", socket_id);
		}
		//清除成功；
		else
		{
			LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): clear init value, success.");
			LogWriter::LOGX("[MTHL] mthl_soc_close_recv_close_ind(): ind_ptr->sock_id = %d.", socket_id);

			//mthl_close_cnf.result    = TRUE;
			//mthl_close_cnf.sock_id   = ind_ptr->socket_id;
			//mthl_close_cnf.user_data = g_mthl_create_conn_soc_save[i].user_data;
			mthl_close_ind.result	 = TRUE;
			mthl_close_ind.sock_id	 = socket_id;
			mthl_close_ind.user_data = g_mthl_create_conn_soc_save[i].user_data;

			//返回success cnf;
			mthl_close_sock_ind(&mthl_close_ind);
		}
	 }
	 else
		LogWriter::LOGX("[MTHL] send data fail: soc_close ret = %d.",error);

}
/*****************************************************************************
 * FUNCTION
 *  imps_soc_notify
 * DESCRIPTION
 *  This function is used to handle the notification for socket events.
 * PARAMETERS
 *  inMsg
 * RETURNS
 *  void
 *****************************************************************************/
void mthl_soc_notify(MSG *inMsg)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    app_soc_notify_ind_struct *ind_ptr;
    mthl_create_conn_cnf_struct    mthl_create_conn_cnf;
    mthl_send_data_ind_struct mthl_send_data_ind;
    mthl_recv_data_ind_struct mthl_recv_data_ind;
    mthl_server_accept_ind_struct    mthl_server_accept_ind;
    //mthl_close_sock_cnf_struct    mthl_close_cnf;

    int i = 0;
    int j = 0;
    INT8 conn_sock_id = SOC_ERROR;
    sockaddr_struct client_addr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    assert(inMsg != NULL);

    memset(&client_addr,0x00,sizeof(sockaddr_struct));
    memset(&mthl_create_conn_cnf, 0x00, sizeof(mthl_create_conn_cnf_struct));
    memset(&mthl_send_data_ind, 0x00, sizeof(mthl_send_data_ind_struct));
    memset(&mthl_recv_data_ind, 0x00, sizeof(mthl_recv_data_ind_struct));
    memset(&mthl_server_accept_ind, 0x00, sizeof(mthl_server_accept_ind_struct));

    ind_ptr = (app_soc_notify_ind_struct*)inMsg->wParam;


	if((mthl_get_transparent_mode_status()== FALSE)||(ind_ptr->socket_id != mthl_transparent_mode_context.sock_id))
		{

    switch (ind_ptr->event_type)
    {
        case SOC_READ:
        {
			//MTHL不用cover重收部分，只用报IND，后续User自己会决定重收；
            /*INT32 size=0;
            UINT8* cir_data = get_ctrl_buffer(256);
            memset((void*)cir_data,0,256);
            size=soc_recv(socketid,cir_data,256,0);
            WinUtil::L_FREE(cir_data);   */
            mthl_recv_data_ind.result = ind_ptr->result;
			//mthl_send_data_ind.ret_val = 0;
			mthl_recv_data_ind.sock_id = ind_ptr->socket_id;
			mthl_recv_data_ind.user_data = 0;

			//返回cnf;
			mthl_recv_sock_ind(&mthl_recv_data_ind);
            break ;
        }

        case SOC_WRITE:
			//MTHL不用cover重发部分，只用报IND，后续User自己会决定重发；
            //error=soc_send(imps_context_p->imps_cir_channel.cir_socket,(UINT8*) hello,len,0);

            mthl_send_data_ind.result = ind_ptr->result;
			//mthl_send_data_ind.ret_val = 0;
			mthl_send_data_ind.sock_id = ind_ptr->socket_id;
			mthl_send_data_ind.user_data = 0;

			//返回cnf;
			mthl_send_sock_ind(&mthl_send_data_ind);
            break ;

        case SOC_CLOSE:
			//由于bearer fail ind/，需要关闭对应的socket，释放sock res;
            mthl_soc_close_recv_close_ind(ind_ptr->socket_id);
            break;

        case SOC_CONNECT:
                //通过sock_id反过来匹配到account_id;
                for(i=0; i<MTHL_GPRS_MAX_PDP_NUM; i++)
                {
                    for(j=0; j<MTHL_MAX_IP_SOCKET_NUM; j++)
                    {
                       if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id== ind_ptr->socket_id)
					   	 break;
					   else
					   	 continue;
                    }
					if(j<MTHL_MAX_IP_SOCKET_NUM)
					  break;
					else
					  continue;
                }

			if (ind_ptr->result == FALSE)
            {
                //如果异步IND fail，则: clear 之前save的sock_id  info;
                g_mthl_create_conn_soc_save[i].socket_info[j].sock_id = SOC_ERROR;
				g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = SOC_ERROR;
				memset(&(g_mthl_create_conn_soc_save[i].socket_info[j].addr), 0x00, sizeof(sockaddr_struct));
                mthl_create_conn_cnf.account_id = g_mthl_create_conn_soc_save[i].account_id;
				mthl_create_conn_cnf.result     = FALSE;
				mthl_create_conn_cnf.sock_id    = SOC_ERROR;
				mthl_create_conn_cnf.user_data  = g_mthl_create_conn_soc_save[i].user_data;

				//异步返回fail cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);

				//error = soc_connect(...);尝试重连的feature;
				//close socket;
				soc_close(ind_ptr->socket_id);
				g_mthl_create_conn_soc_save[i].sock_num--;
            }
            else
            {


                mthl_create_conn_cnf.account_id = g_mthl_create_conn_soc_save[i].account_id;
				mthl_create_conn_cnf.result     = TRUE;
				mthl_create_conn_cnf.sock_id    = ind_ptr->socket_id;
				mthl_create_conn_cnf.user_data  = g_mthl_create_conn_soc_save[i].socket_info[j].user_data;

				//异步返回success cnf;
				mthl_send_create_conn_cnf(&mthl_create_conn_cnf);
            }
            break;

		 case SOC_ACCEPT:
		 	//MTHL不用cover重新accept部分，只用报IND，后续User自己会决定重新accept；
		 	if(ind_ptr->result == FALSE)
		 	{
		 	    mthl_server_accept_ind.conn_sock_id   = SOC_ERROR;
				//mthl_server_accept_ind.addr为初始值0；

				//返回cnf;
				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
		 	}
			else
			{
			   conn_sock_id = soc_accept(ind_ptr->socket_id, &client_addr);
			   mthl_server_accept_ind.conn_sock_id = conn_sock_id;
			    //mthl_server_accept_ind.addr为初始值0；
			        //先匹配account_id;
		            for(i =0 ;i< MTHL_GPRS_MAX_PDP_NUM;i++)
					{
					    if(g_server_info.server_account_id == g_mthl_create_pdp_param_save[i].account_id)
					    {
					        //ori_acct_id匹配成功;
							break;
					    }
						else
							continue;
					}

		            //ori_acct_id匹配失败；
					if(i == MTHL_GPRS_MAX_PDP_NUM)
					{
					   mthl_server_accept_ind.conn_sock_id = SOC_ERROR;
					   //memcpy(&(mthl_server_accept_ind.addr), &conn_addr, sizeof(sockaddr_struct));//保持初值0即可；

					   LogWriter::LOGX("[MTHL] soc_accept(): IND success ,but match account_id fail case.");

					   //IND之后，不用清0，server_account_id是固定的；
                       //g_server_info.server_account_id = CBM_INVALID_NWK_ACCT_ID;
						//返回cnf;
			  			mthl_tcp_server_accept_ind(&mthl_server_accept_ind);

					    //close socket;
						soc_close(conn_sock_id);
						return;
					}

			    //找到一个可用的socket_info[j]，再保存信息；
					for(j=0;j<MTHL_MAX_IP_SOCKET_NUM;j++)
					{
					   if(g_mthl_create_conn_soc_save[i].socket_info[j].sock_id == SOC_ERROR)
					   {
					        LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): find free sock_info, param:");

							g_mthl_create_conn_soc_save[i].socket_info[j].sock_id      = mthl_server_accept_ind.conn_sock_id;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].socket_info[%d].sock_id = %d", i,j,g_mthl_create_conn_soc_save[i].socket_info[j].sock_id);

							g_mthl_create_conn_soc_save[i].socket_info[j].addr.sock_type  = SOC_SOCK_STREAM;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): TCP case.");

				            //TCP 不用保存req带过来的要连接的Server地址；
							/*g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[0] = mthl_create_conn_req->addr.addr[0];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[1] = mthl_create_conn_req->addr.addr[1];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[2] = mthl_create_conn_req->addr.addr[2];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.addr[3] = mthl_create_conn_req->addr.addr[3];
							g_mthl_create_conn_soc_save[i].socket_info[j].addr.port    = mthl_create_conn_req->addr.port;*/

							//保存信息ok，再计数；
							g_mthl_create_conn_soc_save[i].sock_num++;
							LogWriter::LOGX("[MTHL] mthl_create_and_conn_req_hdlr(): g_mthl_create_conn_soc_save[%d].sock_num = %d",i,g_mthl_create_conn_soc_save[i].sock_num);

							//为流量统计再保存一份；
							g_mthl_create_conn_soc_save[i].socket_statistics_info[j].sock_id = mthl_server_accept_ind.conn_sock_id;
							break;
					   }
					   else
					   		continue;
					}
                    //接收连接成功，保存失败；
					if(j == MTHL_MAX_IP_SOCKET_NUM)
					{
							mthl_server_accept_ind.conn_sock_id = SOC_LIMIT_RESOURCE;
							//memcpy(&(mthl_server_accept_ind.addr), &conn_addr, sizeof(sockaddr_struct));//保持初值0即可；

							LogWriter::LOGX("[MTHL] soc_accept(): success ,but save conn_sock_id fail case.");

							//IND之后，不用清0，server_account_id是固定的；
                       		//g_server_info.server_account_id = CBM_INVALID_NWK_ACCT_ID;

							 //返回cnf;
			  				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
							//close socket;
						    soc_close(conn_sock_id);
							break;
					}
					//接收连接成功，保存成功；
					else
					{
					        memcpy(&(mthl_server_accept_ind.addr), &client_addr, sizeof(sockaddr_struct));
							LogWriter::LOGX("[MTHL] soc_accept(): success conn_sock_id case.");

							//IND之后，不用清0，server_account_id是固定的；
                       		//g_server_info.server_account_id = CBM_INVALID_NWK_ACCT_ID;

						    //返回cnf;
			  				mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
							break;
					}

				//返回cnf;
				//mthl_tcp_server_accept_ind(&mthl_server_accept_ind);
			}
			break;

        default:
			break;

    }

		}
	else
	{
		switch (ind_ptr->event_type)
		{
				case SOC_READ:
				{
					mthl_transparent_reveive_data();
					break ;
				}

				case SOC_WRITE:
					mthl_transparent_send_data();
					break ;

				case SOC_CLOSE:
				{
					//由于bearer fail ind/，需要关闭对应的socket，释放sock res;
					//error = soc_close(ind_ptr->socket_id);
					mthl_transparent_mem_dealloc();
					mthl_transparent_send_end_ind(mthl_transparent_mode_context.port,TRANSPARENT_NETWORK_ERROR);
					mthl_soc_close_recv_close_ind(ind_ptr->socket_id);
					break;
				}
				default:
					return;
			}

		}
	return;
}

void mthl_soc_get_host_by_name_ind_hdlr(MSG *msg)
{
	app_soc_get_host_by_name_ind_struct *p = (app_soc_get_host_by_name_ind_struct*)msg->wParam;
    mthl_get_host_by_name_cnf_struct    mthl_get_host_by_name_cnf;
    cbm_sim_id_enum sim_id;
    UINT8 app_id;
    BOOL always_ask;
    UINT32 ori_acct_id;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	memset(&mthl_get_host_by_name_cnf, 0x00, sizeof(mthl_get_host_by_name_cnf_struct));

    if (cbm_decode_data_account_id(p->account_id,
                                    &sim_id,
                                    &app_id,
                                    &always_ask,
                                    &ori_acct_id) == TRUE)
    {
        mthl_get_host_by_name_cnf.account_id = ori_acct_id;
    }


	//异步返回结果失败；
    if (p->result == FALSE)
    {
		mthl_get_host_by_name_cnf.result = FALSE;
		mthl_get_host_by_name_cnf.user_data = g_get_host_by_name_save[p->request_id].user_data;
		LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_name_ind_hdlr(): DNS query fail!");
		//返回cnf;
		mthl_get_host_by_name_soc_cnf(&mthl_get_host_by_name_cnf);

		//异步返回操作完成，清除save;
		g_get_host_by_name_save[p->request_id].bUsed = FALSE;

        return;
    }

        //异步返回结果success;
        mthl_get_host_by_name_cnf.result = TRUE;

		mthl_get_host_by_name_cnf.addr_len = p->addr_len;
		strncpy((char*)mthl_get_host_by_name_cnf.addr, (char*)p->addr, mthl_get_host_by_name_cnf.addr_len);
		LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_name_ind_hdlr(): DNS query from server, Success: IP addr:");
		LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_name_ind_hdlr(): mthl_get_host_by_name_cnf.addr = %d.%d.%d.%d",
			             mthl_get_host_by_name_cnf.addr[0], mthl_get_host_by_name_cnf.addr[1],mthl_get_host_by_name_cnf.addr[2],mthl_get_host_by_name_cnf.addr[3]);

		mthl_get_host_by_name_cnf.user_data = g_get_host_by_name_save[p->request_id].user_data;

		//返回cnf;
		mthl_get_host_by_name_soc_cnf(&mthl_get_host_by_name_cnf);

		//异步返回操作完成，清除save;
		g_get_host_by_name_save[p->request_id].bUsed = FALSE;

        return;

}

void mthl_soc_get_host_by_addr_ind_hdlr(MSG *msg)
{
    app_soc_get_host_by_addr_ind_struct* get_host_by_addr_ind = (app_soc_get_host_by_addr_ind_struct*)msg->wParam;
    mthl_get_host_by_addr_cnf_struct    mthl_get_host_by_addr_cnf;
    cbm_sim_id_enum sim_id;
    UINT8 app_id;
    BOOL always_ask;
    UINT32 ori_acct_id;


    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
	memset(&mthl_get_host_by_addr_cnf, 0x00, sizeof(mthl_get_host_by_addr_cnf_struct));

    if (cbm_decode_data_account_id(get_host_by_addr_ind->account_id,
                                    &sim_id,
                                    &app_id,
                                    &always_ask,
                                    &ori_acct_id) == TRUE)
    {
        mthl_get_host_by_addr_cnf.account_id = ori_acct_id;
    }

	//异步返回结果失败；
    if (get_host_by_addr_ind->result == FALSE)
    {
		mthl_get_host_by_addr_cnf.result = FALSE;
        mthl_get_host_by_addr_cnf.addr_len = g_get_host_by_addr_save.addr_len;
		memcpy(mthl_get_host_by_addr_cnf.addr, g_get_host_by_addr_save.addr, g_get_host_by_addr_save.addr_len);

		mthl_get_host_by_addr_cnf.user_data = g_get_host_by_addr_save.user_data;

		//返回cnf;
		mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);
        LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_addr_ind_hdlr(): IP Addr query fail.");

		//异步返回操作完成，清除save;
		memset(&g_get_host_by_addr_save, 0x00, sizeof(mthl_get_host_by_addr_save_struct));

        return;
    }

        //异步返回结果success;
        mthl_get_host_by_addr_cnf.result = TRUE;

       mthl_get_host_by_addr_cnf.addr_len = g_get_host_by_addr_save.addr_len;
		memcpy(mthl_get_host_by_addr_cnf.addr, g_get_host_by_addr_save.addr, g_get_host_by_addr_save.addr_len);

		mthl_get_host_by_addr_cnf.domain_len= strlen(get_host_by_addr_ind->name);
		strncpy(mthl_get_host_by_addr_cnf.domain_name, get_host_by_addr_ind->name, mthl_get_host_by_addr_cnf.domain_len);
		LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_addr_ind_hdlr(): IP Addr query from server, Success: domain_name:");
		LogWriter::LOGX("[MTHL] mthl_soc_get_host_by_addr_ind_hdlr(): domain_name = %s", mthl_get_host_by_addr_cnf.domain_name);

		mthl_get_host_by_addr_cnf.user_data = g_get_host_by_addr_save.user_data;

		//返回cnf;
		mthl_get_host_by_addr_soc_cnf(&mthl_get_host_by_addr_cnf);

		//异步返回操作完成，清除save;
		memset(&g_get_host_by_addr_save, 0x00, sizeof(mthl_get_host_by_addr_save_struct));

        return;
}


