/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    adc.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          ADC.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_pmd.h"

// adc.open(id)
static int adc_open(lua_State *L) {
    INT id = luaL_checkinteger(L, 1);
    INT mode = luaL_checkinteger(L, 2);
    INT ret;

    MOD_CHECK_ID(adc, id);

    ret = platform_adc_open(id, mode);

    lua_pushinteger(L, ret);

    return 1; 
}

static int adc_close(lua_State *L) {
    int ret;

    ret = platform_adc_close();

    lua_pushinteger(L, ret);

    return 1; 
}

// adc.read(id)
static int adc_read(lua_State *L) {    
    int id = luaL_checkinteger(L,1);
    int adc, volt;

    MOD_CHECK_ID(adc, id);

    platform_adc_read(id, &adc, &volt);

    lua_pushinteger(L, adc);
    lua_pushinteger(L, volt);
   
    return 2; 
}

#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg adc_map[] =
{ 
  {"open",  adc_open},
  {"read",  adc_read},
  {"close", adc_close},
  {NULL, NULL}
};

LUALIB_API int luaopen_adc( lua_State *L )
{
    luaL_register( L, AUXLIB_ADC, adc_map );

    return 1;
}  
