/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    watchdog.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Watchdog.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_watchdog.h"

// watchdog.open(mode[,io])
static int l_watchdog_open(lua_State *L) {
    watchdog_info_t info;

    info.mode = luaL_checkinteger(L, 1);
    info.param.pin_ctl = luaL_checkinteger(L, 2);
    
    lua_pushinteger(L, platform_watchdog_open(&info));
    
    return 1; 
}

// watchdog.close()
static int l_watchdog_close(lua_State *L) {
    lua_pushinteger(L, platform_watchdog_close());
    return 1; 
}

// watchdog.kick()
static int l_watchdog_kick(lua_State *L) {
    lua_pushinteger(L, platform_watchdog_kick());
    return 1; 
}

#define MIN_OPT_LEVEL 2
// Module function map
const luaL_reg watchdog_map[] =
{ 
  {"open",  l_watchdog_open},
  {"close", l_watchdog_close},
  {"kick",  l_watchdog_kick},
  { NULL, NULL }
};

LUALIB_API int luaopen_watchdog( lua_State *L )
{
    luaL_register( L, AUXLIB_WATCHDOG, watchdog_map );
    
    MOD_REG_NUMBER(L, "DEFAULT", WATCHDOG_DEFAULT_MODE);

    return 1;
}  
