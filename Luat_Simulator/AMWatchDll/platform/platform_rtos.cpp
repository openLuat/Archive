/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_rtos.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'RTOS' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/
#include "stdafx.h"
#include "platform.h"
#include "sim_key.h"
#include "sim_tp.h"
#include "disp.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "Win32LuaShell.h"

#define RTOS_WAIT_MSG_TIMER_ID          (0xffffffff)
#define SEND_SOUND_END_TIMER_ID         (-2)

BOOL startRtosSimulator = FALSE;
BOOL keypad_is_press_bool = FALSE;
BOOL g_s_init_end_bool = FALSE;

extern void platform_sendsound_end(void);

static   CHAR   base64_table[]   =       
{
	'A',   'B',   'C',   'D',   'E',   'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',       
	'N',   'O',   'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',   'Z',       
	'a',   'b',   'c',   'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',       
	'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',   'x',   'y',   'z',       
	'0',   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '+',   '/',   '\0'       
};

void SendToLoadLuaMessage(const MSG *msg) 
{
	if(!startRtosSimulator)
	{
		LogWriter::LOGX("[SendToLoadLuaMessage]: lost msg(%d)", msg->message);
		return;
	}

	LogWriter::LOGX("[platform_rtos] SendToLoadLuaMessage(message) : %d",msg->message);

	PostThreadMessage(gLoadLuaThreadId, msg->message, msg->wParam, msg->lParam);
}

void SendToLuaShellMessage(const MSG *msg) 
{
	if(!startRtosSimulator)
	{
		LogWriter::LOGX("[SendToLuaShellMessage]: lost msg(%d)", msg->message);
		return;
	}

	LogWriter::LOGX("[platform_rtos] SendToLuaShellMessage(message) : %d",msg->message);

	PostThreadMessage(gLuaShellThreadId, msg->message, msg->wParam, msg->lParam);
}

int platform_rtos_send(PlatformMessage *pMsg){
	MSG msg;

	msg.message = SIMU_RTOS_MSG_ID;
	msg.wParam = (WPARAM)pMsg;
	SendToLoadLuaMessage(&msg);

	return PLATFORM_OK;
}

int platform_rtos_receive(msg_type* msg_id, void **ppMessage, UINT32 timeout)
{
	MSG msg;
	PlatformMessage *platform_msg;
	int ret = PLATFORM_OK;
	int result;

	LogWriter::LOGX("[platform_rtos] platform_rtos_receive msg_id=%d,timeout=%d,startRtosSimulator=%d.",
		*msg_id,timeout,startRtosSimulator);

	if(timeout != 0)
	{
		WinTimer::GetWinTimer()->win_start_timer(RTOS_WAIT_MSG_TIMER_ID, timeout);
	}

	/* 第一次调用该接口才允许其他线程发送消息 */
	if(!startRtosSimulator)
	{
		startRtosSimulator = TRUE;
	}

	while(1)
	{
		LogWriter::LOGX("[platform_rtos] platform_rtos_receive[GetMessage...]");

		result = GetMessage(&msg, (HWND)-1, 0, 0);
		if(result == -1)
		{
			//TO DO ...
		}
		*msg_id = msg.message;

		LogWriter::LOGX("[platform_rtos] platform_rtos_receive message=%d,wParam=%d",
			msg.message,msg.wParam);

		if(msg.message == DAEMON_TIMER_MSG_ID)
		{
			static UINT8 count = 0;

			if(10 > count)
				continue;

			platform_msg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

			if(count == 0)
			{
				platform_msg->id = RTOS_MSG_PMD;
			}
			else if(count >= 1 && count <= 4)
			{
				platform_msg->id = RTOS_MSG_KEYPAD;
				platform_msg->data.keypadMsgData.bPressed = count%2;
				platform_msg->data.keypadMsgData.data.matrix.row = count%5;
				platform_msg->data.keypadMsgData.data.matrix.col = count%3;
			}
			else
			{
				platform_msg->id = RTOS_MSG_UART_RX_DATA;
				platform_msg->data.uart_id = PLATFORM_UART_ID_ATC;
				if(count == 5)
				{
					platform_msg->data.uart_id = 1;
				}
			}

			*ppMessage = platform_msg;

			count++;

			break;
		}
		else if(msg.message == MSG_ID_RTOS_TIMER)
		{
			platform_msg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

			if(msg.wParam == RTOS_WAIT_MSG_TIMER_ID)
			{
				platform_msg->id = RTOS_MSG_WAIT_MSG_TIMEOUT;
			}
			else if(msg.wParam == SEND_SOUND_END_TIMER_ID)
			{
				platform_msg->id = RTOS_MSG_UART_RX_DATA;
				platform_msg->data.uart_id = PLATFORM_UART_ID_ATC;
                platform_sendsound_end();
			}
			else
			{
				platform_msg->id = RTOS_MSG_TIMER;
				platform_msg->data.timer_id = msg.wParam;
			}

			*ppMessage = platform_msg;

			break;
		}
		else if(msg.message == SIMU_UART_ATC_RX_DATA)
		{
			platform_msg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

                     /*
			platform_msg->id = RTOS_MSG_UART_RX_DATA;
			platform_msg->data.uart_id = PLATFORM_UART_ID_ATC;
			*/
			memcpy(platform_msg, (void*)msg.wParam, sizeof(PlatformMessage));

			*ppMessage = platform_msg;
			break;
		}
		else if(msg.message == SIMU_RTOS_MSG_ID)
		{
			platform_msg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));
			memcpy(platform_msg, (void*)msg.wParam, sizeof(PlatformMessage));
			*msg_id = platform_msg->id;

			LogWriter::LOGX("[platform_rtos] platform_rtos_receive(platform_msg) id: %d",platform_msg->id);

			WinUtil::L_FREE((void*)msg.wParam);

			platform_msg->id = msg.lParam;
			*ppMessage = platform_msg;
			break;
		}
	}

	return ret;
}

void platform_rtos_free_msg(void *msg_body)
{
	WinUtil::L_FREE(msg_body);    
}

int platform_rtos_start_timer(int timer_id, int milliSecond)
{
	return WinTimer::GetWinTimer()->win_start_timer(timer_id, milliSecond);
}

int platform_rtos_stop_timer(int timerId)
{
	return WinTimer::GetWinTimer()->win_stop_timer(timerId);
}

int platform_rtos_init_module(int module, void *pParam)
{
	switch(module){
	case RTOS_MODULE_ID_KEYPAD:
		keypad_init((PlatformKeypadInitParam*)pParam);    
		break;

	case RTOS_MODULE_ID_ALARM:
		break;

	case RTOS_MODULE_ID_TOUCH:
		tp_init();
		break;
	}
	return PLATFORM_OK;
}

int platform_rtos_init(void)
{
	return PLATFORM_OK;
}

int platform_rtos_poweroff(void)
{
	LogWriter::LOGX("[platform_rtos] platform_rtos_poweroff");
	Disp::GetDisp()->poweroff();
	return PLATFORM_OK;
}

int platform_rtos_tick(void)
{
	return GetTickCount();
}

int platform_rtos_sms_is_ready(void)
{
    return FALSE;
}

int platform_rtos_restart(void)
{
	return PLATFORM_OK;
}

int platform_get_poweron_reason(void)
{
	return PLATFORM_POWERON_KEY;
}

int platform_rtos_poweron(int flag)
{
	return PLATFORM_OK;
}

int platform_rtos_repoweron(void)
{
    LogWriter::LOGX("platform_rtos_repoweron");
   
    return PLATFORM_OK;
}

void platform_poweron_try(void)
{

}

int platform_rtos_setalarm(void *pParam)
{
	return PLATFORM_OK;
}

char*  platform_base64_encode(const char *str, size_t length)
{
	static char  base64_pad = '=';       
	const char *current = (const   char*)str;     
	int i = 0;       
	char *result = (CHAR *)WinUtil::L_MALLOC(((length + 3 - length % 3) * 4 / 3 + 1) * sizeof(char)); 

	while (length > 2) 
	{ 
		/*   keep   going   until   we   have   less   than   24   bits   */     
		result[i++] = base64_table[current[0] >> 2];   
		result[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];       
		result[i++] = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];       
		result[i++] = base64_table[current[2] & 0x3f];       
		current += 3;       
		length -= 3;   /*   we   just   handle   3   octets   of   data   */       
	} 

	/*   now   deal   with   the   tail   end   of   things   */       
	if   (length != 0)  
	{       
		result[i++] = base64_table[current[0] >> 2]; 

		if   (length > 1) 
		{       
			result[i++] = base64_table[((current[0] & 0x03 ) << 4) + (current[1] >> 4)];       
			result[i++] = base64_table[(current[1] & 0x0f) << 2];   
			result[i++] = base64_pad;     
		}     
		else   
		{     
			result[i++] = base64_table[(current[0] & 0x03) << 4];     
			result[i++] = base64_pad;     
			result[i++] = base64_pad;     
		}     
	}    

	result[i] = '\0';   //   printf("%s/n",result);     
	return result;     
}

char* platform_base64_decode (const char *src, int length, int* decodedLen) 
{
	INT i = 0;
	INT j = 0;
	INT l = 0;
	INT size = 0;
	char *dec = NULL;
	char buf[3];
	char tmp[4];

	if(!src)
	{
		return NULL;
	}
	dec = (CHAR*)WinUtil::L_MALLOC(((length+3)/4)*3 + 1);
	if (NULL == dec) 
	{
		return NULL;
	}

	// parse until end of source
	while (length--) {
		// break if char is `=' or not base64 char
		if ('=' == src[j]) { break; }
		if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j])) { break; }

		// read up to 4 bytes at a time into `tmp'
		tmp[i++] = src[j++];

		// if 4 bytes read then decode into `buf'
		if (4 == i) {
			// translate values in `tmp' from table
			for (i = 0; i < 4; ++i) {
				// find translation char in `b64_table'
				for (l = 0; l < 64; ++l) {
					if (tmp[i] == base64_table[l]) {
						tmp[i] = l;
						break;
					}
				}
			}

			// decode
			buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
			buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
			buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

			// write decoded buffer to `dec'
			//dec = (unsigned char *) tls_mem_realloc(dec, size + 3);
			for (i = 0; i < 3; ++i) {
				dec[size++] = buf[i];
			}

			// reset
			i = 0;
		}
	}

	// remainder
	if (i > 0) {
		// fill `tmp' with `\0' at most 4 times
		for (j = i; j < 4; ++j) {
			tmp[j] = '\0';
		}

		// translate remainder
		for (j = 0; j < 4; ++j) {
			// find translation char in `b64_table'
			for (l = 0; l < 64; ++l) {
				if (tmp[j] == base64_table[l]) {
					tmp[j] = l;
					break;
				}
			}
		}

		// decode remainder
		buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
		buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
		buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

		// write remainer decoded buffer to `dec'
		//dec = (unsigned char *) tls_mem_realloc(dec, size + (i - 1));
		for (j = 0; (j < i - 1); ++j) {
			dec[size++] = buf[j];
		}
	}

	// Make sure we have enough space to add '\0' character at end.
	//dec = (unsigned char *) tls_mem_realloc(dec, size + 1);
	dec[size] = '\0';

	// Return back the size of decoded string if demanded.
	if (decodedLen != NULL) *decodedLen = size;

	return dec;
}


long platform_rtos_disk_free(int drvtype)
{
	return 600*1024;
}

int platform_rtos_disk_volume(int drvtype)
{
	return '-'; //"C"
}

BOOL platform_lua_get_keypad_is_press(void)
{
	return keypad_is_press_bool;
}

BOOL paltform_is_lua_init_end(void)
{
	LogWriter::LOGX("[platform_rtos] paltform_is_lua_init_end");
	return g_s_init_end_bool == TRUE;
}
