/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    gps.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Global Positioning System(GPS).
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_gps.h"

// gpscore.open()
static int l_gps_open(lua_State *L) {
    lua_pushinteger(L, platform_gps_open());    
    return 1; 
}

// gpscore.close()
static int l_gps_close(lua_State *L) {
    lua_pushinteger(L, platform_gps_close());
    return 1; 
}


#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg gpscore_map[] =
{ 
  {"open",  l_gps_open},
  {"close", l_gps_close},
  { NULL, NULL }
};

LUALIB_API int luaopen_gpscore( lua_State *L )
{
    luaL_register( L, AUXLIB_GPSCORE, gpscore_map );
    return 1;
}
