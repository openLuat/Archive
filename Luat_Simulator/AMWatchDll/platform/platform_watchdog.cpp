/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_watchdog.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Watchdog.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_watchdog.h"

int platform_watchdog_open(watchdog_info_t *info){
    return PLATFORM_ERR;
}

int platform_watchdog_close(void){
    return PLATFORM_ERR;
}

int platform_watchdog_kick(void){
    return PLATFORM_ERR;
}
