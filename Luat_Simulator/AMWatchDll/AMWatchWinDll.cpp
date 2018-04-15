/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    AMWatchWinDll.cpp
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
#include "AMWatchWinDll.h"

void DispAMWatch(HWND hwnd, HINSTANCE hInstance)
{
	Disp::GetDisp()->DispInit(hwnd,hInstance);
	Disp::GetDisp()->w32_screen_init(hwnd, hInstance);
}

void DispUpdate(void)
{
	Disp::GetDisp()->w32_update_screen();
}

void DispDrawrect(INT startx, INT starty,
				  INT endx, INT endy,
				  DWORD pColor)
{
	Disp::GetDisp()->drawrect(startx,starty,endx,endy,pColor);
}

void DispDrawMainBitmap(HDC hdc)
{
	Disp::GetDisp()->w32_draw_main_bitmap(hdc);
}

