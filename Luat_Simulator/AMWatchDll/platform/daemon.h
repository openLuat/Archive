/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    daemon.h
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

#if !defined(__DAEMON_H__)
#define __DAEMON_H__

#include "event.h"

#define DAEMON_UDP_PORT (62887)
#define DAEMON_UDP_IP ("127.0.0.1")
#define DAEMON_UDP_PACKAGE_HEAD (2+1) //length: 2 bytes, id: 1 byte

DWORD WINAPI DaemonThreadProc(LPVOID lpParam);
void daemon_emit_event(unsigned char id, const char *data, int length);

#endif //__DAEMON_H__
