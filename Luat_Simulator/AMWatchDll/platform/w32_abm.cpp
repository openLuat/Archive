/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    w32_abm.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          This file is intends for Socket APIs on WIN32 environment.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "platform_rtos.h"
#include "w32_abm.h"
#include "w32_socket.h"

#define _W32_ABM_C_

#define ABM_MAX_APP_NWK_NUM  50

typedef enum
{
    W32_ABM_NON_AUTO_DEACT = 0x01,
    W32_ABM_ASK_NEED_NOTIFY = 0x02,
    W32_ABM_ASK_NEED_RESULT = 0x04
} bearer_option_enum;

typedef enum
{
    W32_BEARER_CLOSED         = 0,
    W32_BEARER_OPENING        = 1,
    W32_BEARER_OPENED         = 2
} bearer_state_enum;

typedef struct
{
    int app_id;
    int account_id;
    bearer_option_enum option;
    bearer_state_enum  state;
} abm_app_nwk_state_struct;

static abm_app_nwk_state_struct abm_app_nwk[ABM_MAX_APP_NWK_NUM];
static HANDLE w32_abm_lock;
#define ABM_LOCK          WaitForSingleObject(w32_abm_lock, INFINITE)
#define ABM_UNLOCK        ReleaseSemaphore(w32_abm_lock, 1, NULL)

void abm_init_win32(void)
{
    int i;

    for (i = 0 ; i < ABM_MAX_APP_NWK_NUM ; i++)
    {
        abm_app_nwk[i].app_id         = -1;
    
        abm_app_nwk[i].account_id     = -1;
        abm_app_nwk[i].option         = (bearer_option_enum)0;
        abm_app_nwk[i].state          = W32_BEARER_CLOSED;
    }
    
     w32_abm_lock = CreateSemaphore(NULL, 1, 1, NULL);
    return;
}

static
int abm_find_app_slot(int app_id,
                      int nwk_account_id, 
                      int *idx)
{
    int i;
    assert(idx);
    
    for (i = 0 ; i < ABM_MAX_APP_NWK_NUM ; i++)
    {
        if (((app_id != 0 && abm_app_nwk[i].app_id == app_id) ||
             (abm_app_nwk[i].app_id == app_id)) &&
             (nwk_account_id == -1 ||
              abm_app_nwk[i].account_id == nwk_account_id))
        {
            *idx = i;
            return 1;
        }
    }
    return 0;
}

static
int abm_find_free_app_slot(int *idx)
{
    int found;

    assert(idx);
    found = abm_find_app_slot(-1,
                              -1, 
                              idx);
    assert(found == 1);

    return found;
}

static
void abm_free_soc_app_slot(int idx)
{
    assert(idx >= 0 && idx < ABM_MAX_APP_NWK_NUM);

    abm_app_nwk[idx].app_id = -1;
    abm_app_nwk[idx].account_id = -1;
    abm_app_nwk[idx].option = (bearer_option_enum)(abm_app_nwk[idx].option & ~W32_ABM_NON_AUTO_DEACT);
    abm_app_nwk[idx].state = W32_BEARER_CLOSED;
    
    return;
}

INT32 abm_get_bearer_app_result(UINT32 type, UINT32 state)
{
    return CBM_OK;
}

INT32 abm_get_bearer_app_value(UINT32 type, UINT32 state)
{
    return CBM_OK;
}


void mmi_abm_bearer_connect_req_hdlr(mmi_abm_bearer_event_connect_req_struct *info)
{
    UINT8 ap_id;
    int id;
    int ori_account_id;
    abm_app_id_struct app;
    UINT8 ip_addr[4];
    abm_app_proc_struct *app_proc;
    cbm_account_info_struct acct_info;
    mmi_abm_bearer_event_connect_cnf_struct *cnf = NULL;
	MSG msg;

    /* send back response */
	cnf = (mmi_abm_bearer_event_connect_cnf_struct*)WinUtil::L_MALLOC(sizeof(mmi_abm_bearer_event_connect_cnf_struct));
    cnf->event = info->event;
    cnf->ori_acct_id = info->ori_acct_id;
    cnf->result = TRUE;

	msg.message = MSG_ID_MMI_ABM_BEARER_EVENT_CONNECT_CNF;
	msg.wParam = (WPARAM)cnf;
	msg.lParam = -1;

	SendToLuaSocketMessage(&msg);
                    
    ap_id = cbm_get_app_id(info->ori_acct_id);
    cbm_decode_data_account_id_ext(info->ori_acct_id, &acct_info);
    if (acct_info.acct_num > 1)
        ori_account_id = acct_info.account[1].account_id;
    else
        ori_account_id = acct_info.account[0].account_id;
    
    if (abm_find_app_slot(ap_id,
                          info->ori_acct_id, 
                          &id) )
    {
        app_proc = abm_find_app_proc(ap_id);
        if (app_proc)
        {
            memcpy(app.report_mod_id, app_proc->app.report_mod_id, sizeof(app.report_mod_id));
        }
        app.ap_id = ap_id;
        
        if (info->user_result == MMI_ABM_BEARER_EVENT_ACCEPT_THIS_TIME)
        {
            if (abm_app_nwk[id].state == W32_BEARER_OPENING)
            {
                ip_addr[0] = 192;
                ip_addr[1] =168;
                ip_addr[2] = 1;
                ip_addr[3] = 1;
                abm_send_bearer_info_ind2app(&app, abm_app_nwk[id].account_id, 
                                            ori_account_id, 0, 
                                            CBM_ACTIVATING, 
                                            CBM_PS, 
                                            ip_addr, 0, (cbm_result_error_enum)0, NULL);
                abm_send_bearer_info_ind2mmi(0, CBM_PS, ori_account_id, CBM_ACTIVATED);
                abm_send_bearer_info_ind2app(&app, abm_app_nwk[id].account_id, 
                                            ori_account_id, 0, 
                                            CBM_ACTIVATED, 
                                            CBM_PS, 
                                            ip_addr, 0, (cbm_result_error_enum)0, NULL);
                abm_app_nwk[id].state = W32_BEARER_OPENED;
            }

            abm_send_act_cnf2soc(CBM_PS,
                                 (UINT32)abm_app_nwk[id].account_id, /* nw_acc_id */
                                 (UINT8)ori_account_id,
                                 0,               /* qos_profile_id */
                                 0,           /* bearer_ctx_id */
                                 0,     /* qos_fulfilled */
                                 ip_addr,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 1500);      /* beaer MTU=0 */
        }
        else
        {
            if (abm_app_nwk[id].state != W32_BEARER_CLOSED)
            {
                abm_send_bearer_info_ind2app(&app, (UINT32)abm_app_nwk[id].account_id, 
                                            ori_account_id, 0, 
                                            CBM_DEACTIVATING, 
                                            CBM_PS, NULL, 0, (cbm_result_error_enum)0, NULL);
                abm_send_bearer_info_ind2mmi(0, CBM_PS, ori_account_id, CBM_DEACTIVATED);
                abm_send_bearer_info_ind2app(&app, (UINT32)abm_app_nwk[id].account_id, 
                                            ori_account_id, 0, 
                                            CBM_DEACTIVATED, 
                                            CBM_PS, NULL, 0, (cbm_result_error_enum)0, NULL);
                    
            }
            abm_send_rej2soc(MSG_ID_SOC_ABM_ACTIVATE_CNF,
                             CBM_PS, 0, 0, abm_app_nwk[id].account_id, 
                             0, 0, FALSE);
            abm_free_soc_app_slot(id);
        }
    }
        
}


void mmi_abm_bearer_event_notify_rsp_hdlr(app_cbm_bearer_event_notify_rsp_struct * info)
{
    UINT8 ap_id;
    int id;
    int ori_account_id;
    abm_app_id_struct app;
    abm_app_proc_struct *app_proc;
    
    ap_id = cbm_get_app_id(info->account_id);
    ori_account_id = cbm_get_original_account(info->account_id);
       
    app.ap_id = ap_id;    
    
    if (abm_find_app_slot(ap_id,
                          info->account_id, 
                          &id) == 0)
    {
        app_proc = abm_find_app_proc(ap_id);
        if (app_proc)
        {
            memcpy(app.report_mod_id, app_proc->app.report_mod_id, sizeof(app.report_mod_id));
        }
        /* wait for connect request */       
    }
}

void abm_soc_activate_req_hdlr(soc_abm_activate_req_struct* info)
{
    UINT8 ap_id;
    UINT32 id;
    UINT32 ori_account_id;
    abm_app_id_struct app;
    abm_app_proc_struct *app_proc;
    UINT8 ip_addr[4];
    cbm_sim_id_enum sim_id;
    cbm_account_info_struct acct_info;

    ABM_LOCK;
    cbm_decode_data_account_id_ext(info->nw_acc_id, &acct_info);
    ap_id = acct_info.app_id;
    ori_account_id = acct_info.account[0].account_id;
    sim_id = acct_info.account[0].sim_id;
    app.ap_id = ap_id;    
    
    if (abm_find_app_slot(ap_id,
                          info->nw_acc_id, 
                          (INT*)&id) == 0)
    {
        if (abm_find_app_slot(ap_id,
                          -1, 
                          (INT*)&id))
        {
            if (abm_app_nwk[id].account_id == -1)
                abm_app_nwk[id].account_id = info->nw_acc_id;
            
            abm_app_nwk[id].state = W32_BEARER_CLOSED;
        }
        else
        {
            if (abm_find_free_app_slot((INT*)&id))
            {
                abm_app_nwk[id].app_id = ap_id;
                
                if (abm_app_nwk[id].account_id == -1)
                    abm_app_nwk[id].account_id = info->nw_acc_id;
                
                abm_app_nwk[id].state = W32_BEARER_CLOSED;
            }
        }
    }

    app_proc = abm_find_app_proc(ap_id);
    if (app_proc)
    {
        memcpy(app.report_mod_id, app_proc->app.report_mod_id, sizeof(app.report_mod_id));
    }

    if (abm_app_nwk[id].state == W32_BEARER_CLOSED)
    {
        ip_addr[0] = 192;
        ip_addr[1] =168;
        ip_addr[2] = 1;
        ip_addr[3] = 1;
        ori_account_id = cbm_encode_data_account_id(ori_account_id, sim_id, 
        0, FALSE);
        abm_send_bearer_info_ind2app(&app, abm_app_nwk[id].account_id, 
                                    ori_account_id, info->qos_profile_id, 
                                    CBM_ACTIVATING, 
                                    CBM_PS, 
                                    ip_addr, 0, (cbm_result_error_enum)0, NULL);
        abm_send_bearer_info_ind2mmi(0, CBM_PS, ori_account_id, CBM_ACTIVATED);
        abm_send_bearer_info_ind2app(&app, abm_app_nwk[id].account_id, 
                                    ori_account_id, info->qos_profile_id, 
                                    CBM_ACTIVATED, 
                                    CBM_PS, 
                                    ip_addr, 0, (cbm_result_error_enum)0, NULL);
        abm_app_nwk[id].state = W32_BEARER_OPENED;
    }

    abm_send_act_cnf2soc(CBM_PS,
                         (UINT32)abm_app_nwk[id].account_id, /* nw_acc_id */
                         (UINT8)ori_account_id,
                         info->qos_profile_id,               /* qos_profile_id */
                         0,           /* bearer_ctx_id */
                         0,     /* qos_fulfilled */
                         ip_addr,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         0,
                         1500);      /* beaer MTU=0 */

    ABM_UNLOCK;
    return;
}

void abm_soc_deactivate_req_hdlr(soc_abm_deactivate_req_struct* info)
{
    UINT8 ap_id;
    int id;
    cbm_sim_id_enum sim_id;
    UINT32 ori_account_id;
    abm_app_id_struct app;
    abm_app_proc_struct *app_proc;
    
    ap_id = cbm_get_app_id(info->nw_acc_id);
    ori_account_id = cbm_get_original_account(info->nw_acc_id);
    sim_id = cbm_get_sim_id(info->nw_acc_id);
    app.ap_id = ap_id;  
    
    ABM_LOCK;
    if (abm_find_app_slot(ap_id,
                          info->nw_acc_id, 
                          &id))
    {
        app_proc = abm_find_app_proc(ap_id);
        if (app_proc)
        {
            memcpy(app.report_mod_id, app_proc->app.report_mod_id, sizeof(app.report_mod_id));
        }
    
        if ((info->cause & ABM_APP_NON_AUTO_DEACT) != ABM_APP_NON_AUTO_DEACT)
        {
            if (abm_app_nwk[id].option & W32_ABM_NON_AUTO_DEACT)
            {
                abm_send_rej2soc(MSG_ID_SOC_ABM_DEACTIVATE_CNF,
                        (cbm_bearer_enum)ABM_E_SOCKET_BEARER_IP_ANY,
                        ABM_E_NON_AUTO_DEACT,
                        0,
                        info->nw_acc_id,
                        ABM_DEFAULT_QOS,
                        ABM_NULL_BEARER,
                        FALSE);
                ABM_UNLOCK;
                return;
            }
        }
        if (abm_app_nwk[id].state != W32_BEARER_CLOSED)
        {
            ori_account_id = cbm_encode_data_account_id(ori_account_id, sim_id, 
            0, 0);
            abm_send_bearer_info_ind2app(&app, info->nw_acc_id, 
                                        ori_account_id, info->qos_profile_id, 
                                        CBM_DEACTIVATING, 
                                        CBM_PS, NULL, 0, (cbm_result_error_enum)0, NULL);
            abm_send_bearer_info_ind2mmi(0, CBM_PS, ori_account_id, CBM_DEACTIVATED);
            abm_send_bearer_info_ind2app(&app, info->nw_acc_id, 
                                        ori_account_id, info->qos_profile_id, 
                                        CBM_DEACTIVATED, 
                                        CBM_PS, NULL, 0, (cbm_result_error_enum)0, NULL);
                
        }
        abm_free_soc_app_slot(id);
    }
       
    abm_send_deact_cnf2soc(info->nw_acc_id, 
                           info->qos_profile_id, 0);
    ABM_UNLOCK;
    return;
}

INT32 abm_get_bearer_mmi_value(UINT32 type, UINT32 state)
{
    return CBM_OK;
}

#ifndef __NWK_APP_UT__

INT8 cbm_hold_bearer(UINT8 app_id)
{
    int i;
    int free_slot = -1;

    ABM_LOCK;
    
    for (i = 0; i < ABM_MAX_APP_NWK_NUM; i++)
    {
        if ((app_id != 0 && abm_app_nwk[i].app_id == app_id))
        {
            abm_app_nwk[i].option = W32_ABM_NON_AUTO_DEACT;
            break;
        }
        if (free_slot == -1)
        {
            if (abm_app_nwk[i].app_id == -1)
                free_slot = i;
        }
    }
    if (i >= ABM_MAX_APP_NWK_NUM && free_slot != -1)
    {
        abm_app_nwk[free_slot].app_id = app_id;
        abm_app_nwk[free_slot].option = W32_ABM_NON_AUTO_DEACT;
    }

    ABM_UNLOCK;
    return CBM_OK;
}

INT8 cbm_release_bearer(UINT8 app_id)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    app_soc_deactivate_req_struct *ind_ptr;
    MSG msg;
    UINT8 i;
    abm_app_proc_struct *app_proc = NULL;
    UINT32 nwk_acct_id = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    ABM_LOCK;
    for (i = 0 ; i < ABM_MAX_APP_NWK_NUM ; i++)
    {
        if ((app_id != 0 && abm_app_nwk[i].app_id == app_id))
        {
            if (abm_app_nwk[i].state != W32_BEARER_CLOSED)
                break;
        }
    }
    if (i >= ABM_MAX_APP_NWK_NUM)
    {
        ABM_UNLOCK;
        return CBM_ERROR;
    }
    
    nwk_acct_id = cbm_encode_data_account_id(CBM_ALL_NWK_ACCT_ID, CBM_SIM_ID_SIM1, app_id, FALSE);

    ind_ptr = (app_soc_deactivate_req_struct *) 
		WinUtil::L_MALLOC(sizeof(app_soc_deactivate_req_struct));
    ind_ptr->account_id = nwk_acct_id;
    ind_ptr->qos_profile_id = CBM_ALL_QOS_ID;
   
    msg.message = MSG_ID_APP_SOC_DEACTIVATE_REQ;
    msg.wParam = (WPARAM)ind_ptr;
    msg.lParam = -1;
    SendToLuaSocketMessage(&msg);

    ABM_UNLOCK;
    return CBM_WOULDBLOCK; 
}

INT8 cbm_get_bearer_type(UINT32 account_id,
                             cbm_bearer_enum *bearer_type)
{
    UINT32 ori_account_id;

    ori_account_id = cbm_get_original_account(account_id);
   
    if (abm_get_bearer_type(ori_account_id, 
        (cbm_bearer_enum *)bearer_type) != CBM_OK)
    {
        *bearer_type = CBM_BEARER_NONE;
    }
    
    return CBM_OK;
}

INT32 cbm_get_bearer_status(UINT32 account_id)
{

    INT32 id = 0;
    INT32 ret = 0;
    
    ABM_LOCK;
    
    
    if (abm_find_app_slot(cbm_get_app_id(account_id),
                          account_id, 
                          &id))
    {
        if (abm_app_nwk[id].state == W32_BEARER_OPENED)
        {
            ret = CBM_ACTIVATED;
        }
        else
        {
            ret = CBM_DEACTIVATED;
        }
    }
    else
    {
        ret = CBM_DEACTIVATED;
    }
    
    ABM_UNLOCK;
    return ret;
}

#endif /* __NWK_APP_UT__ */

INT32 abm_get_bearer_message_result(UINT32 type, UINT32 state)
{
    return CBM_OK;
}

INT32 abm_get_bearer_message_value(UINT32 type, UINT32 state)
{
    return CBM_OK;
}

