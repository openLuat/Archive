/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    dllmain.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          The entry point for the DLL application.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __DLLMAIN_H__
#define __DLLMAIN_H__

#include "daemon.h"
#include "platform_rtos.h"
#include "platform_wtimer.h"
#include "disp.h"


void destroy_events(void);
DWORD WINAPI WinSimlaMain(LPVOID lpParameter);  
LRESULT CALLBACK WinSimlaProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WinSimlaCreate(HINSTANCE hInst, HWND hwnd);
void WinSimlaClose(HINSTANCE hInst, HWND hwnd);
void WinSimlaAbnormalExit(PCSTR msg);
void WinSimlaCmdProc(HINSTANCE hInst, HWND hwnd, WORD nCmdMenu);
void WinSimlaImitatedInputProc(HWND hwnd, UINT wmType, LPARAM lParam);
void WinSimlaUpdateProc(HDC hDC);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif //__DLLMAIN_H__