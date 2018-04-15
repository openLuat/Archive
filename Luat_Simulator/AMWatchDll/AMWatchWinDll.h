/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    AMWatchWinDll.h
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

#ifndef __AMWATCH_WIN_DLL_H__
#define __AMWATCH_WIN_DLL_H__

#include "Windows.h"
#include "Disp.h"
#include "AMWatchDll.h"

AMWATCH_DLL_API Disp WatchDisp;

AMWATCH_DLL_API void DispAMWatch(HWND hwnd, HINSTANCE hInstance);
AMWATCH_DLL_API void DispUpdate(void);
AMWATCH_DLL_API void DispDrawMainBitmap(HDC hdc);
AMWATCH_DLL_API void DispDrawrect(INT startx,INT starty,INT endx,INT endy,DWORD pColor);

#endif //__AMWATCH_WIN_DLL_H__
