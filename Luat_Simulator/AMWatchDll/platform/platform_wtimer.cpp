/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_wtimer.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'w_timer' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/
 
#include "stdafx.h"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_wtimer.h"

WinTimer *WinTimerInst = NULL;

WinTimer::WinTimer()
{
	hDaemonWnd = NULL;
	global_timer_id_unique = 0;
	memset(&winTimer, 0, sizeof(winTimer));
	hTimerSem = NULL;

	for(INT i = 0; i < MAX_WIN_TIMERS; i++)
	{
		winTimer[i].period = 0;
		winTimer[i].start_period = 0;
		winTimer[i].user_id = 0;
		winTimer[i].win_timer_id = 0;
	}

	memset(&socTimer, 0, sizeof(socTimer));
	socTimer.period = -1;
	socTimer.start_period = -1;
	socTimer.user_id = -1;
	socTimer.win_timer_id = SOC_TIMER_ID;

	memset(&nbrTimer, 0, sizeof(nbrTimer));
	nbrTimer.period = -1;
	nbrTimer.start_period = -1;
	nbrTimer.user_id = -1;
	nbrTimer.win_timer_id = NBR_TIMER_ID;
	LogWriter::LOGX("[WinTimer] WinTimer.");
}

WinTimer::WinTimer(HWND hwnd)
{
	hDaemonWnd = hwnd;
	global_timer_id_unique = 0;
	memset(&winTimer, 0, sizeof(winTimer));
	hTimerSem = CreateSemaphore(NULL, 1, 1, NULL);

	for(INT i = 0; i < MAX_WIN_TIMERS; i++)
	{
		winTimer[i].period = 0;
		winTimer[i].start_period = 0;
		winTimer[i].user_id = 0;
		winTimer[i].win_timer_id = 0;
	}

	memset(&socTimer, 0, sizeof(socTimer));
	socTimer.period = -1;
	socTimer.start_period = -1;
	socTimer.user_id = -1;
	socTimer.win_timer_id = SOC_TIMER_ID;

	memset(&nbrTimer, 0, sizeof(nbrTimer));
	nbrTimer.period = -1;
	nbrTimer.start_period = -1;
	nbrTimer.user_id = -1;
	nbrTimer.win_timer_id = NBR_TIMER_ID;
	LogWriter::LOGX("[WinTimer] WinTimer: hwnd=%x.",hwnd);
}

WinTimer::~WinTimer()
{
	if(hTimerSem != NULL)CloseHandle(hTimerSem);
	LogWriter::LOGX("[WinTimer] ~WinTimer");
}

int WinTimer::win_start_timer(int timer_id, int milliSecond)
{
	int index;
	int ret = PLATFORM_ERR;
	int winTimerId = 0;

	if(hDaemonWnd == NULL)
	{
		LogWriter::LOGX("[WinTimer] win_start_timer: hWnd==NULL.");
		return ret;
	}

	for(index = 0; index < MAX_WIN_TIMERS; index++)
	{
		if( winTimer[index].start_period != 0 && \
			WinUtil::ust_get_current_time() - winTimer[index].start_period > winTimer[index].period)
		{
			KillTimer(hDaemonWnd, winTimer[index].win_timer_id);
			memset(&winTimer[index], 0, sizeof(winTimer[index]));
		}

		if(winTimer[index].user_id == timer_id)
		{
			KillTimer(hDaemonWnd, winTimer[index].win_timer_id);
			memset(&winTimer[index], 0, sizeof(winTimer[index]));
			break;
		}
	}

	for(index = 0; index < MAX_WIN_TIMERS; index++)
	{
		if(winTimer[index].win_timer_id == 0)
		{
			if(global_timer_id_unique > 30000)
			{
				global_timer_id_unique = 0;
			}
			winTimer[index].win_timer_id = winTimerId = TIMER_ID_BASE + (++global_timer_id_unique);
			winTimer[index].user_id = timer_id;
			break;
		}
	}

	LogWriter::LOGX("[WinTimer] win_start_timer: index=%d,timer_id=%d,winTimerId=%d,hDaemonWnd=%x.",
		index,timer_id,winTimerId,hDaemonWnd);

	if(index < MAX_WIN_TIMERS)    
	{
		winTimer[index].period = milliSecond;
		winTimer[index].start_period = WinUtil::ust_get_current_time();
		SetTimer(hDaemonWnd, winTimerId, milliSecond, NULL);
		ret = PLATFORM_OK;
	}
	else
	{
		LogWriter::LOGX("[WinTimer] win_start_timer: no timer resource.");
		ret = PLATFORM_ERR;
	}

	return ret;
}

int WinTimer::win_stop_timer(int timerId)
{
	int index;

	if(hDaemonWnd == NULL)
	{
		LogWriter::LOGX("[WinTimer] win_stop_timer: hWnd==NULL.");
		return PLATFORM_ERR;
	}

	for(index = 0; index < MAX_WIN_TIMERS; index++)
	{
		if(winTimer[index].user_id == timerId)
		{
			break;
		}
	}

	if(index < MAX_WIN_TIMERS)    
	{
		KillTimer(hDaemonWnd, winTimer[index].win_timer_id);
		memset(&winTimer[index], 0, sizeof(winTimer[index]));
	}
	else
	{
		LogWriter::LOGX("[WinTimer] win_stop_timer: Don't find timer.");
	}

	return PLATFORM_OK;
}

BOOL WinTimer::win_timeout(int timerId)
{
	int index;
	MSG msg;
	int user_id;

	if(hDaemonWnd == NULL)
	{
		LogWriter::LOGX("[WinTimer] win_timeout: hWnd==NULL.");
		return FALSE;
	}

	LogWriter::LOGX("[WinTimer] win_timeout: timerId=%d.",timerId);


	if(timerId == socTimer.win_timer_id)
	{
		stop_soc_timer();
		soc_polling_timer_expiry();
		return TRUE;
	}
	else if(timerId == nbrTimer.win_timer_id)
	{
		start_nbr_timer(NBR_TIMER_TIMEOUT);
		srv_on_nbr_info();
		return TRUE;
	}

	for(index = 0; index < MAX_WIN_TIMERS; index++)
	{
		if(winTimer[index].win_timer_id == timerId)
		{
			user_id = winTimer[index].user_id;
			break;
		}
	}

	KillTimer(hDaemonWnd, timerId);
	memset(&winTimer[index], 0, sizeof(winTimer[index]));


	msg.message = MSG_ID_RTOS_TIMER;
	msg.wParam = user_id;
	msg.lParam = 0;

	SendToLoadLuaMessage(&msg);

	return TRUE;
}

WinTimer* WinTimer::GetWinTimer()
{
	if(WinTimerInst == NULL)
	{
		LogWriter::LOGX("[WinTimer] GetWinTimer: WinTimerInst == NULL.");
		return NULL;
	}

	if(WinTimerInst->hDaemonWnd == NULL)
	{
		LogWriter::LOGX("[WinTimer] GetWinTimer: hDaemonWnd == NULL.");
		return NULL;
	}
	
	return WinTimerInst;
}

void WinTimer::SetWinTimer(WinTimer *wTimer)
{
	if(WinTimerInst == NULL)
	{
		WinTimerInst = wTimer;
	}
	else
	{
		LogWriter::LOGX("[WinTimer] SetWinTimer: Repeatedly Create 'WinTimer'.");
	}
}

void WinTimer::DeleteWinTimer()
{
	if(WinTimerInst != NULL)
	{
		CloseHandle(GetWinTimer()->hTimerSem);
		WinTimerInst = NULL;
	}
}

int WinTimer::start_soc_timer(int milliSecond)
{
	stop_soc_timer();
	socTimer.period = milliSecond;
	socTimer.start_period = WinUtil::ust_get_current_time();
	SetTimer(hDaemonWnd, socTimer.win_timer_id, milliSecond, NULL);

	return PLATFORM_OK;
}

void WinTimer::stop_soc_timer(void)
{
	if(socTimer.period == -1 && socTimer.start_period == -1) return;
	KillTimer(hDaemonWnd, socTimer.win_timer_id);
	socTimer.period = -1;
	socTimer.start_period = -1;
}

int WinTimer::start_nbr_timer(int milliSecond)
{
	stop_nbr_timer();
	nbrTimer.period = milliSecond;
	nbrTimer.start_period = WinUtil::ust_get_current_time();
	SetTimer(hDaemonWnd, nbrTimer.win_timer_id, milliSecond, NULL);

	return PLATFORM_OK;
}

void WinTimer::stop_nbr_timer(void)
{
	if(nbrTimer.period == -1 && nbrTimer.start_period == -1) return;
	KillTimer(hDaemonWnd, nbrTimer.win_timer_id);
	nbrTimer.period = -1;
	nbrTimer.start_period = -1;
}