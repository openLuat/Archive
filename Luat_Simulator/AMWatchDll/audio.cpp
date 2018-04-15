/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    audio.cpp
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/10/21
 *
 * Description:
 *          audio.core
 *
 * History:
 *     panjun 2015.04.30 Add audio's API according to MTK.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_audio.h"

static int l_audio_play_file(lua_State *L) {
    const char *name = luaL_checkstring(L,1);
    AudioPlayParam param;

    param.isBuffer = FALSE;
    param.u.filename = name;

    lua_pushboolean(L, platform_audio_play(&param) == PLATFORM_OK);
    return 1;
}

static int l_audio_play_data(lua_State *L) {
    PCSTR data;
    int l;
    AudioPlayParam param;
    
    data = luaL_checklstring(L, 1, (size_t*)&l);
    param.isBuffer = TRUE;
    param.u.buffer.format = (PlatformAudioFormat)luaL_checkinteger(L, 2);
    param.u.buffer.loop = luaL_optinteger(L, 3, 0);
    param.u.buffer.data = data;
    param.u.buffer.len = l;
    
    lua_pushboolean(L, platform_audio_play(&param) == PLATFORM_OK);
    return 1;
}

static int l_audio_stop(lua_State *L) {
    platform_audio_stop();
    return 0;
}

/*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
static int l_audio_set_channel(lua_State *L) {
    UINT32 channel = luaL_checkinteger(L,1);
    UINT32 res;
    
    res = platform_audio_set_channel((PlatformAudioChannel)channel);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_vol(lua_State *L) {
    UINT32 vol = luaL_checkinteger(L,1);
    UINT32 res;

/*+\NEW\xiongjunqun\2015.05.28\修改无法切换到听筒模式问题*/	
    //platform_audio_set_channel(PLATFORM_AUD_CHANNEL_LOUDSPEAKER);
/*-\NEW\xiongjunqun\2015.05.28\修改无法切换到听筒模式问题*/
    res = platform_audio_set_vol((PlatformAudioVol)vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*+\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/
static int l_audio_set_sph_vol(lua_State *L) {
    UINT32 vol = luaL_checkinteger(L,1);
    UINT32 res;

    res = platform_audio_set_sph_vol((PlatformAudioVol)vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*-\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/

static int l_audio_speaker_set_vol(lua_State *L) {
    UINT32 vol = luaL_checkinteger(L,1);
    UINT32 res;

    platform_audio_set_channel(PLATFORM_AUD_CHANNEL_LOUDSPEAKER);
    res = platform_audio_set_vol((PlatformAudioVol)vol);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_mic_vol(lua_State *L) {
    UINT32 vol = luaL_checkinteger(L,1);
    UINT32 res;
    
    res = platform_audio_set_mic_vol((PlatformMicVol)vol);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_loopback(lua_State *L) {
    UINT32 flag = luaL_checkinteger(L,1);
    UINT32 typ = luaL_checkinteger(L,2);
    UINT32 setvol = luaL_checkinteger(L,3);
    UINT32 vol = luaL_checkinteger(L,4);
    UINT32 res;
    
    res = platform_audio_set_loopback(flag,(PlatformAudioLoopback)typ,setvol,vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/

static int l_audio_record(lua_State *L) {
    size_t  len      = 0;
    char* file_name      = (char*)luaL_checklstring(L, 1, &len);
    int     time_sec        = luaL_checkinteger(L, 2);
    int    quality        =  luaL_optint(L, 3, 0);
    int res = platform_audio_record(file_name, time_sec, quality);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_stop_record(lua_State *L) {
    int res = platform_audio_stop_record();
    lua_pushinteger(L, res);
    
    return 1;
}


#define MIN_OPT_LEVEL 2
// Module function map
const luaL_Reg audiocore_map[] =
{ 
  {"play",  l_audio_play_file},
  {"playdata",  l_audio_play_data},
  {"stop",  l_audio_stop},
  /*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
  {"setchannel",  l_audio_set_channel},
  {"setvol",  l_audio_set_vol},
/*+\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/  
  {"setsphvol",  l_audio_set_sph_vol},
/*-\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/  
  {"setspeakervol",  l_audio_speaker_set_vol},
  {"setmicvol",  l_audio_set_mic_vol},
  {"setloopback",  l_audio_set_loopback},
  /*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
  {"record",  l_audio_record},
  {"stoprecord",  l_audio_stop_record},

  { NULL, NULL }
};

LUALIB_API int luaopen_audiocore( lua_State *L )
{
    luaL_register( L, AUXLIB_AUDIOCORE, audiocore_map );

    MOD_REG_NUMBER(L, "AMR122", PLATFORM_AUD_AMR122);
    MOD_REG_NUMBER(L, "MP3", PLATFORM_AUD_MP3);
    MOD_REG_NUMBER(L, "PCM", PLATFORM_AUD_PCM);
    MOD_REG_NUMBER(L, "WAV", PLATFORM_AUD_WAV);
    MOD_REG_NUMBER(L, "MIDI", PLATFORM_AUD_MIDI);

    /*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
    #define REG_AUD_CHANNEL(CHANNEL) MOD_REG_NUMBER(L, #CHANNEL, PLATFORM_AUD_CHANNEL_##CHANNEL)
    REG_AUD_CHANNEL(HANDSET);
    REG_AUD_CHANNEL(EARPIECE);
    REG_AUD_CHANNEL(LOUDSPEAKER);
    REG_AUD_CHANNEL(BLUETOOTH);
    REG_AUD_CHANNEL(FM);
    REG_AUD_CHANNEL(FM_LP);
    REG_AUD_CHANNEL(TV);
    REG_AUD_CHANNEL(AUX_HANDSET);
    REG_AUD_CHANNEL(AUX_LOUDSPEAKER);
    REG_AUD_CHANNEL(AUX_EARPIECE);
    REG_AUD_CHANNEL(DUMMY_HANDSET);
    REG_AUD_CHANNEL(DUMMY_AUX_HANDSET);
    REG_AUD_CHANNEL(DUMMY_LOUDSPEAKER);    
    REG_AUD_CHANNEL(DUMMY_AUX_LOUDSPEAKER);

    #define REG_AUD_VOL(VOL) MOD_REG_NUMBER(L, #VOL, PLATFORM_AUD_##VOL)
    REG_AUD_VOL(VOL0);
    REG_AUD_VOL(VOL1);
    REG_AUD_VOL(VOL2);
    REG_AUD_VOL(VOL3);
    REG_AUD_VOL(VOL4);
    REG_AUD_VOL(VOL5);
    REG_AUD_VOL(VOL6);
    REG_AUD_VOL(VOL7);   

    #define REG_MIC_VOL(VOL) MOD_REG_NUMBER(L, #VOL, PLATFORM_##VOL)
    REG_MIC_VOL(MIC_VOL0);
    REG_MIC_VOL(MIC_VOL1);
    REG_MIC_VOL(MIC_VOL2);
    REG_MIC_VOL(MIC_VOL3);
    REG_MIC_VOL(MIC_VOL4);
    REG_MIC_VOL(MIC_VOL5);
    REG_MIC_VOL(MIC_VOL6);
    REG_MIC_VOL(MIC_VOL7); 
    REG_MIC_VOL(MIC_VOL8);
    REG_MIC_VOL(MIC_VOL9);
    REG_MIC_VOL(MIC_VOL10);
    REG_MIC_VOL(MIC_VOL11);
    REG_MIC_VOL(MIC_VOL12);
    REG_MIC_VOL(MIC_VOL13);
    REG_MIC_VOL(MIC_VOL14);
    REG_MIC_VOL(MIC_VOL15); 

    #define REG_AUD_LOOPBACK(TYPE) MOD_REG_NUMBER(L, #TYPE, PLATFORM_AUD_##TYPE)
    REG_AUD_LOOPBACK(LOOPBACK_HANDSET);
    REG_AUD_LOOPBACK(LOOPBACK_EARPIECE);
    REG_AUD_LOOPBACK(LOOPBACK_LOUDSPEAKER);
    REG_AUD_LOOPBACK(LOOPBACK_AUX_HANDSET);
    REG_AUD_LOOPBACK(LOOPBACK_AUX_LOUDSPEAKER);
    /*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
	
    return 1;
}  

