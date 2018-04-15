
/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    daemon.cpp
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

#include "stdafx.h"
#include "daemon.h"
#include "Winsock2.h"

static int daemon_socket = INVALID_SOCKET;
static SOCKADDR_IN server_addr;

void daemon_emit_event(unsigned char id, PCSTR data, int length){
    PSTR packet,pnet;
    UINT16 pos = 0;
    UINT16 packet_len;

    if(INVALID_SOCKET == daemon_socket){
        return;
    }

    packet_len = length + DAEMON_UDP_PACKAGE_HEAD;
	pnet = packet = (PSTR)WinUtil::L_MALLOC(packet_len);
    
    NET_STORE_LEN(pnet, packet_len);
    NET_STORE_ID(pnet, id);
    for(INT i=0; i < length; i++)
	{
		NET_STORE_DATA(pnet, data[i]);
	}

    sendto(daemon_socket, packet, packet_len, 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN));

	WinUtil::L_FREE(packet);
}

void daemon_close(void){
    if(INVALID_SOCKET == daemon_socket){
        return;
    }

    closesocket(daemon_socket);
	WSACleanup();
    daemon_socket = INVALID_SOCKET;
}

DWORD WINAPI DaemonThreadProc(LPVOID lpParam) {
    struct sockaddr_in local_addr;
    WSADATA wsa_data;    
    int ret;
    fd_set fdr;
    UINT8 buf[1024];
    int rcvdlen;
    int addrlen = sizeof(struct sockaddr_in);

    // Initialize Windows socket library
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    daemon_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(DAEMON_UDP_PORT);
    local_addr.sin_addr.S_un.S_addr = inet_addr(DAEMON_UDP_IP);

    if(bind(daemon_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
		LogWriter::LOGX("[daemon] error:local port 62887 is used!");
        closesocket(daemon_socket);
        system("pause");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.S_un.S_addr = inet_addr(DAEMON_UDP_IP);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DAEMON_UDP_PORT);
    
    FD_ZERO(&fdr);
    FD_SET(daemon_socket, &fdr);

    while(1)
	{
        ret = select(0, &fdr, NULL, NULL, NULL);

        if(ret == 0){
            continue;
        }

        if(FD_ISSET(daemon_socket, &fdr)){
            rcvdlen = recvfrom(daemon_socket, (CHAR*)buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, &addrlen);
            if(rcvdlen > 2){
                UINT16 packet_len = READ_NET_DATA16(buf, 0);
                
                if(packet_len == rcvdlen){
                    dispatch_event(&buf[2], rcvdlen-2);
                }
            }
        }
    }
}