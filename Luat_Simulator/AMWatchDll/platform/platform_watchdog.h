/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_watchdog.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Watchdog.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_WATCHDOG_H__
#define __PLATFORM_WATCHDOG_H__

#define WATCHDOG_DEFAULT_MODE       0

typedef struct{
    int     mode;
    union{
        int pin_ctl;
    }param;
}watchdog_info_t;

int platform_watchdog_open(watchdog_info_t *info);

int platform_watchdog_close(void);

int platform_watchdog_kick(void);

#endif //__PLATFORM_WATCHDOG_H__
