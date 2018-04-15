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

#ifndef __PLATFORM_WTIMER_H__
#define __PLATFORM_WTIMER_H__

#define DAEMON_TIMER_ID         0x1234
#define DAEMON_TIME_OUT         (1000)

#define MAX_WIN_TIMERS          (40)
#define TIMER_ID_BASE           (0x2000)

#define SOC_TIMER_ID (0x1000)
#define NBR_TIMER_TIMEOUT (500)
#define NBR_TIMER_ID (0x1001)

typedef struct WinTimerInfoTag
{
	int	user_id;
	int	period;
	int	start_period;
	int	win_timer_id;
}WinTimerInfo;

class WinTimer
{
public:
	WinTimer();
	WinTimer(HWND hwnd);
	~WinTimer();
	int win_start_timer(int timer_id, int milliSecond);
	int win_stop_timer(int timerId);
	BOOL win_timeout(int timerId);
	int start_soc_timer(int milliSecond);
	int start_nbr_timer(int milliSecond);
	void stop_soc_timer(void);
	void stop_nbr_timer(void);

	void SetWinTimer(WinTimer *wTimer);
	static WinTimer* GetWinTimer();
	static void DeleteWinTimer();

protected:

private:
	HWND hDaemonWnd;
	WinTimerInfo socTimer;
	WinTimerInfo nbrTimer;
	WinTimerInfo winTimer[MAX_WIN_TIMERS];
	short global_timer_id_unique;
	HANDLE hTimerSem;
};

extern void soc_polling_timer_expiry(void);
extern void srv_on_nbr_info(void);

#endif //__PLATFORM_WTIMER_H__