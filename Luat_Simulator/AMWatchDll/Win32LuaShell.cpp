/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    Win32LuaShell.cpp
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

#include "stdafx.h"
#include "direct.h"
#include "io.h"
#include "lua.hpp"
#include "dllmain.h"
#include "platform_conf.h"
#include "Win32LuaShell.h"

ULONG gDaemonThreadId = NULL;
HANDLE gDaemonThread = NULL;

ULONG gLoadLuaThreadId = NULL;
HANDLE gLoadLuaThread = NULL;

ULONG gLuaShellThreadId = NULL;
HANDLE gLuaShellThread = NULL;

lua_State* gL = NULL;


void join_lroot(lua_State* L, CHAR* path, CHAR* name)
{
	CHAR regPath[FS_PATH_LEN] = {0};
	CHAR prefix[FS_PATH_LEN] = {0};
	CHAR *root;

	lua_getglobal(L,"_LROOT");
	root = (CHAR*)lua_tostring(L,-1);
	if(strlen(root) != 0)
	{
		WinUtil::separator_strrpl(regPath, (CHAR*)name, "/", "\\");
		strcat(prefix, root);
		strcat(prefix,"\\src");
		strcat(prefix,regPath);
	}
	else
	{
		strcat(prefix,name);
	}

	WinUtil::separator_strrpl(path, prefix, "\\", "\\\\");

	lua_pop(L,1);
}

int traceback (lua_State *L) {
	const CHAR *msg = NULL;
	CHAR *sub = NULL;

	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}

	msg = lua_tostring(L, 1);
	sub = (CHAR*)strrchr(msg, '\\');
	if(sub == NULL)
	{
		sub = (CHAR*)msg;
	}
	else
	{
		sub = sub + 1;
	}
	LogWriter::LOGX("[Win32LuaShell] traceback %s",sub);
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */

	Disp::GetDisp()->WinAbnormalExit(NULL, sub);
	return 1;
}

int LuaMain(void)
{
	int status;
	int base;
	lua_getglobal(gL,"win32luamain");
	base = lua_gettop(gL);  /* function index */
	lua_pushcfunction(gL, traceback);  /* push traceback function */
	lua_insert(gL, base);  /* put it under chunk and args */
	status = lua_pcall(gL, 0, 0, base);
	lua_remove(gL, base);  /* remove traceback function */

	LogWriter::LOGX("[Win32LuaShell] LuaMain. status=%d", status);

	return status;
}

DWORD WINAPI LoadLuaThreadProc(LPVOID lpParam){
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))
	{  
		if(WM_LUA_MAIN == msg.message)
		{			
			LogWriter::LOGX("LoadLuaThreadProc WM_LUA_MAIN.");
			LuaMain();
		}
		else
		{
			//TO DO ...
		}
	} 

	LogWriter::LOGX("[Win32LuaShell] LoadLuaThreadProc QUIT %d.",GetLastError());

	return 0;
}

DWORD WINAPI LuaShellRoutine(LPVOID p)
{
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))  
	{  
		TranslateMessage(&msg);

		switch(msg.message)
		{
		case WM_LUA_MAIN:
			LogWriter::LOGX("LuaShellRoutine WM_LUA_MAIN.");
			LuaMain();
			break;

		case WM_LUA_PLAY_GIF:
			if(msg.wParam != NULL)
			{
				PlayGifImage *playInfo = (PlayGifImage*)msg.wParam;
				Disp::GetDisp()->playgif(playInfo->szImagePath,\
					playInfo->x, playInfo->y, playInfo->times);
				WinUtil::L_FREE((void*)msg.wParam);
			}
			break;

		case WM_LUA_STOP_GIF:
			Disp::GetDisp()->stopgif();
			break;

		default:
			break;
		}

		DispatchMessage(&msg);  
	}

	LogWriter::LOGX("[Win32LuaShell] LuaShellRoutine QUIT %d.", GetLastError());

	return 0;
}

int LuaShell_Print(lua_State* L)
{
	int n = lua_gettop(L);  /* number of arguments */
	const char *s = NULL;
	int i;
	char buffer[1024] = {0};
	strcat(buffer, LUA_LOG_HEADER);
	for (i=1; i<=n; i++) {
		lua_pushvalue(L, i);
		s = lua_tostring(L, -1);
        if (s == NULL) continue;
		if (i>1) strcat(buffer+strlen(buffer)," ");
		strcat(buffer+strlen(buffer), s);
		lua_pop(L, 1);
	}
	lua_settop(L, 0);

	LogWriter::LOGX(buffer);
	strcat(buffer+strlen(buffer), "\n");
	fprintf(stdout,buffer+strlen(LUA_LOG_HEADER));

	return 0;
}

int LuaShell_Makedir(lua_State* L)
{
	PCSTR filename = luaL_checkstring(L, 1);
	int mode = luaL_checkinteger(L, 2);
	CHAR fullpath[FS_PATH_LEN] = {0};
	
	if(filename[0] == '-')
	{
		join_lroot(L, fullpath, (CHAR*)(filename+2));
	}
	else
	{
		join_lroot(L, fullpath, (CHAR*)filename);
	}

	if(_access(fullpath,0) == 0)
	{
		lua_pushinteger(L, 0);
	}
	else
	{
		lua_pushinteger(L, _mkdir(fullpath));
	}

	return 1;
}

int LuaShell_Findfirstdir(lua_State* L)
{
	struct _finddata_t fileinfo;
	intptr_t fHdl;
	PCSTR filename = luaL_checkstring(L, 1);
	CHAR fullpath[FS_PATH_LEN] = {0};

	if(filename[0] == '-')
	{
		join_lroot(L, fullpath, (CHAR*)(filename+2));
	}
	else
	{
		join_lroot(L, fullpath, (CHAR*)filename);
	}

	memset(&fileinfo, 0, sizeof(fileinfo));
	fileinfo.attrib = _A_SUBDIR;
	fHdl = _findfirst(fullpath,&fileinfo);

	lua_pushnumber(L, fHdl);
	lua_pushstring(L, fileinfo.name);

	return 2;
}

int LuaShell_Findfirst(lua_State* L)
{
	struct _finddata_t fileinfo;
	intptr_t fHdl;
	PCSTR filename = luaL_checkstring(L, 1);
	CHAR fullpath[FS_PATH_LEN] = {0};

	join_lroot(L, fullpath, (CHAR*)filename);

	memset(&fileinfo, 0, sizeof(fileinfo));
	fileinfo.attrib = _A_ARCH;
	fHdl = _findfirst(fullpath,&fileinfo);

	lua_pushnumber(L, fHdl);
	lua_pushstring(L, fileinfo.name);

	return 2;
}

int LuaShell_Findnext(lua_State* L)
{
	struct _finddata_t fileinfo;
	INT result;
	intptr_t fHdl = (intptr_t)luaL_checkinteger(L, 1);

	memset(&fileinfo, 0, sizeof(fileinfo));
	result = _findnext(fHdl,&fileinfo);

	lua_pushinteger(L, result);
	lua_pushstring(L, fileinfo.name);

	return 2;
}

int LuaShell_Findclose(lua_State* L)
{
	INT result;
	intptr_t fHdl = (intptr_t)luaL_checkinteger(L, 1);

	if(fHdl < 0)
	{
		result = -1;
	}
	else
	{
		result = _findclose(fHdl);
	}	

	lua_pushinteger(L, result);

	return 1;
}

int LuaShell_SetRoot(lua_State* L)
{
	CHAR *str;

	str  = (CHAR*)luaL_checkstring(L, 1);

	lua_setglobal(L, "_LROOT");

	return 0;
}

int LuaShell_MainLoop(lua_State* L)
{
	gL = L;

	platform_set_console_port(PLATFORM_UART_ID_ATC);

	platform_init();
	platform_uart_init();

	gDaemonThread = CreateThread(NULL, 
		DAEMON_THREAD_STACK_SIZE, 
		DaemonThreadProc,
		0, 0,
		&gDaemonThreadId);

	gLoadLuaThread = CreateThread(NULL,
		LOAD_LUA_TSTACK_SIZE, 
		LoadLuaThreadProc,
		0, 0,
		&gLoadLuaThreadId);


	gLuaShellThread = CreateThread(NULL,
		LUA_SHELL_TSTACK_SIZE, 
		LuaShellRoutine,
		0, 0,
		&gLuaShellThreadId);


	WaitForSingleObject(gLuaShellThread, INFINITE);

	LogWriter::LOGX("[Win32LuaShell] LuaShell_MainLoop Exit status=%d.",
		GetLastError());

	CloseHandle(gDaemonThread);
	CloseHandle(gLoadLuaThread);

	LogWriter::LOGX("[Win32LuaShell] LuaShell_MainLoop Exit.");

	return 0;
}

struct luaL_reg LuaShellList[] = {
	{"print",LuaShell_Print},
	{"makedir",LuaShell_Makedir},
	{"findfirstdir",LuaShell_Findfirstdir},
	{"findfirst",LuaShell_Findfirst},
	{"findnext",LuaShell_Findnext},
	{"findclose",LuaShell_Findclose},
	{"SetRoot",LuaShell_SetRoot},
	{"MainLoop",LuaShell_MainLoop},
	{NULL,NULL}
};


int luaopen_LuaShell(lua_State* L)
{
	luaL_register(L,"LuaShell",LuaShellList);

	lua_pushstring(L, "_VERSION");
	lua_pushstring(L, LUAT_VERSION);
	lua_rawset(L, -3);

	return 1;
}

static const luaL_Reg lualibs[] = {
	{"LuaShell",luaopen_LuaShell},
#if defined(LUA_PLATFORM_LIBS_ROM)
#define _ROM( name, openf, table ) { name, openf },
	LUA_PLATFORM_LIBS_ROM
#endif //LUA_PLATFORM_LIBS_ROM
	{NULL, NULL}
};

int luaopen_AMWatch(lua_State* L)
{
	int libs = sizeof(lualibs)/sizeof(lualibs[0]);
	const luaL_Reg *lib = lualibs;

	for (; lib->func != NULL; lib ++) {
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}

	LogWriter::LOGX("luaopen_AMWatch libs=%d.", libs);

	return libs;
}
