/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    sim_tp.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Simulate touchpad's flow.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "platform.h"
#include "platform_rtos.h"
#include "sim_tp.h"

static event_handle_t tp_event = INVALID_EVENT_HANDLE;
static TOUCH_ACTION_STATE_enum touchState = TOUCH_ACTION_NULL;
static T_AMOPENAT_TOUCH_MESSAGE moveStart;
static UINT16 last_move_x = 65534;
static UINT16 last_move_y = 65534;

UINT16 calcMoveOffset(UINT16 x1, UINT16 x2)
{
	return (x1>x2) ? (x1-x2) : (x2-x1);
}

void tp_event_cb(UINT8* data, UINT32 length)
{
	PlatformMessage *message = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

	message->id = MSG_ID_RTOS_TOUCH;
	if(data[0] == TP_INVALID)
	{
		LogWriter::LOGX("[sim_tp] touch is unknown type!");
		return;
	}
	message->data.touchMsgData.type = data[0];
	message->data.touchMsgData.x = data[1] | (data[2] << 8);
	message->data.touchMsgData.y = data[3] | (data[4] << 8);

	platform_rtos_send(message);
}

void tp_init(void)
{
	UINT8 data[5] = {0};

	data[0] = TP_INVALID;
	data[1] = last_move_x & 0xff;
	data[2] = (last_move_x & 0xff00) >> 8;
	data[3] = last_move_y & 0xff;
	data[4] = (last_move_y & 0xff00) >> 8;

	tp_event = add_event("TOUCHPAD", EVENT_TP, tp_event_cb);
	send_event(tp_event, (PCSTR)data, sizeof(data));
}

void tpActionSend(UINT8 type,UINT16 x, UINT16 y)
{
	UINT8 data[5] = {0};

	data[0] = type;
	data[1] = x & 0xff;
	data[2] = (x & 0xff00) >> 8;
	data[3] = y & 0xff;
	data[4] = (y & 0xff00) >> 8;

	daemon_emit_event(EVENT_TP, (PCSTR)data, sizeof(data));
}

void tp_capture_stance_reset(void)
{
	touchState = TOUCH_ACTION_NULL;

	moveStart.x = 0;
	moveStart.y = 0;
	last_move_x = 0;
	last_move_y = 0;
}

INT tp_capture_stance(UINT event, UINT16 x, UINT16 y)
{
	UINT16 xoffset,yoffset,cmd = -1;
	switch(touchState)
	{
	case TOUCH_ACTION_NULL:
		{
			if(event == TP_EVENT_DOWN)
			{
				touchState = TOUCH_ACTION_PRESS;
				tpActionSend(TP_PRESS,x,y);
			}
		}
		break;
	case TOUCH_ACTION_PRESS:
		{
			if(event == TP_EVENT_UP)
			{
				touchState = TOUCH_ACTION_NULL;
			}
			else if(event == TP_EVENT_LONGTAP_HOLD)
			{
				touchState = TOUCH_ACTION_HOLD;
			} 
			else if (event == TP_EVENT_MOVE)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_PRESS move x=%d y=%d", x,y);
				touchState = TOUCH_ACTION_MOVE;
				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
			}
		}
		break;
	case TOUCH_ACTION_HOLD:
		{
			if(event == TP_EVENT_UP)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_HOLD touch_action single x=%d y=%d", x,y);
				tpActionSend(TP_SINGLE,x,y);
				touchState = TOUCH_ACTION_NULL;
			}
			else if(event == TP_EVENT_MOVE)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_HOLD touch_action move start x=%d y=%d", x,y);
				moveStart.x = x;
				moveStart.y = y;
				last_move_x = x;
				last_move_y = y;

				touchState = TOUCH_ACTION_MOVE;
				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
			}
			else if(event == TP_EVENT_REPEAT)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_HOLD touch_action long start x=%d y=%d", x,y);
				tpActionSend(TP_LONG,x,y);
				touchState = TOUCH_ACTION_PRESS_LONG;
			}

		}
		break;
	case TOUCH_ACTION_PRESS_LONG:
		{
			if(event == TP_EVENT_UP)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_PRESS_LONG touch_action long release x=%d y=%d", x,y);
				tpActionSend(TP_LONG_RELEASE,x,y);
				touchState = TOUCH_ACTION_NULL;
			}
			else if(event == TP_EVENT_MOVE)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_PRESS_LONG touch_action long move start x=%d y=%d", x,y);
				touchState = TOUCH_ACTION_MOVE_LONG;
				moveStart.x = x;
				moveStart.y = y;

				last_move_x = x;
				last_move_y = y;

				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
			}
		}
		break;
	case TOUCH_ACTION_MOVE:
		{
			if(((event == TP_EVENT_MOVE) || (event == TP_EVENT_LONGTAP)
				|| (event == TP_EVENT_REPEAT) || (event == TP_EVENT_REPEAT_HOLD)) &&
				(calcMoveOffset(last_move_x,x) > TP_MOVE_STEP_MIN_VALUE || calcMoveOffset(last_move_y,y) > TP_MOVE_STEP_MIN_VALUE))
			{
				last_move_x = x;
				last_move_y = y;
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_PRESS_LONG TOUCH_ACTION_MOVE refresh x=%d y=%d", x,y);
				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
			}
			else if(event == TP_EVENT_UP)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_PRESS_LONG touch_action move end x=%d y=%d", x,y);
				xoffset = calcMoveOffset(moveStart.x,x);
				yoffset = calcMoveOffset(moveStart.y,y);
				if(xoffset > yoffset) //x方向移动
				{
					if (x < moveStart.x)
					{
						tpActionSend(TP_MOVE_RIGHT,x,y);
						cmd = 4;
					}
					else
					{
						tpActionSend(TP_MOVE_LEFT,x,y);
						cmd = 3;
					}
				}
				else //y方向移动
				{
					if (y < moveStart.y)
					{
						tpActionSend(TP_MOVE_DOWN,x,y);
						cmd = 2;
					}
					else
					{
						tpActionSend(TP_MOVE_UP,x,y);
						cmd = 1;
					}
				}
				touchState = TOUCH_ACTION_NULL;
				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
				tpActionSend(TP_LONG_MOVE_RELEASE,x,y);
			}
		}
		break;
	case TOUCH_ACTION_MOVE_LONG:
		{
			if(((event == TP_EVENT_MOVE) || (event == TP_EVENT_LONGTAP)
				|| (event == TP_EVENT_REPEAT) || (event == TP_EVENT_REPEAT_HOLD))&&
				(calcMoveOffset(last_move_x, x) > TP_MOVE_STEP_MIN_VALUE ||
				calcMoveOffset(last_move_y, y) > TP_MOVE_STEP_MIN_VALUE))
			{
				last_move_x = x;
				last_move_y = y;
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_MOVE_LONG refresh x=%d y=%d", x,y);
				tpActionSend(TP_LONG_MOVE_REFRESH,x,y);
			}
			else if(event == TP_EVENT_UP)
			{
				LogWriter::LOGX("[sim_tp] TOUCH_ACTION_MOVE_LONG touch_action long move end x=%d y=%d", x,y);
				xoffset = calcMoveOffset(moveStart.x,x);
				yoffset = calcMoveOffset(moveStart.y,y);
				tpActionSend(TP_LONG_MOVE_RELEASE,x,y);
				if(xoffset > yoffset)
				{
					if (x < moveStart.x)
					{
						tpActionSend(TP_LONG_MOVE_RIGHT,x,y);
					}
					else
					{
						tpActionSend(TP_LONG_MOVE_LEFT,x,y);
					}
				}
				else
				{
					if (y > moveStart.y)
					{
						tpActionSend(TP_LONG_MOVE_DOWN,x,y);
					}
					else
					{
						tpActionSend(TP_LONG_MOVE_UP,x,y);
					}
				}
				touchState = TOUCH_ACTION_NULL;
			}
		}
		break;
	default:
		{
			// do nothing
		}
		break;
	}

	if(event == TP_EVENT_UP) {
		if(cmd != -1)tpActionSend(TP_LAYER_FLATTEN_HANG_END, cmd, y);
		tpActionSend(TP_LAYER_FLATTEN_END, x, y);
	}

	return PLATFORM_OK;
}

#if defined(TOUCH_PANEL_SUPPORT)
void platform_tp_sleep_out(void)
{
	//TO DO ...
}

void platform_tp_sleep_in(void)
{
	//TO DO ...
}
#endif //TOUCH_PANEL_SUPPORT

