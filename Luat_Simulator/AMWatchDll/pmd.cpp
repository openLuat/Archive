/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    pmd.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Register all functions of 'PMD' into LUA's stack.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_pmd.h"

static int getFiledInt(lua_State *L, int index, const char *key, int defval)
{
    lua_getfield(L, index, key);
    return luaL_optint(L, -1, defval);
}

static int pmd_init(lua_State *L) {
    #define GET_FIELD_VAL(fIELD, dEFault) pmdcfg.fIELD = getFiledInt( L, 1, #fIELD, dEFault)

    PlatformPmdCfg pmdcfg;

    luaL_checktype(L, 1, LUA_TTABLE);
    
    GET_FIELD_VAL(ccLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(cvLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(ovLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(pvLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(poweroffLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(ccCurrent, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(fullCurrent, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(batdetectEnable, PMD_CFG_INVALID_VALUE);

    lua_pushinteger(L, platform_pmd_init(&pmdcfg));

    return 1;
}

static int pmd_ldo_set(lua_State *L) {
    INT total = lua_gettop(L);
    INT level = luaL_checkinteger(L, 1);
    INT i;
    INT ldo;

    for(i = 2; i <= total; i++)
    {
        ldo = luaL_checkinteger(L, i);
        platform_ldo_set((PlatformLdoId)ldo, level);
    }

    return 0; 
}

// pmd.sleep(sleepornot)
static int pmd_deepsleep(lua_State *L) {    
    int sleep = luaL_checkinteger(L,1);

    platform_pmd_powersave(sleep);
    return 0; 
}

static int pmd_charger(lua_State *L) {
    lua_pushboolean(L, platform_pmd_get_charger());
    return 1;
}

static int pmd_chargingcurrent(lua_State *L) {
    lua_pushinteger(L, platform_pmd_getChargingCurrent());
    return 1;
}

#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg pmd_map[] =
{ 
  {"init",  pmd_init},
  {"ldoset",  pmd_ldo_set},
  {"sleep",  pmd_deepsleep},
  {"charger",  pmd_charger},
  {"chrcurrent",  pmd_chargingcurrent},
  { NULL, NULL}
};

LUALIB_API int luaopen_pmd( lua_State *L )
{
    luaL_register( L, AUXLIB_PMD, pmd_map );

    MOD_REG_NUMBER(L, "LDO_KEYPAD", PLATFORM_LDO_KEYPAD);
    MOD_REG_NUMBER(L, "LDO_LCD", PLATFORM_LDO_LCD);
    MOD_REG_NUMBER(L, "KP_LEDR", PLATFORM_LDO_KP_LEDR);
    MOD_REG_NUMBER(L, "KP_LEDG", PLATFORM_LDO_KP_LEDG);
    MOD_REG_NUMBER(L, "KP_LEDB", PLATFORM_LDO_KP_LEDB);
    MOD_REG_NUMBER(L, "LDO_VIB", PLATFORM_LDO_VIB);
    MOD_REG_NUMBER(L, "LDO_VLCD", PLATFORM_LDO_VLCD);
    MOD_REG_NUMBER(L, "LDO_VASW", PLATFORM_LDO_VASW);
    MOD_REG_NUMBER(L, "LDO_VMMC", PLATFORM_LDO_VMMC);
    MOD_REG_NUMBER(L, "LDO_VCAM", PLATFORM_LDO_VCAM);
    MOD_REG_NUMBER(L, "LDO_SINK", PLATFORM_LDO_SINK);
    MOD_REG_NUMBER(L, "LDO_VSIM1", PLATFORM_LDO_VSIM1);
    MOD_REG_NUMBER(L, "LDO_VSIM2", PLATFORM_LDO_VSIM2);
    MOD_REG_NUMBER(L, "LDO_VMC",   PLATFORM_LDO_VMC);
    
    return 1;
}  

