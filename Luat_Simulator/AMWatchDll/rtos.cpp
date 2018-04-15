/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    rtos.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Module for interfacing with the RTOS interface.
 * History:
 *     panjun 16/09/13 Initially create file.
 *     panjun 17/02/22 Add a dummy API,rtos.get_version.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "auxmods.h"
#include "platform.h"
#include "platform_rtos.h"
#include "platform_socket.h" 

#if !defined(_WIN32)
lua_State *appL;
int appnum;
#endif //!_WIN32

static void setfieldInt(lua_State *L, const char *key, int value)
{
    lua_pushstring(L, key);
    lua_pushinteger(L, value);
    lua_rawset(L, -3);
}

static void setfieldBool(lua_State *L, const char *key, int value)
{
    if(value < 0) // invalid value
        return;

    lua_pushstring(L, key);
    lua_pushboolean(L, value);
    lua_rawset(L, -3);
}

static void setfieldString(lua_State* L, const char* key, const char* str, const size_t len)
{
  lua_pushstring(L, key);
  lua_pushlstring(L, str, len);
  lua_rawset(L, -3);
}

static int handle_msg(lua_State *L, msg_type msg_id, PlatformMessage* pMsg)
{   
    int ret = 1;
    PlatformMsgData* msgData = &pMsg->data;

    
    switch(msg_id)
    {
    case MSG_ID_RTOS_WAIT_MSG_TIMEOUT:
        lua_pushinteger(L, msg_id);
        // no error msg data.
        break;
        
    case MSG_ID_RTOS_TIMER:
        lua_pushinteger(L, msg_id);
        lua_pushinteger(L, msgData->timer_id);
        ret = 2;
        break;

	case SIMU_UART_ATC_RX_DATA:
		lua_pushinteger(L, MSG_ID_RTOS_UART_RX_DATA);
		lua_pushinteger(L, msgData->uart_id);
		ret = 2;
		break;
    case MSG_ID_RTOS_UART_RX_DATA:
        lua_pushinteger(L, msg_id);
        lua_pushinteger(L, msgData->uart_id);
        ret = 2;
        break;

    case MSG_ID_RTOS_KEYPAD:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldBool(L, "pressed", msgData->keypadMsgData.bPressed);
        setfieldInt(L, "key_matrix_row", msgData->keypadMsgData.data.matrix.row);
        setfieldInt(L, "key_matrix_col", msgData->keypadMsgData.data.matrix.col);
        break;

    case MSG_ID_RTOS_INT:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldInt(L, "int_id", msgData->interruptData.id);
        setfieldInt(L, "int_resnum", msgData->interruptData.resnum);
        break;

    case MSG_ID_RTOS_PMD:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldBool(L, "present", msgData->pmdData.battStatus);
        setfieldInt(L, "voltage", msgData->pmdData.battVolt);
        setfieldInt(L, "level", msgData->pmdData.battLevel);
        setfieldBool(L, "charger", msgData->pmdData.chargerStatus);
        setfieldInt(L, "state", msgData->pmdData.chargeState);
        break;

    case MSG_ID_RTOS_AUDIO:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        if(msgData->audioData.playEndInd == TRUE)
            setfieldBool(L,"play_end_ind",TRUE);
        else if(msgData->audioData.playErrorInd == TRUE)
            setfieldBool(L,"play_error_ind",TRUE);
        break;

    case MSG_ID_RTOS_WEAR_STATUS:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldBool(L,"wear_status_ind",msgData->wearStatusData.wearStatus);
        break;
        
    case MSG_ID_RTOS_RECORD:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        
        if(msgData->recordData.recordEndInd == TRUE)
            setfieldBool(L,"record_end_ind",TRUE);
        else if(msgData->recordData.recordErrorInd == TRUE)
            setfieldBool(L,"record_error_ind",TRUE);
        break;
       
    case MSG_ID_RTOS_ALARM:
        lua_pushinteger(L, msg_id);
        ret = 1;
        break;

    case MSG_ID_RTOS_TOUCH:
#ifdef AM_RH_TP_SUPPORT
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldInt(L, "type", msgData->touchMsgData.type);
        setfieldInt(L, "x", msgData->touchMsgData.x);
        setfieldInt(L, "y", msgData->touchMsgData.y);
#else
        lua_pushinteger(L, msg_id);
        lua_pushinteger(L, msgData->touchMsgData.type);
        lua_pushinteger(L, msgData->touchMsgData.x);
        lua_pushinteger(L, msgData->touchMsgData.y);
        ret = 4;
#endif
        break;

    #if defined(__AM_LUA_TTSPLY_SUPPORT__)
    case MSG_ID_RTOS_TTSPLY_STATUS:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldInt(L,"ttsply_status_ind", msgData->ttsPlyData.ttsPlyStatusInd);
        break;
    case MSG_ID_RTOS_TTSPLY_ERROR:
        lua_newtable(L);    
        setfieldInt(L, "id", msg_id);
        setfieldInt(L,"ttsply_error_ind", msgData->ttsPlyData.ttsPlyErrorInd);
        break;
    #endif //__AM_LUA_TTSPLY_SUPPORT__

    case MSG_ID_APP_MTHL_CREATE_PDP_PARAM_CNF:
	ret = platform_on_create_pdp_param_cnf(L, pMsg);
        break;
        
    
    case MSG_ID_APP_MTHL_ACTIVATE_PDP_CNF:
	ret = platform_on_active_pdp_cnf(L, pMsg);
        break;

    case MSG_ID_APP_MTHL_GET_HOST_BY_NAME_CNF:
        ret = platform_on_get_host_by_name_cnf(L, pMsg);
        break;
        

    case MSG_ID_APP_MTHL_CREATE_CONN_CNF:
        ret = platform_on_create_conn_cnf(L, pMsg);
        break;
        
    case MSG_ID_APP_MTHL_CREATE_SOCK_IND:
        ret = platform_on_create_sock_ind(L, pMsg);
        break;
        
    case MSG_ID_APP_MTHL_SOCK_SEND_CNF:
        ret = platform_on_send_data_cnf(L, pMsg);
        break;

    case MSG_ID_APP_MTHL_SOCK_SEND_IND:
        ret = platform_on_send_data_ind(L, pMsg);
        break;

    case MSG_ID_APP_MTHL_SOCK_RECV_IND:
        ret = platform_on_recv_data_ind(L, pMsg);
        break;

    case MSG_ID_APP_MTHL_DEACTIVATE_PDP_CNF:
        ret = platform_on_deactivate_pdp_cnf(L, pMsg);
        break;

    case MSG_ID_APP_MTHL_SOCK_CLOSE_CNF:
        ret = platform_on_sock_close_cnf(L, pMsg);
        break;
        
    case MSG_ID_APP_MTHL_SOCK_CLOSE_IND:
        ret = platform_on_sock_close_ind(L, pMsg);
        break;
        
    case MSG_ID_APP_MTHL_DEACTIVATED_PDP_IND:
        ret = platform_on_deactivate_pdp_ind(L, pMsg);
        break;
       
    default:
        ret = 0;
        break;
    }
    
    return ret;
}

static int l_rtos_receive(lua_State *L) 		/* rtos.receive() */
{
    UINT32 timeout = luaL_checkinteger( L, 1 );
    PlatformMessage* pMsg = NULL;
    msg_type msg_id;
    
    static BOOL firstRecv = TRUE;
    int ret = 0;

    if(firstRecv)
    {
        firstRecv = FALSE;
        platform_poweron_try();
    }

    if(platform_rtos_receive(&msg_id, (void**)&pMsg, timeout) != PLATFORM_OK)
    {
        return luaL_error(L, "rtos.receive error!");
    }

    
    ret = handle_msg(L, msg_id, pMsg);

    if(pMsg)
    {
        platform_rtos_free_msg(pMsg);
    }

    return ret;
}

static int l_rtos_sleep(lua_State *L)   /* rtos.sleep()*/
{
    int ms = luaL_checkinteger( L, 1 );

    platform_os_sleep(ms);
    
    return 0;
}

static int l_rtos_timer_start(lua_State *L)
{
    int timer_id = luaL_checkinteger(L,1);
    int ms = luaL_checkinteger(L,2);
    int ret;

    ret = platform_rtos_start_timer(timer_id, ms);

    lua_pushinteger(L, ret);

    return 1;
}

static int l_rtos_timer_stop(lua_State *L)
{
    int timer_id = luaL_checkinteger(L,1);
    int ret;

    ret = platform_rtos_stop_timer(timer_id);

    lua_pushinteger(L, ret);

    return 1;
}

static int l_rtos_init_module(lua_State *L)
{
    int module_id = luaL_checkinteger(L, 1);
    int ret;

    switch(module_id)
    {
    case RTOS_MODULE_ID_KEYPAD:
        {
            PlatformKeypadInitParam param;

            UINT8 type = (UINT8)luaL_checkinteger(L, 2);
            UINT8 inmask = (UINT8)luaL_checkinteger(L, 3);
            UINT8 outmask = (UINT8)luaL_checkinteger(L, 4);

            param.type = type;
            param.matrix.inMask = inmask;
            param.matrix.outMask = outmask;

            ret = platform_rtos_init_module(RTOS_MODULE_ID_KEYPAD, &param);
        }
        break;

    case RTOS_MODULE_ID_ALARM:
        {
            ret = platform_rtos_init_module(RTOS_MODULE_ID_ALARM, NULL);
        }
		break;

    case RTOS_MODULE_ID_TOUCH:
        {
            ret = platform_rtos_init_module(RTOS_MODULE_ID_TOUCH, NULL);
        }
		break;
    default:
        return luaL_error(L, "rtos.init_module: module id must < %d", NumOfRTOSModules);
        break;
    }

    lua_pushinteger(L, ret);

    return 1;
}

static int l_rtos_poweron_reason(lua_State *L)
{
    lua_pushinteger(L, platform_get_poweron_reason());
    return 1;
}

static int l_rtos_poweron(lua_State *L)
{
    int flag = luaL_checkinteger(L, 1);
    platform_rtos_poweron(flag);
    return 0;
}

static int l_rtos_repoweron(lua_State *L)
{
    platform_rtos_repoweron();
    return 0;
}

static int l_rtos_poweroff(lua_State *L)
{
	platform_rtos_poweroff();	
	return 0;
}

static int l_rtos_restart(lua_State *L)
{
	platform_rtos_restart();	
	return 0;
}

static int l_rtos_tick(lua_State *L)
{
    lua_pushinteger(L, platform_rtos_tick());
    return 1;
}
static int l_rtos_sms_is_ready(lua_State *L)
{
    lua_pushinteger(L, platform_rtos_sms_is_ready());
    return 1;
}

#if !defined(_WIN32)
kal_timerid monitorid1=0;

static void app_monitor_call()
{
	LogWriter::LOGX("app_monitor_call");
    lua_getglobal(appL, "check_app"); /* function to be called */
    lua_call(appL, 0, 0);
    lua_sethook(appL,NULL,0,0);  /* close hooks */
}

static int app_monitor()
{
    const char *flag,*flag1;
    int   mask = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
    lua_sethook(appL, app_monitor_call, mask, 1);  /* set hooks */ 
}

static int l_rtos_app_monitor(lua_State *L)
{
	LogWriter::LOGX("l_rtos_app_monitor");
    appL=L;
    appnum=luaL_checkinteger(appL, 1);
   if (monitorid1==0)
    {  
      monitorid1= kal_create_timer("app_monitor");
    }
    kal_set_timer(monitorid1,app_monitor,NULL,appnum,0);  
    return 0;
}
#endif //_WIN32

static int l_rtos_setalarm(lua_State *L)
{
    PlatformSetAlarmParam alarmparam;
    
    alarmparam.alarmon = luaL_checkinteger(L, 1);
    alarmparam.year = (UINT16)luaL_checkinteger(L, 2);
    alarmparam.month = (UINT8)luaL_checkinteger(L, 3);
    alarmparam.day = (UINT8)luaL_checkinteger(L, 4);
    alarmparam.hour = (UINT8)luaL_checkinteger(L, 5);
    alarmparam.min = (UINT8)luaL_checkinteger(L, 6);
    alarmparam.sec = (UINT8)luaL_checkinteger(L, 7);

    LogWriter::LOGX("l_rtos_setalarm:%d %d %d %d %d %d %d", 
        alarmparam.alarmon,
        alarmparam.year,
        alarmparam.month,
        alarmparam.day,
        alarmparam.hour,
        alarmparam.min,
        alarmparam.sec);
    platform_rtos_setalarm(&alarmparam);	
    return 0;
}



static int l_rtos_base64_encode(lua_State *L)
{ 
    size_t  len      = 0;
    char* string     = (char*)luaL_checklstring(L, 1, &len);
    char* endcode_string;
    
    endcode_string = platform_base64_encode(string, len);

    lua_pushstring(L, endcode_string);
    
	WinUtil::L_FREE(endcode_string);
    return 1;
}

static int l_rtos_base64_decode(lua_State *L)
{ 
    INT  len      = 0;
    INT  decoded_len = 0;
    char* string     = (char*)luaL_checklstring(L, 1, (size_t*)&len);
    char* endcode_string;
    
    endcode_string = platform_base64_decode(string, len, &decoded_len);

    lua_newtable(L);    
    setfieldInt(L, "len", decoded_len);
    setfieldString(L, "str", endcode_string, decoded_len);
        
	WinUtil::L_FREE(endcode_string);
    return 1;
}

static int l_rtos_disk_free(lua_State *L)
{
    int drvtype = luaL_optinteger(L,1,0);

    lua_pushinteger(L, platform_rtos_disk_free(drvtype));
    return 1;
}

static int l_rtos_disk_volume(lua_State *L)
{
    int drvtype = luaL_optinteger(L,1,0);
    
    lua_pushinteger(L, platform_rtos_disk_volume(drvtype));
    return 1;
}

int l_rtos_keypad_state_get(lua_State *L)
{
    lua_pushinteger(L, platform_lua_get_keypad_is_press());
    return 1;
}

int l_rtos_keypad_init_over(lua_State *L)
{
	lua_pushinteger(L, paltform_is_lua_init_end());
	return 1;
}

#ifdef WIFI_UPDATE_SUPPORT
typedef struct BOOTLOADER_INFO_TAG
{
    UINT8   version[24];
    UINT32 restart_to_download;
    UINT32 download_status;
    UINT8   reserved[64];
}BOOTLOADER_INFO;

extern const BOOTLOADER_INFO bootloader_info;
extern UINT32 Load$$BOOT_INFO_GFH$$Base;

static int l_rtos_wifiupdate(lua_State *L)
{
    UINT32 value = 0;
    UINT8 *p = (UINT8*)&bootloader_info;
    BOOTLOADER_INFO info = {0};

    LogWriter::LOGX("bootloader_info %p, base %p, %p, %p", \
		&bootloader_info, sizeof(BOOTLOADER_INFO), \
		(kal_uint32 *)&Load$$BOOT_INFO_GFH$$Base, \
		(kal_uint32 *)&(bootloader_info.restart_to_download));
    
    writeCustomResData((char *)value, &(bootloader_info.restart_to_download), 4);
    OPENAT_Delayms(1000);
    platform_rtos_restart();

    return 1;
}
#endif

static int l_rtos_get_version(lua_State* L)
{
	lua_pushstring(L, "Luat_V9999_AirM2M");
	return 1;
}

#define MIN_OPT_LEVEL 2
const luaL_reg rtos_map[] =
{
    {"init_module",  l_rtos_init_module},
    {"poweron_reason",  l_rtos_poweron_reason},
    {"poweron",  l_rtos_poweron},
    {"repoweron", l_rtos_repoweron},
    {"poweroff",  l_rtos_poweroff},
    {"restart",  l_rtos_restart},
    {"receive",  l_rtos_receive},
    //{"send", l_rtos_send},
    {"sleep", l_rtos_sleep},
    {"timer_start", l_rtos_timer_start},
    {"timer_stop", l_rtos_timer_stop},
    {"tick", l_rtos_tick},
    {"sms_is_ready", l_rtos_sms_is_ready},
#if !defined(_WIN32)
    {"app_monitor", l_rtos_app_monitor},
#endif //_WIN32
    {"set_alarm", l_rtos_setalarm},
    
    {"base64_encode", l_rtos_base64_encode},
    {"base64_decode",  l_rtos_base64_decode},
    {"disk_free", l_rtos_disk_free},
    {"disk_volume", l_rtos_disk_volume},
	{"keypad_state", l_rtos_keypad_state_get},
	{"keypad_init_end", l_rtos_keypad_init_over},
#ifdef WIFI_UPDATE_SUPPORT
    {"wifiupdate", l_rtos_wifiupdate},
#endif
	{"get_version", l_rtos_get_version},
	{NULL, NULL}
};

int luaopen_rtos( lua_State *L )
{
    luaL_register( L, AUXLIB_RTOS, rtos_map );

    // module id
    MOD_REG_NUMBER(L, "MOD_KEYPAD", RTOS_MODULE_ID_KEYPAD);
    MOD_REG_NUMBER(L, "MOD_ALARM", RTOS_MODULE_ID_ALARM);
    MOD_REG_NUMBER(L, "MOD_TOUCH", RTOS_MODULE_ID_TOUCH);
    
    #define REG_POWERON_RESON(rEASON) MOD_REG_NUMBER(L, #rEASON, PLATFORM_##rEASON)
    REG_POWERON_RESON(POWERON_KEY);
    REG_POWERON_RESON(POWERON_CHARGER);
    REG_POWERON_RESON(POWERON_ALARM);
    REG_POWERON_RESON(POWERON_RESTART);
    REG_POWERON_RESON(POWERON_OTHER);
    REG_POWERON_RESON(POWERON_UNKNOWN);
    REG_POWERON_RESON(POWERON_EXCEPTION);
    REG_POWERON_RESON(POWERON_HOST);
    REG_POWERON_RESON(POWERON_WATCHDOG);

    // msg id
    MOD_REG_NUMBER(L, "WAIT_MSG_TIMEOUT", MSG_ID_RTOS_WAIT_MSG_TIMEOUT);
    MOD_REG_NUMBER(L, "MSG_TIMER", MSG_ID_RTOS_TIMER);
    MOD_REG_NUMBER(L, "MSG_KEYPAD", MSG_ID_RTOS_KEYPAD);
    MOD_REG_NUMBER(L, "MSG_UART_RXDATA", MSG_ID_RTOS_UART_RX_DATA);
    MOD_REG_NUMBER(L, "MSG_INT", MSG_ID_RTOS_INT);
    MOD_REG_NUMBER(L, "MSG_PMD", MSG_ID_RTOS_PMD);
    MOD_REG_NUMBER(L, "MSG_AUDIO", MSG_ID_RTOS_AUDIO);
    MOD_REG_NUMBER(L, "MSG_RECORD", MSG_ID_RTOS_RECORD);
    MOD_REG_NUMBER(L, "MSG_ALARM", MSG_ID_RTOS_ALARM);
    MOD_REG_NUMBER(L, "MSG_TOUCH", MSG_ID_RTOS_TOUCH);

    #if defined(__AM_LUA_TTSPLY_SUPPORT__)
    MOD_REG_NUMBER(L, "MSG_TTSPLY_STATUS", MSG_ID_RTOS_TTSPLY_STATUS);
    MOD_REG_NUMBER(L, "MSG_TTSPLY_ERROR", MSG_ID_RTOS_TTSPLY_ERROR);
    #endif //__AM_LUA_TTSPLY_SUPPORT__

    MOD_REG_NUMBER(L, "MSG_WEAR_STATUS", MSG_ID_RTOS_WEAR_STATUS);

	MOD_REG_NUMBER(L, "MSG_PDP_ACT_CNF", MSG_ID_TCPIP_PDP_ACTIVATE_CNF);
	MOD_REG_NUMBER(L, "MSG_PDP_DEACT_CNF", MSG_ID_TCPIP_PDP_DEACTIVATE_CNF);
	MOD_REG_NUMBER(L, "MSG_PDP_DEACT_IND", MSG_ID_TCPIP_PDP_DEACTIVATE_IND);
	MOD_REG_NUMBER(L, "MSG_SOCK_CONN_CNF", MSG_ID_TCPIP_SOCKET_CONNECT_CNF);
	MOD_REG_NUMBER(L, "MSG_SOCK_SEND_CNF", MSG_ID_TCPIP_SOCKET_SEND_CNF);
	MOD_REG_NUMBER(L, "MSG_SOCK_SEND_IND", MSG_ID_TCPIP_SOCKET_SEND_IND);
	MOD_REG_NUMBER(L, "MSG_SOCK_RECV_IND", MSG_ID_TCPIP_SOCKET_RECV_IND);
	MOD_REG_NUMBER(L, "MSG_SOCK_CLOSE_CNF", MSG_ID_TCPIP_SOCKET_CLOSE_CNF);
	MOD_REG_NUMBER(L, "MSG_SOCK_CLOSE_IND", MSG_ID_TCPIP_SOCKET_CLOSE_IND);

    //timeout
    MOD_REG_NUMBER(L, "INF_TIMEOUT", PLATFORM_RTOS_WAIT_MSG_INFINITE);

    platform_rtos_init();

    return 1;
}


