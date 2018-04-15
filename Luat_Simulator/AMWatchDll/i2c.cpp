/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    i2c.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Module for interfacing with the I2C interface.
 * History:
 *     panjun 16/09/13 Initially create file.
 *     panjun 17/02/22 Add a dummy API,i2c.close.
 **************************************************************************/
#include "stdafx.h"
#include "lua.hpp"
#include "auxmods.h"
#include "platform.h"
#include "platform_i2c.h"

static int i2c_gSensorParam_get(lua_State *L)
{
  UINT id = luaL_checkinteger( L, 1 );
  UINT slave_addr = 0;
  UINT slave_id = 0;

  platform_i2c_gSensorParam_get(id, (UINT8*)&slave_addr, (UINT8*)&slave_id);
  
  lua_pushinteger( L, slave_addr);
  lua_pushinteger( L, slave_id);
  return 2;
}

// Lua: speed = i2c.setup( id, speed, slaveaddr )
static int i2c_setup(lua_State *L)
{
    unsigned id = luaL_checkinteger(L, 1);
    PlatformI2CParam i2cParam;

    i2cParam.speed = (UINT32)luaL_checkinteger(L, 2);
    i2cParam.slaveAddr = (UINT8)luaL_checkinteger(L, 3);

    MOD_CHECK_ID(i2c, id);
    lua_pushinteger(L, platform_i2c_setup(id, &i2cParam));
    return 1;
}

// Lua: wrote = i2c.write( id, [slave,] reg, data )
// data can be either a string, a table or an 8-bit number
static int i2c_write( lua_State *L )
{
    UINT8 arg_index = 1;
    unsigned id = luaL_checkinteger( L, arg_index++ );
    UINT16 slave_addr = I2C_NULL_SLAVE_ADDR;

    UINT8 regAddr;
    UINT32 write = 0;

    if(lua_gettop(L) == 4){
        slave_addr = luaL_checkinteger(L, arg_index++);
    }
    regAddr = (UINT8)luaL_checkinteger(L, arg_index++);

    MOD_CHECK_ID( i2c, id );

    switch(lua_type(L, arg_index))
    {
    case LUA_TNUMBER:
        {
            UINT8 numdata = (UINT8)luaL_checkinteger(L, arg_index);

            write = platform_i2c_send_data(id,slave_addr,(PCSTR)&regAddr,(PCSTR)&numdata,1);
        }
        break;

    case LUA_TSTRING:
        {
            PCSTR pdata;
            UINT32 slen;

            pdata = luaL_checklstring(L, arg_index, (size_t*)&slen);
            write = platform_i2c_send_data(id,slave_addr,(PCSTR)&regAddr,pdata,slen);
        }
        break;

    case LUA_TTABLE:
        {
            size_t datalen = lua_objlen(L, arg_index);
            size_t i;
            PSTR pBuff;

			pBuff = (PSTR)WinUtil::L_MALLOC(datalen);

            for(i = 0; i < datalen; i++)
            {
                lua_rawgeti(L, arg_index, i+1);
                pBuff[i] = luaL_checkinteger(L, -1);
            }

            write = platform_i2c_send_data(id,slave_addr,(PCSTR)&regAddr,pBuff,datalen);

			WinUtil::L_FREE(pBuff);
        }
        break;

    default:
        return luaL_error(L, "i2c.write: data must be number,string,table");
        break;
    }

    lua_pushinteger(L, write);
    return 1;
}

// Lua: read = i2c.read( id, [slave,] reg, size )
static int i2c_read( lua_State *L )
{
    UINT8 arg_index = 1;
    unsigned id = luaL_checkinteger( L, arg_index++ );
    UINT16 slave_addr = I2C_NULL_SLAVE_ADDR;

    UINT8 regAddr;
    UINT32 size;
    luaL_Buffer b;

    if(lua_gettop(L) == 4){
        slave_addr = luaL_checkinteger(L, arg_index++);
    }
    regAddr = (UINT8)luaL_checkinteger(L, arg_index++);
    size = (UINT32)luaL_checkinteger( L, arg_index++ );

    MOD_CHECK_ID( i2c, id );
    if( size == 0 )
        return 0;
    
    if(size >= LUAL_BUFFERSIZE)
        return luaL_error(L, "i2c.read: size must < %d", LUAL_BUFFERSIZE);

    luaL_buffinit( L, &b );

    platform_i2c_recv_data(id, slave_addr, (PCSTR)&regAddr, b.p, size);
    b.p += size;

    luaL_pushresult( &b );

    return 1;
}

static int i2c_close( lua_State *L )
{
	UINT id = luaL_checkinteger(L, 1);
	LogWriter::LOGX("i2c_close(%d).",id);
	lua_pushinteger(L, id);

	return 1;
}

// Module function map
#define MIN_OPT_LEVEL   2
const luaL_reg i2c_map[] = 
{
  {"setup", i2c_setup},
  {"write", i2c_write},
  {"read", i2c_read},
  {"gsensorParam_get",  i2c_gSensorParam_get},
#if LUA_OPTIMIZE_MEMORY > 0
  {"FAST", PLATFORM_I2C_SPEED_FAST},
  {"SLOW", PLATFORM_I2C_SPEED_SLOW},
#endif //LUA_OPTIMIZE_MEMORY
  {"close", i2c_close},
  {NULL, NULL}
};

LUALIB_API int luaopen_i2c( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_I2C, i2c_map );
  MOD_REG_NUMBER( L, "FAST", PLATFORM_I2C_SPEED_FAST );
  MOD_REG_NUMBER( L, "SLOW", PLATFORM_I2C_SPEED_SLOW ); 
#endif
  
  return 1;
}

