/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    auxmods.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *
 *
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __AUXMODS_H__
#define __AUXMODS_H__

#include "lua.h"

#define AUXLIB_PIO      "pio"
LUALIB_API int (luaopen_pio)(lua_State *L);

#define AUXLIB_UART     "uart"
LUALIB_API int (luaopen_uart)( lua_State *L );


#define AUXLIB_PACK     "pack"
LUALIB_API int luaopen_pack( lua_State *L );

#define AUXLIB_BIT      "bit"
LUALIB_API int luaopen_bit( lua_State *L );

#define AUXLIB_CPU      "cpu"
LUALIB_API int luaopen_cpu(lua_State* L);

#define AUXLIB_ADC      "adc"
LUALIB_API int luaopen_adc( lua_State *L );


#define AUXLIB_BITARRAY "bitarray"
LUALIB_API int luaopen_bitarray( lua_State *L );

#define AUXLIB_I2C  "i2c"
LUALIB_API int luaopen_i2c( lua_State *L );

#define AUXLIB_RTOS     "rtos"
LUALIB_API int luaopen_rtos(lua_State *L);

#define AUXLIB_DISP     "disp"
LUALIB_API int luaopen_disp(lua_State *L);

#define AUXLIB_JSON     "json"
LUALIB_API int luaopen_cjson( lua_State *L );

#define AUXLIB_PMD     "pmd"
LUALIB_API int luaopen_pmd(lua_State *L);

#define AUXLIB_ICONV     "iconv"
LUALIB_API int luaopen_iconv( lua_State *L );

#define AUXLIB_AUDIOCORE "audiocore"
LUALIB_API int luaopen_audiocore( lua_State *L );

#define AUXLIB_WATCHDOG      "watchdog"
LUALIB_API int luaopen_watchdog( lua_State *L );

#define AUXLIB_GPSCORE      "gpscore"
LUALIB_API int luaopen_gpscore( lua_State *L );

#define AUXLIB_TCPIPSOCK      "tcpipsock"
LUALIB_API int luaopen_tcpipsock( lua_State *L );

#define AUXLIB_QRENCODE      "qrencode"
LUALIB_API int luaopen_qr_encode( lua_State *L );

#define AUXLIB_TTSPLYCORE     "ttsply"
LUALIB_API int luaopen_ttsplycore( lua_State * L );

#define MOD_CHECK_ID( mod, id )\
  if( !platform_ ## mod ## _exists( id ) )\
    return luaL_error( L, #mod" %d does not exist", ( unsigned )id )

#define MOD_CHECK_RES_ID( mod, id, resmod, resid )\
  if( !platform_ ## mod ## _check_ ## resmod ## _id( id, resid ) )\
    return luaL_error( L, #resmod" %d not valid with " #mod " %d", ( unsigned )resid, ( unsigned )id )

#define MOD_REG_NUMBER( L, name, val )\
  lua_pushnumber( L, val );\
  lua_setfield( L, -2, name )
  
#endif //__AUXMODS_H__

