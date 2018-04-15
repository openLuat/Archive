/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    sim_key.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Define 'sim_key' class.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/


#if !defined(__SIM_KEY_H__)
#define __SIM_KEY_H__

#include "event.h"
#include "platform_rtos.h"

#define MAX_KEY_SUPPORT 5

#define   DEVICE_KEY_UP          0x02
#define   DEVICE_KEY_DOWN        0x42
#define   DEVICE_KEY_LEFT        0x20
#define   DEVICE_KEY_RIGHT       0x24
#define   DEVICE_KEY_OK          0x00

#define   DEVICE_KEY_NONE		0xFE

typedef struct
{
	UINT8 key_bitmap_id;
	UINT8 drv_key_code;
	UINT8 lua_key_code;
} key_mapping_struct;

typedef struct 
{
	RECT location;
	key_mapping_struct mapping;
}sim_key_struct;

extern BOOL keypad_is_press_bool;
extern BOOL g_s_init_end_bool;

void key_mapping_init(RECT *pos, key_mapping_struct *map);
INT key_mapping(UINT8 type, POINT p);

#endif //__SIM_KEY_H__
