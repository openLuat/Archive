/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    event.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *
 *
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/
 
#ifndef __EVENT_H__
#define __EVENT_H__

#define EVENT_CTRL_CONSOLE  0x00
#define EVENT_KEY           0x10
#define EVENT_GPIO          0x11
#define EVENT_TP            0x12

typedef intptr_t event_handle_t;
typedef void (*event_cb_t)(UINT8* data, UINT32 length);

#define INVALID_EVENT_HANDLE    ((event_handle_t)0)

event_handle_t add_event(PCSTR name, UINT8 id, event_cb_t cb);

void remove_event(event_handle_t *handle);

void dispatch_event(UINT8* data, int length);

void send_event(event_handle_t handle, PCSTR data, int length);

void daemon_emit_event(unsigned char id, const char *data, int length);

#endif //__EVENT_H__