/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    dllmain.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          The entry point for the DLL application.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "dllmain.h"
#include "platform_atc.h"
#include "AMWatchWinDll.h"
#include "Win32LuaShell.h"

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR tszModulePath[MAX_PATH];
HANDLE mutex_handle;
WinTimer* mWinTimer = NULL;
Disp* mWatchDisp = NULL;

HINSTANCE gInstance;  
HANDLE dllMainThread;
HANDLE handleSemaphore;

BOOL WINAPI DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		GetModuleFileName(NULL, tszModulePath, MAX_PATH);	
		for(int i = _tcslen(tszModulePath) - 1; i > 0; i--)
		{
			if(tszModulePath[i] == '\\' )
				tszModulePath[i] = '/' ;
		}
		mutex_handle = CreateMutex(NULL, 0, tszModulePath);
		if(ERROR_ALREADY_EXISTS == GetLastError() && mutex_handle!=NULL )
		{
			SetConsoleTitle("Multi-instance of one and the same AMLuaDebug is not supported");
			CloseHandle(mutex_handle);
			return 0 ;
		}

		handleSemaphore=CreateSemaphore(NULL,1, 2,"thread");

		gInstance = (HINSTANCE)hModule;
		LoadString(gInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(gInstance, IDS_AMLUADEBUG, szWindowClass, MAX_LOADSTRING);
		dllMainThread = CreateThread(NULL,SIMLA_MAIN_THREAD_STACK_SIZE,WinSimlaMain,NULL,CREATE_SUSPENDED,NULL);
		SetThreadPriority(dllMainThread,THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(dllMainThread);
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if(mutex_handle != NULL)
		{
			CloseHandle(mutex_handle);
		}
		LogWriter::GetLogWriter()->DeleteLogWriter();
		break;
	}
	return TRUE;
}

DWORD WINAPI WinSimlaMain(LPVOID lpParameter)  
{
	WNDCLASSEX wndcls;
	HWND hwnd;

	wndcls.cbSize = sizeof(WNDCLASSEX);
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.cbClsExtra = 0;  
	wndcls.cbWndExtra = 0;
	wndcls.hInstance = (HINSTANCE)gInstance;
	wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndcls.hCursor = LoadCursor(NULL, IDC_HAND);
	wndcls.hIcon = LoadIcon(wndcls.hInstance, MAKEINTRESOURCE(IDI_AMWATCHDLL));
	wndcls.lpfnWndProc = WinSimlaProc;  
	wndcls.lpszClassName = szWindowClass;
	wndcls.lpszMenuName = MAKEINTRESOURCE(IDR_AMWATCH_MMENU);
	wndcls.hIconSm = LoadIcon(wndcls.hInstance, MAKEINTRESOURCE(IDI_AMWATCHDLL_SMALL));

	RegisterClassEx(&wndcls);

	hwnd = CreateWindow(szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 240,
		NULL,
		NULL,
		gInstance, NULL);


	if (hwnd == NULL)
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL );
		MessageBox(NULL, (LPTSTR)lpMsgBuf, "hwnd", MB_OK);
		PostThreadMessage(gDaemonThreadId, WM_QUIT, NULL, NULL);
		PostThreadMessage(gLoadLuaThreadId, WM_QUIT, NULL, NULL);
		PostThreadMessage(gLuaShellThreadId, WM_QUIT, NULL, NULL);
		CloseHandle(dllMainThread);
		return FALSE;
	}


	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);  


	MSG msg;  
	while(GetMessage(&msg, hwnd, 0, 0))
	{  
		TranslateMessage(&msg);  
		DispatchMessage(&msg);  
	}  
	return 0;  
}  

LRESULT CALLBACK WinSimlaProc(HWND hWnd,  UINT uMsg,  
							WPARAM wParam, LPARAM lParam)  
{
	MSG msg;
	HINSTANCE hInstance;

	switch(uMsg)
	{
	case WM_CREATE:
		hInstance = (HINSTANCE)gInstance;

        WinSimlaCreate(hInstance, hWnd);
		PostThreadMessage(gLoadLuaThreadId, WM_LUA_MAIN, NULL, NULL);

		LogWriter::LOGX("[dllmain] WinSimlaProc WM_CREATE hWnd=%x,hInstance=%x.",hWnd,hInstance);
		break;

	case WM_LUA_MAIN:
		LogWriter::LOGX("[dllmain] WinSimlaProc WM_LUA_MAIN.");
		break;

	case WM_CHAR:
		break;

	case WM_TIMER:
		if(wParam == DAEMON_TIMER_ID)
		{
			msg.message = DAEMON_TIMER_MSG_ID;
			msg.wParam = 0;
			msg.lParam = 0;

			SendToLoadLuaMessage(&msg);
		}
		else
		{
			if(!WinTimer::GetWinTimer()->win_timeout(wParam))
			{
				LogWriter::LOGX("[dllmain] WinSimlaProc timeout id %x.", wParam);            
			}
		}
		break;

	case WM_LBUTTONDOWN:
		WinSimlaImitatedInputProc(hWnd,uMsg,lParam);
		break;

	case WM_LBUTTONUP:
		WinSimlaImitatedInputProc(hWnd,uMsg,lParam);
		break;

	case WM_MOUSEMOVE:
		WinSimlaImitatedInputProc(hWnd,uMsg,lParam);
		break;

	case WM_PAINT:  
		HDC hDC;  
		PAINTSTRUCT ps;  
		hDC=BeginPaint(hWnd,&ps);
		WinSimlaUpdateProc(hDC);
		EndPaint(hWnd,&ps);  
		break; 

	case WM_CLOSE:  
		if(IDYES==MessageBox(hWnd,
			"½áÊøLUAµ÷ÊÔÂð?",
			szTitle,
			MB_YESNO))  
		{  
			WinSimlaClose((HINSTANCE)gInstance, hWnd);
		}  
		break;  

	case WM_DESTROY:  
		PostQuitMessage(0);  
		break;

	case WM_COMMAND:
		if(lParam == 0)
		{
			WinSimlaCmdProc((HINSTANCE)gInstance, hWnd, LOWORD(wParam));
		}		
		break;

	default:  
		return DefWindowProc(hWnd,uMsg,wParam,lParam);  
	}

	return TRUE;
}

void WinSimlaCreate(HINSTANCE hInst, HWND hwnd)
{
	LogWriter::LOGX("[dllmain] WinSimlaCreate WinTimer=%x,Disp=%x.",
		mWinTimer,Disp::GetDisp());

	startRtosSimulator = TRUE;

	if(mWinTimer == NULL)
	{
		mWinTimer = new WinTimer(hwnd);
		mWinTimer->SetWinTimer(mWinTimer);
		WinTimer::GetWinTimer()->win_start_timer(0, 100);
	}
	
	if(Disp::GetDisp() == NULL) {
		LogWriter::LOGX("[dllmain] WinSimlaCreate Disp == NULL.");
		mWatchDisp = new Disp(hwnd, hInst);
		mWatchDisp->SetDisp(mWatchDisp);
		Disp::GetDisp()->DispInit(hwnd, hInst);
	} else {
		Disp::GetDisp()->DispInit(hwnd, hInst);
	}
	Disp::GetDisp()->w32_screen_init(hwnd, hInst);
}

void WinSimlaClose(HINSTANCE hInst, HWND hwnd)
{
	startRtosSimulator = FALSE;

	destroy_events();
	platform_uart_close_all();

	PostThreadMessage(gDaemonThreadId, WM_QUIT, NULL, NULL);
	PostThreadMessage(gLoadLuaThreadId, WM_QUIT, NULL, NULL);
	PostThreadMessage(gLuaShellThreadId, WM_QUIT, NULL, NULL);
	if(WinTimer::GetWinTimer() != NULL)
	{
		WinTimer::GetWinTimer()->DeleteWinTimer();
	}
	if(Disp::GetDisp() != NULL)
	{
		Disp::GetDisp()->~Disp();
	}
	Sleep(1000*3);
	CloseHandle(dllMainThread);
	DestroyWindow(hwnd);
}

void WinSimlaCmdProc(HINSTANCE hInst, HWND hwnd, WORD nCmdMenu)
{
	switch(nCmdMenu)
	{
	case IDM_FILE_EXIT:
		WinSimlaClose(hInst, hwnd);
		break;

	case IDM_TOOL_SUSPEND:
		break;

	case IDM_TOOL_RESUME:
		break;

	case IDM_TOOL_SIMINSERT:
		platform_sim_insert();
		break;

	case IDM_TOOL_SIMREMOVE:
		platform_sim_remove();
		break;

	case IDM_HELP_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	}
}

void WinSimlaImitatedInputProc(HWND hwnd, UINT wmType, LPARAM lParam)
{
	POINT p;

	p.x=LOWORD(lParam);
	p.y=HIWORD(lParam);

	Disp::GetDisp()->w32_imitated_input(wmType, p);
}

void WinSimlaUpdateProc(HDC hDC)
{
	LogWriter::LOGX("[dllmain] WinSimlaUpdateProc DispRun=%d.",Disp::GetDispRun());

	if(Disp::GetDispRun())
	{
		Disp::GetDisp()->w32_draw_main_bitmap(hDC);
		Disp::GetDisp()->w32_update_screen();
	}
	else
	{
		LogWriter::LOGX("[dllmain] WinSimlaUpdateProc Disp don't run!");
	}

}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

