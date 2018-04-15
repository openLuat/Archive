/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    disp.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Register all functions of 'Disp' into LUA's stack.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __AMWATCH_LUA_DLL_H__
#define __AMWATCH_LUA_DLL_H__

#include "lua.hpp"
#include "Windows.h"

#include "platform_disp.h"
#include "AMWatchDll.h"

AMWATCH_DLL_API Disp WatchDisp;

#endif //__AMWATCH_LUA_DLL_H__
