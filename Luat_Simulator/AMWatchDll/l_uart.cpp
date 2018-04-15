/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    l_uart.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          UART.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "auxmods.h"
#include "platform.h"
#include "platform_conf.h"
//#include "common.h"
#include "sermux.h"


// Modes for the UART read function
enum
{
  UART_READ_MODE_LINE,
  UART_READ_MODE_NUMBER,
  UART_READ_MODE_SPACE,
  UART_READ_MODE_MAXSIZE
};

// Lua: actualbaud = setup( id, baud, databits, parity, stopbits,[mode] )
static int uart_setup( lua_State* L )
{
  unsigned id, databits, parity, stopbits;
  UINT32 baud, res;
  UINT32 mode;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  if( id >= SERMUX_SERVICE_ID_FIRST && id != PLATFORM_UART_ID_ATC)
    return luaL_error( L, "uart.setup can't be called on virtual UARTs" );
  baud = luaL_checkinteger( L, 2 );
  databits = luaL_checkinteger( L, 3 );
  parity = luaL_checkinteger( L, 4 );
  stopbits = luaL_checkinteger( L, 5 );
  mode = lua_tointeger(L, 6);
  res = platform_uart_setup( id, baud, databits, parity, stopbits, mode );

  if(res != baud)
  {
    return luaL_error( L, "uart.setup error ret(%d)", res );
  }
  
  lua_pushinteger( L, res );
  return 1;
}

// Lua: res = close( id )
static int uart_close( lua_State* L )
{
    UINT32 id = luaL_checkinteger(L,1);
    UINT32 res;

    res = platform_uart_close(id);

    lua_pushinteger(L, res);

    return 1;
}

// Lua: write( id, string1, [string2], ..., [stringn] )
static int uart_write( lua_State* L )
{
  int id;
  UINT8* buf;
  size_t len;
  int total = lua_gettop( L ), s;
  UINT32 wrote = 0;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );
  for( s = 2; s <= total; s ++ )
  {
    if( lua_type( L, s ) == LUA_TNUMBER )
    {
      len = lua_tointeger( L, s );
      if( ( len < 0 ) || ( len > 255 ) )
        return luaL_error( L, "invalid number" );
      wrote += platform_uart_send( id, ( UINT8 )len );
    }
    else
    {
      luaL_checktype( L, s, LUA_TSTRING );
      buf = (UINT8*)lua_tolstring( L, s, &len );
      wrote += platform_uart_send_buff(id, buf, len);
    }
  }

  lua_pushinteger(L, wrote);
  
  return 1;
}

static int uart_read( lua_State* L )
{
  int id, res, mode, issign;
  unsigned timer_id = 0;
  INT32 timeout = PLATFORM_UART_INFINITE_TIMEOUT, maxsize = 0, count = 0;
  const char *fmt;
  luaL_Buffer b;
  char cres;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );

  // Check format
  if( lua_isnumber( L, 2 ) )
  {
    if( ( maxsize = ( INT32 )lua_tointeger( L, 2 ) ) < 0 )
      return luaL_error( L, "invalid max size" );
    mode = UART_READ_MODE_MAXSIZE;
  }
  else
  {
    fmt = luaL_checkstring( L, 2 );
    if( !strcmp( fmt, "*l" ) )
      mode = UART_READ_MODE_LINE;
    else if( !strcmp( fmt, "*n" ) )
      mode = UART_READ_MODE_NUMBER;
    else if( !strcmp( fmt, "*s" ) )
      mode = UART_READ_MODE_SPACE;
    else
      return luaL_error( L, "invalid format" );
  }

  // Check timeout and timer id
  if( lua_gettop( L ) >= 3 )
  {
    timeout = luaL_checkinteger( L, 3 );
    if( ( timeout < 0 ) && ( timeout != PLATFORM_UART_INFINITE_TIMEOUT ) )
      return luaL_error( L, "invalid timeout value" );      
    if( ( timeout != PLATFORM_UART_INFINITE_TIMEOUT ) && ( timeout != 0 ) )
      timer_id = luaL_checkinteger( L, 4 );    
  }

  // Read data
  luaL_buffinit( L, &b );
  while( 1 )
  {
    if( ( res = platform_uart_recv( id, timer_id, timeout ) ) == -1 )
      break; 
    cres = ( char )res;
    count ++;
    issign = ( count == 1 ) && ( ( res == '-' ) || ( res == '+' ) );
    // [TODO] this only works for lines that actually end with '\n', other line endings
    // are not supported.
    if( ( cres == '\n' ) && ( mode == UART_READ_MODE_LINE ) )
    {
        luaL_putchar( &b, cres );
        break;
    }
    if( !isdigit( cres ) && !issign && ( mode == UART_READ_MODE_NUMBER ) )
      break;
    if( isspace( cres ) && ( mode == UART_READ_MODE_SPACE ) )
      break;
    luaL_putchar( &b, cres );
    if( ( count == maxsize ) && ( mode == UART_READ_MODE_MAXSIZE ) )
      break;
  }
  luaL_pushresult( &b );

  // Return an integer if needed
  if( mode == UART_READ_MODE_NUMBER )
  {
    res = lua_tointeger( L, -1 );
    lua_pop( L, 1 );
    lua_pushinteger( L, res );
  }
  return 1;  
}

// Lua: data = getchar( id, [ timeout ], [ timer_id ] )
static int uart_getchar( lua_State* L )
{
  int id, res;
  char cres;
  unsigned timer_id = 0;
  INT32 timeout = PLATFORM_UART_INFINITE_TIMEOUT;
  
  id = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( uart, id );

  // Check timeout and timer id
  if( lua_gettop( L ) >= 2 )
  {
    timeout = luaL_checkinteger( L, 2 );
    if( ( timeout < 0 ) && ( timeout != PLATFORM_UART_INFINITE_TIMEOUT ) )
      return luaL_error( L, "invalid timeout value" );      
    if( ( timeout != PLATFORM_UART_INFINITE_TIMEOUT ) && ( timeout != 0 ) )
      timer_id = luaL_checkinteger( L, 3 );    
  }
  res = platform_uart_recv( id, timer_id, timeout );
  if( res == -1 )
    lua_pushstring( L, "" );
  else
  {
    cres = ( char )res;
    lua_pushlstring( L, &cres, 1 );
  }
  return 1;  
}


// Lua: uart.set_buffer( id, size )
static int uart_set_buffer( lua_State *L )
{
  int id = luaL_checkinteger( L, 1 );
  UINT32 size = ( UINT32 )luaL_checkinteger( L, 2 );
  
  MOD_CHECK_ID( uart, id );
  if( size && ( size & ( size - 1 ) ) )
    return luaL_error( L, "the buffer size must be a power of 2 or 0" );
  if( size == 0 && id >= SERMUX_SERVICE_ID_FIRST )
    return luaL_error( L, "disabling buffers on virtual UARTs is not allowed" );
  if( platform_uart_set_buffer( id, intlog2( size ) ) == PLATFORM_ERR )
    return luaL_error( L, "unable to set UART buffer" );
  return 0;
}

// Lua: uart.set_flow_control( id, type )
static int uart_set_flow_control( lua_State *L )
{
  int id = luaL_checkinteger( L, 1 );
  int type = luaL_checkinteger( L, 2 );

  MOD_CHECK_ID( uart, id );
  if( platform_uart_set_flow_control( id, type ) != PLATFORM_OK )
    return luaL_error( L, "unable to set the flow control on interface %d", id );
  return 0;
}

static int os_sleep( lua_State *L )
{
    int ms = luaL_checkinteger( L, 1 );
    
    platform_os_sleep(ms);
    
    return 0;
}

#ifdef BUILD_SERMUX

#define MAX_VUART_NAME_LEN    6
#define MIN_VUART_NAME_LEN    6

// __index metafunction for UART
// Look for all VUARTx timer identifiers
static int uart_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
  char* pend;
  long res;
  
  if( strlen( key ) > MAX_VUART_NAME_LEN || strlen( key ) < MIN_VUART_NAME_LEN )
    return 0;
  if( strncmp( key, "VUART", 5 ) )
    return 0;  
  res = strtol( key + 5, &pend, 10 );
  if( *pend != '\0' )
    return 0;
  if( res >= SERMUX_NUM_VUART )
    return 0;
  lua_pushinteger( L, SERMUX_SERVICE_ID_FIRST + res );
  return 1;
}
#endif // #ifdef BUILD_SERMUX

// Module function map
#define MIN_OPT_LEVEL   2
const luaL_Reg uart_map[] = 
{
  {"setup", uart_setup},
  {"close", uart_close},
  {"write", uart_write},
  {"read", uart_read},
  {"getchar", uart_getchar},
  {"set_buffer", uart_set_buffer},
  {"set_flow_control", uart_set_flow_control},
  {"sleep", os_sleep},
#if LUA_OPTIMIZE_MEMORY > 0
  {"PAR_EVEN", PLATFORM_UART_PARITY_EVEN},
  {"PAR_ODD", PLATFORM_UART_PARITY_ODD},
  {"PAR_NONE", PLATFORM_UART_PARITY_NONE},
  {"STOP_1", PLATFORM_UART_STOPBITS_1},
  {"STOP_1_5", PLATFORM_UART_STOPBITS_1_5},
  {"STOP_2", PLATFORM_UART_STOPBITS_2},
  {"NO_TIMEOUT", 0},
  {"INF_TIMEOUT", PLATFORM_UART_INFINITE_TIMEOUT},
  {"FLOW_NONE", PLATFORM_UART_FLOW_NONE},
  {"FLOW_RTS", PLATFORM_UART_FLOW_RTS},
  {"FLOW_CTS", PLATFORM_UART_FLOW_CTS},
#endif
#if LUA_OPTIMIZE_MEMORY > 0 && defined( BUILD_SERMUX )
  {"__metatable", uart_map},
  {"__index", uart_mt_index},  
#endif
  {NULL, NULL}
};

LUALIB_API int luaopen_uart( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_UART, uart_map );
  
  MOD_REG_NUMBER( L, "ATC", PLATFORM_UART_ID_ATC );
  
  // Add the stop bits and parity constants (for uart.setup)
  MOD_REG_NUMBER( L, "PAR_EVEN", PLATFORM_UART_PARITY_EVEN );
  MOD_REG_NUMBER( L, "PAR_ODD", PLATFORM_UART_PARITY_ODD );
  MOD_REG_NUMBER( L, "PAR_NONE", PLATFORM_UART_PARITY_NONE );
  MOD_REG_NUMBER( L, "STOP_1", PLATFORM_UART_STOPBITS_1 );
  MOD_REG_NUMBER( L, "STOP_1_5", PLATFORM_UART_STOPBITS_1_5 );
  MOD_REG_NUMBER( L, "STOP_2", PLATFORM_UART_STOPBITS_2 );
  
  // Add the "none" and "infinite" constant used in recv()
  MOD_REG_NUMBER( L, "NO_TIMEOUT", 0 );
  MOD_REG_NUMBER( L, "INF_TIMEOUT", PLATFORM_UART_INFINITE_TIMEOUT );

  // Add the UART flow constants
  MOD_REG_NUMBER( L, "FLOW_RTS", PLATFORM_UART_FLOW_RTS );
  MOD_REG_NUMBER( L, "FLOW_CTS", PLATFORM_UART_FLOW_CTS );
  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}

