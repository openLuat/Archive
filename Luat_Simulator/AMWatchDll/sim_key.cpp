/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    sim_key.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Simulate keypad's flow.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "platform.h"
#include "platform_rtos.h"
#include "sim_key.h"

static event_handle_t keypad_event = INVALID_EVENT_HANDLE;
static PlatformKeypadInitParam mKeypadInitParam;
static sim_key_struct sKey[MAX_KEY_SUPPORT] = {0};


void keypad_event_cb(UINT8* data, UINT32 length)
{
	PlatformMessage *message = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

    message->id = MSG_ID_RTOS_KEYPAD;
	keypad_is_press_bool = TRUE;
	if(data[0] != mKeypadInitParam.type)
	{
		LogWriter::LOGX("[sim_key] keypad matrix is unknown type!");
		return;
	}
    message->data.keypadMsgData.type = mKeypadInitParam.type;
    message->data.keypadMsgData.bPressed = data[1];
	if(data[2] == 0x0f)
		message->data.keypadMsgData.data.matrix.col = 0xff;
	else
		message->data.keypadMsgData.data.matrix.col = data[2] & mKeypadInitParam.matrix.inMask;
	if(data[3] == 0x0f)
		message->data.keypadMsgData.data.matrix.row = 0xff;
	else
		message->data.keypadMsgData.data.matrix.row = data[3] & mKeypadInitParam.matrix.outMask;

    platform_rtos_send(message);
}

void keypad_init(PlatformKeypadInitParam *param){
	mKeypadInitParam = *param;
    keypad_event = add_event("KEYPAD", EVENT_KEY, keypad_event_cb);
    send_event(keypad_event, (PCSTR)param, sizeof(PlatformKeypadInitParam));
	g_s_init_end_bool = TRUE;
}

void key_mapping_init(RECT *pos, key_mapping_struct *map)
{
	INT i;

	for(i = 0; i < MAX_KEY_SUPPORT; i ++)
	{
		memcpy(&sKey[i].location, pos++,sizeof(RECT));
        memcpy(&sKey[i].mapping, map++, sizeof(key_mapping_struct));
	}
}

INT key_mapping(UINT8 type, POINT p)
{
	INT i, result = PLATFORM_OK;
	CHAR data[4] = {0,0,0xfe,0xfe};
	sim_key_struct* key = sKey;


	data[1] = type;
	for(i=0; i < MAX_KEY_SUPPORT; i++)
	{
		if((p.x > key->location.left && p.x < key->location.right)
			&& (p.y > key->location.top && p.y < key->location.bottom))
		{
			data[2] = key->mapping.lua_key_code & 0x0f;
			data[3] = (key->mapping.lua_key_code & 0xf0) >> 4;
			break;
		}
		key ++;
	}

	if(i == MAX_KEY_SUPPORT)
	{
		result = PLATFORM_ERR;
	}
	else
	{
		daemon_emit_event(EVENT_KEY, data, sizeof(data));
	}
	
	return result;
}