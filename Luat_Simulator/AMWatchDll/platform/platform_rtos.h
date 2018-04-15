/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_rtos.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'RTOS' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_RTOS_H__
#define __PLATFORM_RTOS_H__

#include "platform.h"
#include "platform_wtimer.h"

enum
{
	RTOS_MSG_WAIT_MSG_TIMEOUT = (WM_USER+WM_USER_OFFSET), /* 1100 */
	RTOS_MSG_TIMER,
	DAEMON_TIMER_MSG_ID,
	SIMU_TIMER_MSG_ID,
	SIMU_UART_ATC_RX_DATA,
	SIMU_RTOS_MSG_ID,
	RTOS_MSG_PMD,
	RTOS_MSG_KEYPAD,
	RTOS_MSG_UART_RX_DATA,
	RTOS_MSG_INT,
	RTOS_MSG_AUDIO,
	RTOS_MSG_MAX_IND,
};

enum
{
	MSG_ID_RTOS_WAIT_MSG_TIMEOUT = RTOS_MSG_MAX_IND,
	MSG_ID_RTOS_TIMER,
	MSG_ID_RTOS_UART_RX_DATA,
	MSG_ID_RTOS_KEYPAD,
	MSG_ID_RTOS_INT,
	MSG_ID_RTOS_PMD,
	MSG_ID_RTOS_AUDIO,
	MSG_ID_RTOS_WEAR_STATUS,
	MSG_ID_RTOS_RECORD,
	MSG_ID_RTOS_ALARM,
	MSG_ID_RTOS_TOUCH,
	MSG_ID_RTOS_TTSPLY_STATUS,
	MSG_ID_RTOS_TTSPLY_ERROR,
	MSG_ID_UART_ESCAPE_DETECTED_IND,
	MSG_ID_UART_READY_TO_READ_IND,
	MSG_ID_UART_READY_TO_WRITE_IND,
	MSG_ID_UART_PLUGOUT_IND,
	MSG_ID_SOC_ABM_ACTIVATE_REQ,
	MSG_ID_SOC_ABM_ACTIVATE_CNF,
	MSG_ID_MMI_ABM_NWK_SRV_CNF,
	MSG_ID_SOC_ABM_DEACTIVATE_REQ,
	MSG_ID_SOC_ABM_DEACTIVATE_CNF,
	MSG_ID_APP_SOC_DEACTIVATE_REQ,
	MSG_ID_MMI_ABM_BEARER_EVENT_CONNECT_CNF,
	MSG_ID_MMI_ABM_UPDATE_ACCOUNT_INFO_REQ,
	MSG_ID_APP_CBM_BEARER_INFO_IND,
	MSG_ID_APP_MTHL_CREATE_PDP_PARAM_REQ,
	MSG_ID_APP_MTHL_CREATE_PDP_PARAM_CNF,
	MSG_ID_APP_MTHL_GET_PDP_PARAM_REQ,
	MSG_ID_APP_MTHL_GET_PDP_PARAM_CNF,
	MSG_ID_APP_MTHL_GET_PDP_STATE_REQ,
	MSG_ID_APP_MTHL_GET_PDP_STATE_CNF,
	MSG_ID_APP_MTHL_ACTIVATE_PDP_REQ,
	MSG_ID_APP_MTHL_ACTIVATE_PDP_CNF,
	MSG_ID_APP_MTHL_SOCK_RECV_REQ,
	MSG_ID_APP_MTHL_SOCK_RECV_CNF,
	MSG_ID_APP_MTHL_SOCK_RECV_IND,
	MSG_ID_APP_MTHL_QUERY_SOCK_INFO_REQ,
	MSG_ID_APP_MTHL_QUERY_SOCK_INFO_CNF,
	MSG_ID_APP_MTHL_GET_HOST_BY_NAME_REQ,
	MSG_ID_APP_MTHL_GET_HOST_BY_NAME_CNF,
	MSG_ID_APP_MTHL_GET_HOST_BY_NAME_IND,
	MSG_ID_APP_MTHL_GET_HOST_BY_ADDR_REQ,
	MSG_ID_APP_MTHL_GET_HOST_BY_ADDR_CNF,
	MSG_ID_APP_MTHL_GET_HOST_BY_ADDR_IND,
	MSG_ID_APP_MTHL_SERVER_LAUNCH_REQ,
	MSG_ID_APP_MTHL_SERVER_LISTEN_CNF,
	MSG_ID_APP_MTHL_SERVER_ACCEPT_IND,
	MSG_ID_APP_MTHL_CREATE_CONN_REQ,
	MSG_ID_APP_MTHL_CREATE_CONN_CNF,
	MSG_ID_APP_MTHL_CREATE_SOCK_IND,
	MSG_ID_APP_MTHL_SOCK_CLOSE_REQ,
	MSG_ID_APP_MTHL_SOCK_CLOSE_CNF,
	MSG_ID_APP_MTHL_SOCK_CLOSE_IND,
	MSG_ID_APP_MTHL_SOCK_SEND_REQ,
	MSG_ID_APP_MTHL_SOCK_SEND_CNF,
	MSG_ID_APP_MTHL_SOCK_SEND_IND,
	MSG_ID_APP_SOC_NOTIFY_IND,
	MSG_ID_APP_SOC_GET_HOST_BY_ADDR_IND,
	MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND,
	MSG_ID_APP_MTHL_DEACTIVATE_PDP_REQ,
	MSG_ID_APP_MTHL_DEACTIVATE_PDP_CNF,
	MSG_ID_APP_MTHL_QUERY_SOCK_NUM_REQ,
	MSG_ID_APP_MTHL_QUERY_SOCK_NUM_CNF,	
	MSG_ID_APP_MTHL_DEACTIVATED_PDP_IND,
	MSG_ID_APP_MTHL_TRANSPARENT_TRANS_REQ,		
	MSG_ID_APP_MTHL_TRANSPARENT_TRANS_CNF,		
	MSG_ID_APP_MTHL_TRANSPARENT_TRANS_END_IND,
	MSG_ID_APP_MTHL_MAX_IND,
};

enum
{
  MSG_ID_TCPIP_PDP_ACTIVATE_CNF = MSG_ID_APP_MTHL_MAX_IND,
  MSG_ID_TCPIP_PDP_DEACTIVATE_CNF,
  MSG_ID_TCPIP_PDP_DEACTIVATE_IND,
  MSG_ID_TCPIP_SOCKET_CONNECT_CNF,
  MSG_ID_TCPIP_SOCKET_SEND_CNF,
  MSG_ID_TCPIP_SOCKET_SEND_IND,
  MSG_ID_TCPIP_SOCKET_RECV_IND,
  MSG_ID_TCPIP_SOCKET_CLOSE_CNF,
  MSG_ID_TCPIP_SOCKET_CLOSE_IND,
  MSG_ID_TCPIP_MAX_IND,
};

enum
{
	WM_LUA_MAIN = MSG_ID_TCPIP_MAX_IND,
	WM_LUA_UPDATE, // only debug
	WM_LUA_PLAY_GIF,
	WM_LUA_STOP_GIF,
	WM_LUA_MAX_IND,
};

enum
{
	WM_UART_READ_THREAD = WM_LUA_MAX_IND,
	WM_UART_WRITE_THREAD,
};

typedef enum PlatformPoweronReasonTag
{
	PLATFORM_POWERON_KEY,
	PLATFORM_POWERON_CHARGER,
	PLATFORM_POWERON_ALARM,
	PLATFORM_POWERON_RESTART,
	PLATFORM_POWERON_OTHER,
	PLATFORM_POWERON_UNKNOWN,
	PLATFORM_POWERON_EXCEPTION,
	PLATFORM_POWERON_HOST,
	PLATFORM_POWERON_WATCHDOG,
}PlatformPoweronReason;

typedef enum PlatformRtosModuleTag
{
	RTOS_MODULE_ID_KEYPAD,
	RTOS_MODULE_ID_ALARM,
	RTOS_MODULE_ID_TOUCH,
	NumOfRTOSModules
}PlatformRtosModule;

typedef struct KeypadMatrixDataTag
{
    unsigned char       row;
    unsigned char       col;
}KeypadMatrixData;

typedef struct KeypadMsgDataTag
{
	UINT8   type;  // keypad type
	BOOL bPressed; /* push down? */
	union {
		struct {
			UINT8 row;
			UINT8 col;
		}matrix, gpio;
		UINT16 adc;
	}data;
}KeypadMsgData;

typedef struct TouchMsgDataTag
{
	UINT8   type;  // touch action type
	UINT16   x;
	UINT16   y;
}TouchMsgData;

typedef struct PlatformIntDataTag
{
	elua_int_id             id;
	elua_int_resnum         resnum;
}PlatformIntData;

#define PLATFORM_BATT_NOT_CHARGING      0
#define PLATFORM_BATT_CHARING           1
#define PLATFORM_BATT_CHARGE_STOP       2

typedef struct PlatformPmdDataTag
{
	BOOL    battStatus;
	BOOL    chargerStatus;
	UINT8      chargeState;
	UINT8      battLevel;
	UINT16     battVolt;
}PlatformPmdData;

typedef struct PlatformAudioDataTag
{
	BOOL    playEndInd;
	BOOL    playErrorInd;
}PlatformAudioData;

typedef struct PlatformRecordDataTag
{
	BOOL    recordEndInd;
	BOOL    recordErrorInd;
}PlatformRecordData;

typedef struct PlatformWearStatusDataTag
{
	BOOL    wearStatus;
}PlatformWearStatusData;

typedef struct tagPlatformTtsPlyData
{
	INT32    ttsPlyStatusInd;
	INT32    ttsPlyErrorInd;
}PlatformTtsPlyData;

#define PLATFORM_RTOS_WAIT_MSG_INFINITE         (0)
typedef union PlatformMsgDataTag
{
	INT                 timer_id;
	INT                 uart_id;
	KeypadMsgData       keypadMsgData;
	TouchMsgData        touchMsgData;
	PlatformIntData     interruptData;
	PlatformPmdData     pmdData;
	PlatformAudioData   audioData;
	PlatformRecordData recordData;
	PlatformWearStatusData wearStatusData;
	PlatformTtsPlyData ttsPlyData;
}PlatformMsgData;

typedef struct local_para_struct {
    /* ref_count: reference count; 
     * lp_reserved : reserved for future; 
     * msg_len  : total length including this header.
     */
    LOCAL_PARA_HDR
} local_para_struct;

typedef struct PlatformMessageTag
{
	INT id;
	PlatformMsgData data;
	local_para_struct *local_para_ptr;
}PlatformMessage;


typedef struct PlatformKeypadInitParamTag
{
    UINT8 type;
    struct{
        UINT8 inMask;         /* active key in mask */
        UINT8 outMask;        /* active key out mask */        
    }matrix;
}PlatformKeypadInitParam;

typedef struct PlatformSetAlarmParamTag
{
    BOOL alarmon;
    UINT16 year;
    UINT8 month;
    UINT8 day;
    UINT8 hour;
    UINT8 min;
    UINT8 sec;
}PlatformSetAlarmParam;

extern BOOL startRtosSimulator;

extern void keypad_init(PlatformKeypadInitParam *param);
extern void LuaChargingPwnOnThanPwnOn(void);

void SendToLoadLuaMessage(const MSG *msg);
void SendToLuaShellMessage(const MSG *msg);

int platform_rtos_init(void);

int platform_rtos_poweroff(void);
int platform_rtos_restart(void);

int platform_rtos_init_module(int module, void *pParam);

int platform_rtos_receive(msg_type* msg_id, void **ppMessage, UINT32 timeout);

void platform_rtos_free_msg(void *msg_body);

int platform_rtos_send(PlatformMessage *pMsg);

int platform_rtos_start_timer(int timer_id, int milliSecond);

int platform_rtos_stop_timer(int timer_id);
int platform_rtos_tick(void);
int platform_rtos_sms_is_ready(void);
int platform_get_poweron_reason(void);

int platform_rtos_poweron(int flag);

void platform_poweron_try(void);
int platform_rtos_setalarm(void *pParam);
int platform_rtos_repoweron(void);
char*  platform_base64_encode(const char *str, size_t length);

char* platform_base64_decode (const char *src, int length,int* decodedLen);
long platform_rtos_disk_free(int drvtype);
int platform_rtos_disk_volume(int drvtype);

BOOL platform_lua_get_keypad_is_press(void);
BOOL paltform_is_lua_init_end(void);

#endif //__PLATFORM_RTOS_H__
