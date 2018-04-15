/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    Win32LuaShell.h
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

#ifndef __WIN32_LUA_SHELL__
#define __WIN32_LUA_SHELL__

#include "lua.hpp"
#include "Windows.h"
#include "AMWatchDll.h"

/*-------------------------------------------------------------------------
* Current LuaT library version
 -------------------------------------------------------------------------*/
#define LUAT_VERSION    "LuaT 2.0-rc1"
#define LUAT_COPYRIGHT  "Copyright (C) 2015-2017 AirM2M LuaT"

#define LUA_LOG_HEADER "[LUA] "

#define SIMLA_MAIN_THREAD_STACK_SIZE (64*1024)
#define DAEMON_THREAD_STACK_SIZE (16*1024)
#define LOAD_LUA_TSTACK_SIZE (1024*1024)
#define LUA_SHELL_TSTACK_SIZE (16*1024)

extern ULONG gDaemonThreadId;
extern HANDLE gDaemonThread;

extern ULONG gLoadLuaThreadId;
extern HANDLE gLoadLuaThread;

extern ULONG gLuaShellThreadId;
extern HANDLE gLuaShellThread;

extern int platform_init(void);
extern int platform_uart_init(void);

LUALIB_API int luaopen_LuaShell(lua_State *L);

AMWATCH_DLL_API int LuaMain(void);
AMWATCH_DLL_API int luaopen_AMWatch(lua_State* L);

#endif //__WIN32_LUA_SHELL__
