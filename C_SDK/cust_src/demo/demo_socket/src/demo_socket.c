#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"


#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)
typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_SOCKET_MESSAGE;

extern T_AMOPENAT_INTERFACE_VTBL* g_s_InterfaceVtbl;
#define socket_dbg iot_debug_print

#define DEMO_SERVER_TCP_IP "36.7.87.100"
#define DEMO_SERVER_TCP_PORT 20000

#define DEMO_SERVER_UDP_IP "36.7.87.100"
#define DEMO_SERVER_UDP_PORT 20001                                

static HANDLE g_s_socket_task;

static int demo_socket_tcp_recv(int socketfd)
{
    unsigned char recv_buff[64] = {0};
    int recv_len;

    // TCP 接受数据
    recv_len = recv(socketfd, recv_buff, sizeof(recv_buff), 0);
    socket_dbg("[socket] tcp recv result %d data %s", recv_len, recv_buff);
   
    return recv_len;
}

static int demo_socket_tcp_send(int socketfd)
{
    int send_len;

	char data[1024] = {0};

	memset(data, 0x32, 1024);

    // TCP 发送数据
    send_len = send(socketfd, data, 1024, 0);
    socket_dbg("[socket] tcp send datd result = %d", send_len);
    return send_len;
}


static int demo_socket_tcp_connect_server(void)
{
    int socketfd;
    int connErr;
    struct sockaddr_in tcp_server_addr; 
    
    // 创建tcp socket
    socketfd = socket(AF_INET,SOCK_STREAM,0);
    if (socketfd < 0)
    {
        socket_dbg("[socket] create tcp socket error");
        return -1;
    }
       
    socket_dbg("[socket] create tcp socket success");
    
    // 建立TCP链接
    memset(&tcp_server_addr, 0, sizeof(tcp_server_addr)); // 初始化服务器地址  
    tcp_server_addr.sin_family = AF_INET;  
    tcp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_TCP_PORT);  
    inet_aton(DEMO_SERVER_TCP_IP,&tcp_server_addr.sin_addr);

    socket_dbg("[socket] tcp connect to addr %s", DEMO_SERVER_TCP_IP);
    connErr = connect(socketfd, (const struct sockaddr *)&tcp_server_addr, sizeof(struct sockaddr));
    

    if (connErr < 0)
    {
        socket_dbg("[socket] tcp connect error %d", socket_errno(socketfd));
        close(socketfd);
        return -1;
    }
    socket_dbg("[socket] tcp connect success");

    return socketfd;
}

static void demo_socket_tcp_client()
{
    int socketfd, ret, err;
    struct timeval tm;
    fd_set readset;
    int optLen = 4, sendBufSize;

    tm.tv_sec = 2;
    tm.tv_usec = 0;

    socketfd = demo_socket_tcp_connect_server();

    if (socketfd >= 0)
    {
        ret = getsockopt(socketfd, SOL_SOCKET, SO_SNDWNDBUF, (void*)&sendBufSize, (socklen_t*)&optLen);

        if(ret < 0)
        {
            socket_dbg("[socket] getsockopt error = %d", ret);
        }
        else
        {
            socket_dbg("[socket] getsockopt original SO_SNDBUF = %d", sendBufSize);
        }
        while(1)
        {
            ret = demo_socket_tcp_send(socketfd);
            if(ret < 0)
            {
            	err = socket_errno(socketfd);
				socket_dbg("[socket] send last error %d", err);
            	if(err != EWOULDBLOCK)
            	{
                	//break;
            	}
				else
				{
					iot_os_sleep(10);
					continue;
				}
            }

            ret = getsockopt(socketfd, SOL_SOCKET, SO_SNDWNDBUF, (void*)&sendBufSize, (socklen_t*)&optLen);

            if(ret < 0)
            {
                socket_dbg("[socket] getsockopt error = %d", ret);
            }
            else
            {
                socket_dbg("[socket] getsockopt after send SO_SNDBUF = %d", sendBufSize);
            }
            FD_ZERO(&readset);
            FD_SET(socketfd, &readset);
            ret = select(socketfd+1, &readset, NULL, NULL, &tm);
            if(ret > 0)
            {
    			ret = demo_socket_tcp_recv(socketfd);
                if(ret == 0)
                {
                    socket_dbg("[socket] recv close");
                    iot_os_sleep(1000);
                }
                else if(ret < 0)
                {
                    socket_dbg("[socket] recv error %d", socket_errno(socketfd));
                    iot_os_sleep(1000);
                }
            }
            else if(ret == 0)
            {
                socket_dbg("[socket] select timeout");
            }

            ret = getsockopt(socketfd, SOL_SOCKET, SO_SNDWNDBUF, (void*)&sendBufSize, (socklen_t*)&optLen);

            if(ret < 0)
            {
                socket_dbg("[socket] getsockopt error = %d", ret);
            }
            else
            {
                socket_dbg("[socket] getsockopt after recv SO_SNDBUF = %d", sendBufSize);
            }
        }
        close(socketfd);
    }
}


static int demo_socket_udp_connect_server(void)
{
    int socketfd;	
    // 创建tcp socket
    socketfd = socket(AF_INET,SOCK_DGRAM,0);
    if (socketfd < 0)
    {
        socket_dbg("[socket] create udp socket error");
        return -1;
    }
    socket_dbg("[socket] create udp socket success");

    return socketfd;
}


static int demo_socket_udp_send(int socketfd)
{
    int send_len;
	struct sockaddr_in udp_server_addr; 

	memset(&udp_server_addr, 0, sizeof(udp_server_addr)); // 初始化服务器地址  
    udp_server_addr.sin_family = AF_INET;  
    udp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_UDP_PORT);  
    inet_aton(DEMO_SERVER_UDP_IP,&udp_server_addr.sin_addr);

    // UDP 发送数据
    send_len = sendto(socketfd, "hello i'm client", strlen("hello i'm client"), 0,
    				  (struct sockaddr*)&udp_server_addr, sizeof(struct sockaddr));
    socket_dbg("[socket] udp send [hello i'm client] result = %d", send_len);
    return send_len;
}

static int demo_socket_udp_recv(int socketfd)
{
    unsigned char recv_buff[64] = {0};
    int recv_len;
	socklen_t udp_server_len;

	struct sockaddr_in udp_server_addr; 

	memset(&udp_server_addr, 0, sizeof(udp_server_addr)); // 初始化服务器地址  
    udp_server_addr.sin_family = AF_INET;  
    udp_server_addr.sin_port = htons((unsigned short)DEMO_SERVER_UDP_PORT);  
    inet_aton(DEMO_SERVER_UDP_IP,&udp_server_addr.sin_addr);
	udp_server_len = sizeof(udp_server_addr);

    // UDP 接受数据
    recv_len = recvfrom(socketfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&udp_server_addr, &udp_server_len);
    socket_dbg("[socket] udp recv result %d data %s", recv_len, recv_buff);
   
    return recv_len;
}


static void demo_socket_udp_client()
{
    int socketfd, ret, err, count;
    

    socketfd = demo_socket_udp_connect_server();

    if (socketfd >= 0)
    {
        while(1)
        {
            ret = demo_socket_udp_send(socketfd);
            if(ret < 0)
            {
            	err = socket_errno(socketfd);
				socket_dbg("[socket] send last error %d", err);
            	if(err != EWOULDBLOCK)
            	{
                	break;
            	}
				else
				{
					iot_os_sleep(200);
					continue;
				}
            }
			//阻塞读取
            ret = demo_socket_udp_recv(socketfd);
            if(ret <= 0)
            {
                socket_dbg("[socket] recv error %d", socket_errno(socketfd));
                break;
            }

			if(++count >= 5)
			{
				socket_dbg("[socket] udp loop end");
				break;
			}
        }
    }
}

static void demo_gethostbyname(void)
{
    //域名解析

    char *name = "www.baidu.com";
    struct hostent *hostentP = NULL;
    char *ipAddr = NULL;

    //获取域名ip信息
    hostentP = gethostbyname(name);

    if (!hostentP)
    {
        socket_dbg("[socket] gethostbyname %s fail", name);
        return;
    }

    // 将ip转换成字符串
    ipAddr = ipaddr_ntoa((const ip_addr_t *)hostentP->h_addr_list[0]);
    
    socket_dbg("[socket] gethostbyname %s ip %s", name, ipAddr);
}


static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_SOCKET_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_SOCKET_MESSAGE));
    socket_dbg("[socket] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = SOCKET_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_socket_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = SOCKET_MSG_NETWORK_READY;
        iot_os_send_message(g_s_socket_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

static void demo_socket_task(PVOID pParameter)
{
    DEMO_SOCKET_MESSAGE*    msg;
    socket_dbg("[socket] wait network ready....");
    BOOL sock = FALSE;

    while(1)
    {
        iot_os_wait_message(g_s_socket_task, (PVOID)&msg);

        switch(msg->type)
        {
            case SOCKET_MSG_NETWORK_READY:
                socket_dbg("[socket] network connecting....");
                demo_network_connetck();
                break;
            case SOCKET_MSG_NETWORK_LINKED:
                socket_dbg("[socket] network connected");
                if(!sock)
                {
				demo_gethostbyname(); 
				//demo_socket_udp_client();
     			demo_socket_tcp_client();
                sock = TRUE;
                }
                break;
        }

        iot_os_free(msg);
    }
}

void demo_socket_init(void)
{ 
    socket_dbg("[socket] demo_socket_init");

    //注册网络状态回调函数
    iot_network_set_cb(demo_networkIndCallBack);

    g_s_socket_task = iot_os_create_task(demo_socket_task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_socket");
}

VOID app_main(VOID)
{
  iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    demo_socket_init();
}
