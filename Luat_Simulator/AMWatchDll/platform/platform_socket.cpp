/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_socket.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          TCPIP SOCKET.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "platform.h"
#include "platform_rtos.h"
#include "platform_socket.h"

HANDLE hSocketThread = NULL;
DWORD dwSocketThreadId = NULL;

lua_socket_context_struct lua_socket_context;

void SendToLuaSocketMessage(const MSG *msg)
{
	if(!startRtosSimulator)
	{
		LogWriter::LOGX("[SendToLuaSocketMessage]: lost msg(%d)", msg->message);
		return;
	}

	LogWriter::LOGX("[platform_socket] SendToLuaSocketMessage message=%d", msg->message);

	PostThreadMessage(dwSocketThreadId, msg->message, msg->wParam, msg->lParam);
}

void platform_lua_socket_init(void)
{
	unsigned i = 0;
	CycleQueue* queue;

	for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
	{
		queue = &lua_socket_context.socket_info[i].recv_queue;
		queue->buf = lua_socket_context.socket_info[i].recv_buf;
		queue->size = sizeof(lua_socket_context.socket_info[i].recv_buf);
		QueueClean(queue);

		lua_socket_context.socket_info[i].sock_id = LUA_INVALID_SOKCET_ID;
	}

	hSocketThread = CreateThread(NULL, 
		LUA_SOCKET_THREAD_STACK_SIZE,
		(LPTHREAD_START_ROUTINE)mthl_main_thread,
		NULL,
		CREATE_SUSPENDED,
		&dwSocketThreadId);
	SetThreadPriority(hSocketThread,THREAD_PRIORITY_HIGHEST);
	ResumeThread(hSocketThread);

	soc_init_win32();

	LogWriter::LOGX("[platform_socket] platform_lua_socket_init.");
}

BOOL platform_is_domain_name(CHAR* pData)
{    
    if ((*pData < '0') || (*pData > '9'))
    {
        return TRUE;
    }
    
    while(*pData)
    {
        if ((*pData != '.') && ((*pData < '0') || (*pData > '9')))
        {
            return TRUE;
        } 

        pData++;
    }
    
    return FALSE;
}

UINT32 platform_lua_socket_id_to_index(UINT32 sock_id)
{
	unsigned i =0;

	for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
	{
		if(lua_socket_context.socket_info[i].sock_id == sock_id)
		{
			return i;
		}
	}

	return LUA_INVALID_SOCKET_INDEX;
}


BOOL platform_get_ip_by_name(UINT8 socket_id, char* url)
{
    mthl_get_host_by_name_req_struct* get_host_by_name_req;
    MSG msg;

    get_host_by_name_req = (mthl_get_host_by_name_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_get_host_by_name_req_struct));
    memset(get_host_by_name_req->domain_name, 0x00, sizeof(get_host_by_name_req->domain_name));
    memcpy(get_host_by_name_req->domain_name,
        url,
        strlen(url));
    
    get_host_by_name_req->domain_len = strlen(url);
    get_host_by_name_req->user_data  = socket_id;
    
    msg.message = MSG_ID_APP_MTHL_GET_HOST_BY_NAME_REQ;
    msg.wParam = (WPARAM)get_host_by_name_req;
    msg.lParam = -1;

    SendToLuaSocketMessage(&msg);
    
    return TRUE;
}
BOOL platform_activate_pdp(CHAR* apn, CHAR* user_name, CHAR* password)
{
	mthl_create_pdp_param_req_struct* create_pdp_param_req = NULL;
	unsigned apn_length = strlen(apn);
	MSG msg;

	create_pdp_param_req = (mthl_create_pdp_param_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_create_pdp_param_req_struct));
	if(create_pdp_param_req == NULL)
	{
		LogWriter::LOGX("[platform_socket] platform_activate_pdp create_pdp_param_req=NULL.");
		return FALSE;
	}
	strcpy((char*)create_pdp_param_req->apn, apn);
	create_pdp_param_req->apn_length = apn_length;
	create_pdp_param_req->pdp_type   = IPV4_ADDR_TYPE;
	create_pdp_param_req->account_id = LUA_SOCKET_ACCT_ID;

	if(strlen(apn) > 0) strcpy_s(lua_socket_context.apn, MAX_APN_LEN, apn);
	if(strlen(user_name) > 0) strcpy_s(lua_socket_context.user_name, CUSTOM_DTCNT_PROF_MAX_USER_LEN, user_name);
	if(strlen(password) > 0) strcpy_s(lua_socket_context.password, CUSTOM_DTCNT_PROF_MAX_PW_LEN, password);

	msg.message = MSG_ID_APP_MTHL_CREATE_PDP_PARAM_REQ;
	msg.wParam = (WPARAM)create_pdp_param_req;
	msg.lParam = -1;

	SendToLuaSocketMessage(&msg);

    return TRUE;
}

BOOL platform_deactivate_pdp(void)
{
    MSG msg;
    mthl_deactivate_pdp_req_struct* deactivate_pdp_req;
    
    deactivate_pdp_req = (mthl_deactivate_pdp_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_deactivate_pdp_req_struct));

    deactivate_pdp_req->account_id = LUA_SOCKET_ACCT_ID;
    
	msg.message = MSG_ID_APP_MTHL_DEACTIVATE_PDP_REQ;
	msg.wParam = (WPARAM)deactivate_pdp_req;
	msg.lParam = -1;

	SendToLuaSocketMessage(&msg);

    return TRUE;
}

BOOL platform_socket_send(UINT8 socket_index, CHAR* data, UINT16 length)
{
	MSG msg;
	mthl_send_data_req_struct*     send_data_req;
	lua_socket_info_struct*        lua_socket_info;
	char*  buf  = (char*)WinUtil::L_MALLOC(length);

	lua_socket_info = &lua_socket_context.socket_info[socket_index];
	send_data_req   = (mthl_send_data_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_send_data_req_struct));

	memcpy(buf, data, length);

	send_data_req->sock_id   = lua_socket_info->sock_id;
	send_data_req->size      = length;
	send_data_req->data      = (UINT8*)buf;
	send_data_req->flags     = 0;
	send_data_req->user_data = socket_index;

	if(lua_socket_info->sock_type == SOC_SOCK_DGRAM)
	{
		send_data_req->addr.addr_len = 4;
		memcpy(send_data_req->addr.addr, lua_socket_info->addr, send_data_req->addr.addr_len);

		send_data_req->addr.port = lua_socket_info->port;
		send_data_req->addr.sock_type = SOC_SOCK_DGRAM;
	}

	msg.message = MSG_ID_APP_MTHL_SOCK_SEND_REQ;
	msg.wParam = (WPARAM)send_data_req;
	msg.lParam = send_data_req->user_data;
	SendToLuaSocketMessage(&msg);

	return TRUE;
}

UINT32 platform_socket_recv(UINT8 socket_index, UINT8* data, UINT16 length)
{
    assert(data != NULL && length > 0);

    return QueueDelete(&lua_socket_context.socket_info[socket_index].recv_queue,
                data, 
                length);
}
 
void platform_socket_on_recv_done(UINT8 socket_index)
{
    mthl_recv_data_ind_struct *recv_data_ind;
    void * message_body;
    MSG msg;

    recv_data_ind   = (mthl_recv_data_ind_struct*)WinUtil::L_MALLOC(sizeof(mthl_recv_data_ind_struct));
    message_body = (void*)&recv_data_ind->sock_id;
    recv_data_ind->result = TRUE;
    recv_data_ind->sock_id = lua_socket_context.socket_info[socket_index].sock_id;
    recv_data_ind->user_data = sizeof(mthl_recv_data_ind_struct) + (PUINT32)message_body - (PUINT32)recv_data_ind;

    msg.message = MSG_ID_APP_MTHL_SOCK_RECV_IND;
    msg.wParam = (WPARAM)recv_data_ind;
    msg.lParam = recv_data_ind->user_data;
    SendToLuaSocketMessage(&msg);
}

BOOL platform_socket_close(UINT8 socket_index,
	BOOL graceful_close)
{
    mthl_close_sock_req_struct* close_sock_req;
    MSG msg;
    UINT32 sock_id;

    soc_linger_struct linger;
    
    if(graceful_close)
    {
        linger.onoff = TRUE;
        linger.linger_time = 0;
        
    }
    else
    {
        linger.onoff = TRUE;
        linger.linger_time = 1;    
    }

    sock_id  = lua_socket_context.socket_info[socket_index].sock_id;

    if(sock_id == LUA_INVALID_SOKCET_ID)
    {
        mthl_close_sock_cnf_struct* close_sock_cnf;
        
        close_sock_cnf   = (mthl_close_sock_cnf_struct*)WinUtil::L_MALLOC(sizeof(mthl_close_sock_cnf_struct));

        close_sock_cnf->sock_id = sock_id;
        close_sock_cnf->result  =  TRUE;
        close_sock_cnf->user_data = socket_index;
        
        msg.message = MSG_ID_APP_MTHL_SOCK_CLOSE_CNF;
        msg.wParam = (WPARAM)close_sock_cnf;
        msg.lParam = close_sock_cnf->user_data;
        SendToLuaSocketMessage(&msg);
        
        return FALSE;
    }
    else
    {
        soc_setsockopt(sock_id, SOC_LINGER, (void*)&linger, sizeof(linger));

        close_sock_req   = (mthl_close_sock_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_close_sock_req_struct));

        close_sock_req->sock_id = sock_id;
        close_sock_req->user_data = socket_index;
        
        msg.message = MSG_ID_APP_MTHL_SOCK_CLOSE_REQ;
        msg.wParam = (WPARAM)close_sock_req;
        msg.lParam = close_sock_req->user_data;
        SendToLuaSocketMessage(&msg);
    }

    return TRUE;    
}

BOOL platform_socket_conn_IP(UINT8 socket_index, sockaddr_struct* sockaddr)
{
	MSG msg;

	mthl_create_conn_req_struct* create_conn_req;
	create_conn_req = (mthl_create_conn_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_create_conn_req_struct));
	create_conn_req->account_id         = LUA_SOCKET_ACCT_ID;
	create_conn_req->user_data          = socket_index;
	create_conn_req->addr               = *sockaddr;

	msg.message = MSG_ID_APP_MTHL_CREATE_CONN_REQ;
	msg.wParam = (WPARAM)create_conn_req;
	msg.lParam = socket_index;

	SendToLuaSocketMessage(&msg);

	return TRUE;
}

BOOL platform_socket_conn(UINT8 socket_index,
	socket_type_enum sock_type,
	UINT16 port,	CHAR* addr)
{
	lua_socket_info_struct* lua_socket_info = &lua_socket_context.socket_info[socket_index];

	memcpy(lua_socket_info->addr,
		addr,
		MAX_SOCK_ADDR_LEN);

	lua_socket_info->port = port;
	lua_socket_info->sock_type = sock_type;

	if(platform_is_domain_name(addr))
	{
		return platform_get_ip_by_name(socket_index, addr);
	}
	else
	{
		UINT32 ip[4];
		sockaddr_struct sockaddr;

		sscanf(addr,"%d.%d.%d.%d", 
			&ip[0],
			&ip[1],
			&ip[2],
			&ip[3]);

		sockaddr.addr[0] = ip[0];
		sockaddr.addr[1] = ip[1];
		sockaddr.addr[2] = ip[2];
		sockaddr.addr[3] = ip[3];

		sockaddr.addr_len  = 4;
		sockaddr.port      = port;
		sockaddr.sock_type = sock_type;

		memcpy(lua_socket_info->addr, sockaddr.addr, 4);

		return platform_socket_conn_IP(socket_index, &sockaddr);
	}
}

INT32 platform_on_get_host_by_name_cnf(lua_State *L, PlatformMessage* pMsg)
{
	mthl_get_host_by_name_cnf_struct* get_host_by_name_cnf;

	get_host_by_name_cnf = (mthl_get_host_by_name_cnf_struct*)pMsg->local_para_ptr;

    if(get_host_by_name_cnf->result != TRUE)
    {
        lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_CONNECT_CNF);
        lua_pushinteger(L, get_host_by_name_cnf->user_data);  
        lua_pushinteger(L, get_host_by_name_cnf->result);  
        return 3;
    }
    else
    {
        sockaddr_struct sockaddr;
        UINT32 socket_id   = get_host_by_name_cnf->user_data;
        lua_socket_info_struct* socket_info = &lua_socket_context.socket_info[socket_id];

        memcpy(socket_info->addr,
               get_host_by_name_cnf->addr,
               MAX_SOCK_ADDR_LEN);

        socket_info->addr[get_host_by_name_cnf->addr_len] = 0;
        
        memcpy(sockaddr.addr, socket_info->addr, MAX_SOCK_ADDR_LEN);
        sockaddr.addr_len  = get_host_by_name_cnf->addr_len;
        sockaddr.port      = socket_info->port;
        sockaddr.sock_type = socket_info->sock_type;
        
        platform_socket_conn_IP(socket_id, &sockaddr);  

        return 0;
    }
}

INT32 platform_on_create_conn_cnf(lua_State *L, PlatformMessage* pMsg)
{
	mthl_create_conn_cnf_struct* create_conn_cnf = NULL;
	UINT32 socket_index;

	create_conn_cnf = (mthl_create_conn_cnf_struct*)pMsg->local_para_ptr;
	socket_index = create_conn_cnf->user_data;

	lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_CONNECT_CNF);
	lua_pushinteger(L, socket_index);
	lua_pushinteger(L, create_conn_cnf->result);  

	lua_socket_context.socket_info[socket_index].sock_id = create_conn_cnf->sock_id;

#ifdef LUA_SOCKET_TEST
	if(create_conn_cnf->result == TRUE)
	{
		char* buf = (char*)WinUtil::L_MALLOC(5);

		Sleep(1000);

		memcpy(buf, "hello",5);
		platform_socket_send(0, buf, 5);
	}
#endif

	return 3;
}

INT32 platform_on_create_sock_ind(lua_State *L, PlatformMessage* pMsg)
{
	mthl_create_sock_ind_struct* create_sock_ind = NULL;
	UINT32 socket_index;

	create_sock_ind = (mthl_create_sock_ind_struct*)pMsg->local_para_ptr;
	socket_index = create_sock_ind->user_data;
	lua_socket_context.socket_info[socket_index].sock_id = create_sock_ind->sock_id;
	return 0;
}

INT32 platform_on_create_pdp_param_cnf(lua_State *L, PlatformMessage* pMSG)
{
	unsigned index;
	mthl_create_pdp_param_cnf_struct *create_pdp_param_cnf;

    create_pdp_param_cnf = (mthl_create_pdp_param_cnf_struct*)pMSG->local_para_ptr;
	index = create_pdp_param_cnf->user_data;
	if(index == LUA_INVALID_SOCKET_INDEX)
	{
		create_pdp_param_cnf->result = PARAM_INVALID;
		LogWriter::LOGX("[platform_socket] platform_on_create_pdp_param_cnf PARAM_INVALID");
	}

	if(create_pdp_param_cnf->result != SUCCESS)
	{
		lua_pushinteger(L, MSG_ID_TCPIP_PDP_ACTIVATE_CNF);
		lua_pushinteger(L, FALSE);
		lua_pushinteger(L, LUA_INVALID_PARAM); 

		return 3;
	}
	else
	{
		MSG msg;
		mthl_activate_pdp_req_struct*     active_pdp_param_req;

		active_pdp_param_req = (mthl_activate_pdp_req_struct*)WinUtil::L_MALLOC(sizeof(mthl_activate_pdp_req_struct));

		strcpy((char*)active_pdp_param_req->Auth_info.Passwd,   lua_socket_context.password);
		strcpy((char*)active_pdp_param_req->Auth_info.UserName, lua_socket_context.user_name);

		active_pdp_param_req->Auth_info.AuthType = CUSTOM_DTCNT_PROF_GPRS_AUTH_TYPE_NORMAL;
		active_pdp_param_req->account_id         = LUA_SOCKET_ACCT_ID;
		active_pdp_param_req->user_data          = 0;

		msg.message = MSG_ID_APP_MTHL_ACTIVATE_PDP_REQ;
		msg.wParam = (WPARAM)active_pdp_param_req;
		msg.lParam = -1;

		SendToLuaSocketMessage(&msg);

		return 0;
	}
}

INT32 platform_on_active_pdp_cnf(lua_State *L, PlatformMessage* pMSG)
{
	unsigned index;
	mthl_activate_pdp_cnf_struct *active_pdp_cnf;

	active_pdp_cnf = (mthl_activate_pdp_cnf_struct*)pMSG->local_para_ptr;
	index = active_pdp_cnf->user_data;
	if(index == LUA_INVALID_SOCKET_INDEX)
	{
		active_pdp_cnf->result = FALSE;
		LogWriter::LOGX("[platform_socket] platform_on_create_pdp_param_cnf PARAM_INVALID");
	}

	if(active_pdp_cnf->result != TRUE)
	{
		lua_pushinteger(L, MSG_ID_TCPIP_PDP_ACTIVATE_CNF);
		lua_pushinteger(L, FALSE);
		lua_pushinteger(L, active_pdp_cnf->error_cause); 
	}
	else
	{
		lua_pushinteger(L, MSG_ID_TCPIP_PDP_ACTIVATE_CNF);
		lua_pushinteger(L, TRUE);
		lua_pushinteger(L, 0);

		#ifdef LUA_SOCKET_TEST
		platform_socket_conn(0, SOC_SOCK_STREAM, 8001, "120.209.197.145");
		#endif
	}

    return 3;
}



INT32 platform_on_send_data_cnf(lua_State *L, PlatformMessage* pMSG)
{
    mthl_send_data_cnf_struct *send_data_cnf;

    send_data_cnf = (mthl_send_data_cnf_struct*)pMSG->local_para_ptr;
    if(send_data_cnf->result != TRUE)
    {
        lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_SEND_CNF);
        lua_pushinteger(L, platform_lua_socket_id_to_index(send_data_cnf->sock_id));
        lua_pushinteger(L, send_data_cnf->result);
        lua_pushinteger(L, send_data_cnf->ret_val); 
        return 4;
    }
    else 
    {
        lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_SEND_CNF);
        lua_pushinteger(L, platform_lua_socket_id_to_index(send_data_cnf->sock_id));
        lua_pushinteger(L, TRUE);
        lua_pushinteger(L, 0); 
        return 4;
    }
}



INT32 platform_on_send_data_ind(lua_State *L, PlatformMessage* pMSG)
{
	mthl_send_data_ind_struct* send_data_ind = (mthl_send_data_ind_struct*)pMSG->local_para_ptr;

	lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_SEND_IND);
	lua_pushinteger(L, platform_lua_socket_id_to_index(send_data_ind->sock_id));
	lua_pushinteger(L, send_data_ind->result);
    
    if(send_data_ind->result != TRUE)
    {
        lua_pushinteger(L, send_data_ind->ret_val); 
    }
    else
    {
        lua_pushinteger(L, 0); 
    }

    return 4;
}

#define SOCKET_DATA_RECV_LEN 2048

UINT8 socket_data[SOCKET_DATA_RECV_LEN];

INT32 platform_on_recv_data_ind(lua_State *L, PlatformMessage* pMSG)
{
    UINT32 socket_index;
    INT32 recv_result;
    INT32 recv_count = 0;
    INT32 to_recv_bytes;
    
    lua_socket_info_struct* sock_info;
    mthl_recv_data_ind_struct* recv_data_ind = (mthl_recv_data_ind_struct*)pMSG->local_para_ptr;
    
    socket_index = platform_lua_socket_id_to_index(recv_data_ind->sock_id);

    sock_info = &lua_socket_context.socket_info[socket_index];
    

    assert((socket_index >= 0) && (socket_index < LUA_MAX_SOCKET_SUPPORT));
    
    if(recv_data_ind->result != TRUE)
    {
        assert(0);
        return 0;
    }
    else
    {
        sockaddr_struct from_addr;

        memcpy(from_addr.addr, sock_info->addr, MAX_SOCK_ADDR_LEN);
        from_addr.port = sock_info->port;
        from_addr.addr_len = 4;
        from_addr.sock_type = SOC_SOCK_DGRAM;
        
        do
        {
            to_recv_bytes = QueueGetFreeSpace(&lua_socket_context.socket_info[socket_index].recv_queue);

            if(to_recv_bytes >  SOCKET_DATA_RECV_LEN)
            {
                to_recv_bytes = SOCKET_DATA_RECV_LEN;
            }
            else if(to_recv_bytes <= 0)
            {
                break;
            }
            
            if(sock_info->sock_type == SOC_SOCK_STREAM)
            {
                recv_result = soc_recv(recv_data_ind->sock_id,
                                        socket_data, 
                                        to_recv_bytes, 
                                        0);
            }
            else if(sock_info->sock_type == SOC_SOCK_DGRAM)
            {
                recv_result = soc_recvfrom(recv_data_ind->sock_id,
                                           socket_data, 
                                           to_recv_bytes, 
                                           0,
                                           &from_addr);
            }

            if(recv_result > 0)
            {
                assert((recv_result > 0) && (recv_result <= SOCKET_DATA_RECV_LEN));
                
                QueueInsert(&lua_socket_context.socket_info[socket_index].recv_queue, 
                            socket_data, 
                            recv_result);

                recv_count += recv_result;
            }
        }while(recv_result > 0);

        if(recv_count > 0)
        {
            lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_RECV_IND);

            lua_pushinteger(L, socket_index);
            lua_pushinteger(L, recv_count);
            return 3;
        }
        else
        {
            return 0;
        }
    }
}



INT32 platform_on_sock_close_ind(lua_State *L, PlatformMessage* pMSG)
{
    UINT32 socket_index;

    mthl_close_sock_ind_struct* sock_close_ind = (mthl_close_sock_ind_struct*)pMSG->local_para_ptr;
    socket_index = platform_lua_socket_id_to_index(sock_close_ind->sock_id);

    lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_CLOSE_IND);
    lua_pushinteger(L, socket_index);
    lua_pushinteger(L, sock_close_ind->result);

    lua_socket_context.socket_info[socket_index].sock_id = LUA_INVALID_SOKCET_ID;
    
    return 3;
}

INT32 platform_on_sock_close_cnf(lua_State *L, PlatformMessage* pMSG)
{  
    mthl_close_sock_cnf_struct* sock_close_cnf = (mthl_close_sock_cnf_struct*)pMSG->local_para_ptr;
    UINT32 socket_index = platform_lua_socket_id_to_index(sock_close_cnf->sock_id);

    if(sock_close_cnf->sock_id == (LUA_INVALID_SOKCET_ID & 0xff))
    {
        socket_index = sock_close_cnf->user_data;
    }
    
    lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_CLOSE_CNF);
    lua_pushinteger(L, socket_index);
    lua_pushinteger(L, sock_close_cnf->result);

    lua_socket_context.socket_info[socket_index].sock_id = LUA_INVALID_SOKCET_ID;

    return 3;
}

INT32 platform_on_deactivate_pdp_cnf(lua_State *L, PlatformMessage* pMSG)
{
    mthl_deactivate_pdp_cnf_struct* deactivate_pdp_cnf = (mthl_deactivate_pdp_cnf_struct*)pMSG->local_para_ptr;

    lua_pushinteger(L, MSG_ID_TCPIP_PDP_DEACTIVATE_CNF);
    lua_pushinteger(L, deactivate_pdp_cnf->result);

    if(deactivate_pdp_cnf->result == TRUE)
    {
        lua_pushinteger(L, 0);
    }
    else
    {
        lua_pushinteger(L, deactivate_pdp_cnf->error_cause);
    }

    return 3;
}


INT32 platform_on_deactivate_pdp_ind(lua_State *L, PlatformMessage* pMSG)
{
    mthl_deactivate_pdp_ind_struct* deactivate_pdp_ind = (mthl_deactivate_pdp_ind_struct*)pMSG->local_para_ptr;

    lua_pushinteger(L, MSG_ID_TCPIP_PDP_DEACTIVATE_IND);

    if(deactivate_pdp_ind->error == CBM_OK)
    {
        lua_pushinteger(L, TRUE);
        lua_pushinteger(L, 0);
    }
    else
    {
        lua_pushinteger(L, deactivate_pdp_ind->error);
        lua_pushinteger(L, deactivate_pdp_ind->error_cause);
    }

    return 3;
}



