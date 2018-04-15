/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_audio.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Implement 'audio' class.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "platform.h"
#include "platform_rtos.h"
#include "platform_audio.h"

static UINT8 *audplaybuffer = NULL;

static BOOL is_recording = FALSE;

int platform_audio_play(AudioPlayParam *param)
{
    PlatformMessage *rtosmsg;
    MSG msg;
    
	rtosmsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));
    rtosmsg->id = RTOS_MSG_AUDIO;
    rtosmsg->data.audioData.playEndInd = TRUE;

    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)rtosmsg;
    SendToLoadLuaMessage(&msg);
    return PLATFORM_OK;
}

int platform_audio_stop(void)
{
    return PLATFORM_OK;
}

int platform_audio_set_channel(PlatformAudioChannel channel)
{
    return PLATFORM_OK;
}

int platform_audio_set_vol(PlatformAudioVol vol)
{
    return PLATFORM_OK;
}

int platform_audio_set_sph_vol(PlatformAudioVol vol)
{
    return PLATFORM_OK;
}

int platform_audio_set_mic_vol(PlatformMicVol vol)
{
    return PLATFORM_OK;
}


int platform_audio_set_loopback(BOOL flag, PlatformAudioLoopback typ, BOOL setvol, UINT32 vol)
{
    return PLATFORM_OK;
}

int platform_audio_record(char* file_name, int time_sec, int quality)
{
	if(is_recording)
	{
		return PLATFORM_ERR;
	}

	is_recording = TRUE;

	return PLATFORM_OK;
}

int platform_audio_stop_record(void)
{
	is_recording = FALSE;

	return PLATFORM_OK;
}
