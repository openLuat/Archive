/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_SLI3108.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          SLI3108.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_SLI3108.h"

#ifdef AM_SLI3108_SUPPORT

void platform_SLI3108_wear_status_callback(void* param)
{
	/*
    SLI3108_STATUS sli3108_status = (SLI3108_STATUS)param;
    PlatformMsgData rtosmsg;

    rtosmsg.wearStatusData.wearStatus = (sli3108_status == SLI3108_STATUS_HIGH);
    platform_rtos_send(MSG_ID_RTOS_WEAR_STATUS, &rtosmsg);
	*/

	SLI3108_STATUS* sli3108_status = (SLI3108_STATUS*)param;
	PlatformMessage *message = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

	message->id = MSG_ID_RTOS_WEAR_STATUS;
	message->data.wearStatusData.wearStatus = (*sli3108_status == SLI3108_STATUS_HIGH);

	platform_rtos_send(message);
}


void platform_SLI3108_init(void)
{
	//TO DO ...
}


void platform_SLI3108_set_work_mode(BOOL factory_mode)
{
	//TO DO ...
}

void platform_SLI3108_open(void)
{
  //TO DO ...
}

void platform_SLI3108_close(void)
{
  //TO DO ...
}

#endif //AM_SLI3108_SUPPORT
