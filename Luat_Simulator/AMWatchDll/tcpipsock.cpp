/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    tcpipsock.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'tcpipsock' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_socket.h"

static int l_pdp_activate(lua_State *L) {
    size_t  len      = 0;
    char* apn;
    char* username;
    char* password;
    
    apn      = (char*)luaL_checklstring(L, 1, &len);
    username = (char*)luaL_checklstring(L, 2, &len);
    password = (char*)luaL_checklstring(L, 3, &len);
    
    lua_pushinteger(L, platform_activate_pdp(apn, username, password));    
    return 1; 
}


static int l_pdp_deactivate(lua_State *L) {
    lua_pushinteger(L, platform_deactivate_pdp());   
    return 1; 
}


static int l_sock_conn(lua_State *L) {
    size_t len      = 0;
    int sock_id;
    int sock_type;
    int port;
    CHAR* addr;
    
    sock_id = luaL_checkinteger(L, 1);
    sock_type  = luaL_checkinteger(L, 2);
    port       = luaL_checkinteger(L, 3);
    addr       = (char*)luaL_checklstring(L, 4, &len);

    lua_pushinteger(L, platform_socket_conn(sock_id, 
                                            (socket_type_enum)sock_type,
                                            port,
                                            addr));   
    return 1; 
}



static int l_sock_send(lua_State *L) {
    size_t len      = 0;
    int sock_index;
    CHAR* buf;

    sock_index = luaL_checkinteger(L, 1);
  
    luaL_checktype( L, 2, LUA_TSTRING );
    
    buf = (char*)lua_tolstring( L, 2, &len );
 
    lua_pushinteger(L, platform_socket_send(sock_index, buf, len));
    return 1;
}

static int l_sock_close(lua_State *L) {
    int sock_index;
    int graceful_close;

    sock_index     = luaL_checkinteger(L, 1);
    graceful_close = luaL_checkinteger(L, 2);

    lua_pushinteger(L, platform_socket_close(sock_index, graceful_close));
    return 1;
}


static int l_sock_recv(lua_State *L) {
    int sock_index;
    int total_len = 0; 
    luaL_Buffer b;
    int count ;
    
    sock_index     = luaL_checkinteger(L, 1);
    total_len      = luaL_checkinteger(L, 2);
    count          = total_len / LUAL_BUFFERSIZE;

	LogWriter::LOGX("[tcpipsock] l_sock_recv sock_index=%d, total_len=%d, count=%d", sock_index, total_len, count);
    
    luaL_buffinit( L, &b );

    while(count-- > 0)
    {
        platform_socket_recv(sock_index, (UINT8*)b.p, LUAL_BUFFERSIZE);
        b.p += LUAL_BUFFERSIZE;
        luaL_prepbuffer(&b);
    }

    if(total_len % LUAL_BUFFERSIZE)
    {
        platform_socket_recv(sock_index, (UINT8*)b.p, total_len % LUAL_BUFFERSIZE);
        b.p += (total_len % LUAL_BUFFERSIZE);
    }
    
    luaL_pushresult( &b );

    platform_socket_on_recv_done(sock_index);
    
    return 1;
} 

#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg tcpipsock_map[] =
{ 
  {"pdp_activate",  l_pdp_activate},
  {"pdp_deactivate", l_pdp_deactivate},
  {"sock_conn",  l_sock_conn},
  {"sock_send",  l_sock_send},
  {"sock_close", l_sock_close},
  {"sock_recv",  l_sock_recv},
  {NULL, NULL}
};

LUALIB_API int luaopen_tcpipsock( lua_State *L )
{
    luaL_register( L, AUXLIB_TCPIPSOCK, tcpipsock_map);
    platform_lua_socket_init();
    return 1;
}  
