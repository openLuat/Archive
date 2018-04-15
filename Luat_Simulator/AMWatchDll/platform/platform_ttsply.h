/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_ttsply.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          TTS Play API.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_TTSPLY_H__
#define __PLATFORM_TTSPLY_H__

typedef struct tagTtsPlyParam
{
    INT16    volume;
    INT16    speed;
    INT16    pitch;
    UINT16    codepage;
    UINT8      digit_mode;
    UINT8      punc_mode;
    UINT8      tag_mode;
    UINT8      wav_format;
    UINT8      eng_mode;
}ttsPlyParam;

typedef struct tagTtsPly
{
    WORD *text;
    UINT32 text_size;
    UINT8 spk_vol;
}ttsPly;

int platform_ttsply_init(ttsPlyParam *param);
int platform_ttsply_initEngine(ttsPlyParam *param);
int platform_ttsply_setParam(UINT16 plyParam, INT16 value);
int platform_ttsply_getParam(UINT16 plyParam);
int platform_ttsply_play(ttsPly *param);
int platform_ttsply_pause(void);
int platform_ttsply_stop(void);

#endif  //__PLATFORM_TTSPLY_H__

