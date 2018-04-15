/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    ttsply.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          ttsply.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_ttsply.h"

#if defined(__AM_LUA_TTSPLY_SUPPORT__)

static int l_ttsply_initEngine(lua_State *L) {
    ttsPlyParam param;

    param.volume = (INT16)luaL_checkinteger(L, 1);
    param.speed = (INT16)luaL_checkinteger(L, 2);
    param.pitch = (INT16)luaL_checkinteger(L, 3);
    param.codepage = (UINT16)luaL_checkinteger(L, 4);
    param.digit_mode = (UINT8)luaL_checkinteger(L, 5);
    param.punc_mode = (UINT8)luaL_checkinteger(L, 6);
    param.tag_mode = (UINT8)luaL_checkinteger(L, 7);
    param.wav_format = (UINT8)luaL_checkinteger(L, 8);
    param.eng_mode = (UINT8)luaL_checkinteger(L, 9);

    //lua_pushinteger(L, platform_ttsply_initEngine(&param));
    lua_pushinteger(L, platform_ttsply_init(&param));

    return 1;
}

static int l_ttsply_setParam(lua_State *L) {
    UINT16 plyParam = (UINT16)luaL_checkinteger(L, 1);
    INT16 value = (INT16)luaL_checkinteger(L, 2);

    lua_pushinteger(L, platform_ttsply_setParam(plyParam, value));

    return 1;
}

static int l_ttsply_getParam(lua_State *L) {
    UINT16 plyParam = (UINT16)luaL_checkinteger(L, 1);

    lua_pushinteger(L, platform_ttsply_getParam(plyParam));

    return 1;
}

static int l_ttsply_play(lua_State *L) {
    ttsPly param;

    param.text = (WORD*)luaL_checkstring(L,1);
    param.text_size = (UINT32)luaL_checkinteger(L, 2);
    param.spk_vol = (UINT8)luaL_checkinteger(L, 3);

    //LogWriter::LOGX("l_ttsply_play text_size=%d, spk_vol=%d.", param.text_size, param.spk_vol);

    lua_pushinteger(L, platform_ttsply_play(&param));

    return 1;
}

static int l_ttsply_pause(lua_State *L) {
    /*
    ttsPly param;

    param.text = luaL_checkstring(L,1);
    param.text_size = cast(u32, luaL_checkinteger(L, 2));
    param.spk_vol = cast(u8, luaL_checkinteger(L, 3));
  
    lua_pushinteger(L, platform_ttsply_pause(&param));
    */

    lua_pushinteger(L, platform_ttsply_pause());
    
    return 1;
}

static int l_ttsply_stop(lua_State *L) {
    /*
    ttsPly param;

    param.text = luaL_checkstring(L,1);
    param.text_size = cast(u32, luaL_checkinteger(L, 2));
    param.spk_vol = cast(u8, luaL_checkinteger(L, 3));
  
    lua_pushinteger(L, platform_ttsply_stop(&param));
    */

    lua_pushinteger(L, platform_ttsply_stop());
    
    return 1;
}

#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg ttsplycore_map[] =
{ 
  {"initEngine", l_ttsply_initEngine},
  {"setParm", l_ttsply_setParam},
  {"getParam", l_ttsply_getParam},
  {"play", l_ttsply_play},
  {"pause", l_ttsply_pause},
  {"stop", l_ttsply_stop},

  {NULL, NULL}
};

LUALIB_API int luaopen_ttsplycore( lua_State *L )
{
    luaL_register( L, AUXLIB_TTSPLYCORE, ttsplycore_map );

    return 1;
}

#else

// Module function map
const luaL_Reg ttsplycore_map[] =
{ 
  {NULL, NULL}
};

LUALIB_API int luaopen_ttsplycore( lua_State *L )
{
    luaL_register( L, AUXLIB_TTSPLYCORE, ttsplycore_map );

    return 1;
}

#endif //__AM_LUA_TTSPLY_SUPPORT__

