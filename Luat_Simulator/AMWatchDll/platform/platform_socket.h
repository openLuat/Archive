/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_socket.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          TCPIP SOCKET.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#if !defined(__PLATFORM_SOCKET_H__)
#define __PLATFORM_SOCKET_H__

#include "lua.hpp"
#include "mthl.h"

#define LUA_SOCKET_THREAD_STACK_SIZE (10*1024)

typedef struct 
{
	socket_type_enum sock_type;
	UINT16    port;
	char    addr[MAX_SOCK_ADDR_LEN];  
	UINT32  sock_id;
	CycleQueue recv_queue;
	UINT8  recv_buf[LUA_SOCKET_RX_BUF_SIZE];
}lua_socket_info_struct;

typedef struct
{
	char  apn[ MAX_APN_LEN + 1];
	char  user_name[CUSTOM_DTCNT_PROF_MAX_USER_LEN+1];     /* User string of a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_USER_LEN) */
	char  password[CUSTOM_DTCNT_PROF_MAX_PW_LEN+1];         /* Password for a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_PW_LEN) */
	lua_socket_info_struct socket_info[LUA_MAX_SOCKET_SUPPORT];
}lua_socket_context_struct;

void platform_lua_socket_init(void);

BOOL platform_activate_pdp(CHAR* apn, CHAR* user_name, CHAR* password);

BOOL platform_deactivate_pdp(void);

BOOL platform_socket_send(UINT8 socket_index, CHAR* data, UINT16 length);

UINT32 platform_socket_recv(UINT8 socket_index, UINT8* data, UINT16 length);

BOOL platform_socket_close(UINT8 socket_index, BOOL graceful_close);

BOOL platform_socket_conn(UINT8 socket_id,
	socket_type_enum sock_type,
	UINT16	port, CHAR*	addr);

BOOL platform_get_ip_by_name(UINT8 index, char* url);

INT32 platform_on_get_host_by_name_cnf(lua_State *L, PlatformMessage* pMsg);

INT32 platform_on_create_conn_cnf(lua_State *L, PlatformMessage* pMsg);

INT32 platform_on_create_sock_ind(lua_State *L, PlatformMessage* pMsg);

INT32 platform_on_create_pdp_param_cnf(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_active_pdp_cnf(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_send_data_cnf(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_send_data_ind(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_recv_data_ind(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_deactivate_pdp_cnf(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_deactivate_pdp_ind(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_sock_close_ind(lua_State *L, PlatformMessage* pMSG);

INT32 platform_on_sock_close_cnf(lua_State *L, PlatformMessage* pMSG);

void platform_socket_on_recv_done(UINT8 socket_index);

BOOL platform_socket_conn_IP(UINT8 index);

#endif //__PLATFORM_SOCKET_H__
