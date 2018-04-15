/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    sim_tp.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Simulate touchpad's flow.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#if !defined(__SIM_TP__)
#define __SIM_TP__

#include "event.h"
#include "platform_rtos.h"

#define TP_SINGLE_DEBOUNCE_TIME (100)
#define TP_LONGTAG_DEBOUNCE_TIME (300)

//tp event
#define TP_EVENT_DOWN 0  
#define TP_EVENT_UP   1
#define TP_EVENT_MOVE 2
#define TP_EVENT_LONGTAP 3
#define TP_EVENT_REPEAT 4
#define TP_EVENT_LONGTAP_HOLD 10
#define TP_EVENT_REPEAT_HOLD 11

// tp touch-stance
#define TP_SINGLE 0
#define TP_MOVE_UP       1
#define TP_MOVE_DOWN     2
#define TP_MOVE_LEFT     3
#define TP_MOVE_RIGHT    4
#define TP_LONG          5
#define TP_LONG_RELEASE  6

#define TP_LONG_MOVE_UP       7
#define TP_LONG_MOVE_DOWN     8
#define TP_LONG_MOVE_LEFT     9
#define TP_LONG_MOVE_RIGHT    10

#define TP_LONG_MOVE_REFRESH    11
#define TP_LONG_MOVE_RELEASE    12
#define TP_PRESS 13
#define TP_LAYER_FLATTEN_END 14
#define TP_LAYER_FLATTEN_HANG_END 15
#define TP_INVALID       0xFF

#define TP_MOVE_STEP_MIN_VALUE 12

//tp state
typedef enum 
{
	TOUCH_ACTION_NULL,
	TOUCH_ACTION_PRESS,
	TOUCH_ACTION_HOLD,
	TOUCH_ACTION_PRESS_LONG,
	TOUCH_ACTION_MOVE,
	TOUCH_ACTION_MOVE_LONG,
}TOUCH_ACTION_STATE_enum;

typedef struct T_AMOPENAT_TOUCH_MESSAGE_TAG
{
	UINT8 type;
	UINT16 x;
	UINT16 y;
}T_AMOPENAT_TOUCH_MESSAGE;

typedef struct
{
	// Touch_Panel_Event_enum
	UINT event; 
	/*coordinate point, not diff*/
	UINT16 x;  
	UINT16 y;
} TP_SINGLE_EVENT_T;

typedef struct
{
	UINT16   model; // Single=0
	DWORD  time_stamp;
	TP_SINGLE_EVENT_T points;
} TouchPanelEventStruct;

UINT16 calcMoveOffset(UINT16 x1, UINT16 x2);
void tp_init(void);
void tp_capture_stance_reset(void);
INT tp_capture_stance(UINT event, UINT16 x, UINT16 y);
#if defined(TOUCH_PANEL_SUPPORT)
void platform_tp_sleep_out(void);
void platform_tp_sleep_in(void);
#endif //TOUCH_PANEL_SUPPORT

#endif //__SIM_TP__