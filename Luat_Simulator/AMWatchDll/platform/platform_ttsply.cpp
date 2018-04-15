/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_ttsply.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          TTS Play API.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_ttsply.h"

#if defined(__AM_LUA_TTSPLY_SUPPORT__)

int platform_ttsply_init(ttsPlyParam *param)
{
   return PLATFORM_OK;
}

int platform_ttsply_initEngine(ttsPlyParam *param)
{
   return PLATFORM_OK;
}

void platform_ttsply_push_status(int ttsStatus)
{
   /*
   PlatformMsgData rtosmsg;

   rtosmsg.ttsPlyData.ttsPlyStatusInd = ttsStatus;
   platform_rtos_send(MSG_ID_RTOS_TTSPLY_STATUS, &rtosmsg);
   */
   PlatformMessage *message = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

   message->id = MSG_ID_RTOS_TTSPLY_STATUS;
   message->data.ttsPlyData.ttsPlyStatusInd = ttsStatus;

   platform_rtos_send(message);
}

void platform_ttsply_push_error(int ttsError)
{
   /*
   PlatformMsgData rtosmsg;

   rtosmsg.ttsPlyData.ttsPlyErrorInd = ttsError;
   platform_rtos_send(MSG_ID_RTOS_TTSPLY_ERROR, &rtosmsg);
   */

   PlatformMessage *message = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

   message->id = MSG_ID_RTOS_TTSPLY_ERROR;
   message->data.ttsPlyData.ttsPlyErrorInd = ttsError;

   platform_rtos_send(message);
}

int platform_ttsply_setParam(UINT16 playParam, INT16 value)
{
   return PLATFORM_OK;
}

int platform_ttsply_getParam(UINT16 playParam)
{
   return PLATFORM_OK;
}

int platform_ttsply_play(ttsPly *param)
{
    return PLATFORM_OK;
}

int platform_ttsply_pause(void)
{
    return PLATFORM_OK;
}

int platform_ttsply_stop(void)
{
    return PLATFORM_OK;
}

#endif //__AM_LUA_TTSPLY_SUPPORT__

