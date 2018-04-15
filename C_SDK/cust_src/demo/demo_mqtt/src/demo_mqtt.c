/*
 * 1、DEMO基于MQTT-3.1.1协议编写，测试用客户端MQTT.fx-1.3.0，简单演示了报文类型1~14的使用方法，
 *   并且启用了会话清理功能，因此也不使用消息重试功能
 * 2、DEMO订阅2个主题，air202/gprs/tx,air202/ctrl，为防止冲突，请用户修改为其他主题
 * 3、用户的客户端上订阅1个主题，air202/gprs/rx，为防止冲突，请用户修改为其他主题，或者使用自己的服务器
 * 4、connect成功后，模块往air202/gprs/tx主题上publish一条登录消息
 * 5、用户往air202/gprs/tx主题上publish数据，模块接收后原样publish到air202/gprs/rx
 * 6、用户往air202/ctrl主题上publish"quit"，模块往air202/gprs/tx主题上publish一条qos0的退出消息，
 * 	    模块取消所有订阅，disconnect，任务停止
 * 7、DEMO所有报文长度限制在1460，可以修改
 * 8、MQTT协议由于经过中转，所以客户端收到模块的返回信息，大概在1~3秒左右都是正常
 * 9、宏定义和全局变量可根据实际情况修改
 */
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "mqttlib.h"

typedef struct {
    UINT8 Type;
    UINT8 Data;
}USER_MESSAGE;

enum
{
	USER_MSG_NETWORK,
	USER_MSG_TIMER,
};

extern T_AMOPENAT_INTERFACE_VTBL* g_s_InterfaceVtbl;
#define DBG_INFO(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG_ERROR(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define SOCKET_CLOSE(A)         if (A >= 0) {close(A);A = -1;}


#define MQTT_RECONNECT_MAX			(8)				//最大重连次数
#define MQTT_PUBLISH_DEFAULT_QOS	MQTT_MSG_QOS2	//模块PUBLISH的默认QOS,QOS1
#define MQTT_PAYLOAD_MAX			(1400)			//有效载荷最大长度1400
#define MQTT_MSG_LEN_MAX			(1460)			//MQTT报文最大长度1460
#define MQTT_HEAD_LEN_MAX			(128)			//MQTT报头最大长度128
#define MQTT_TCP_TO					(30)			//MQTT TCP收发超时30S
#define MQTT_HEAT_TO				(120)			//MQTT心跳周期120S
#define MQTT_TOPIC_LEN					(256)
#ifdef __SSL_ENABLE__
#define MQTT_SSL_URL				"mqtt.test.com"
#define MQTT_SSL_PORT				(18883)
#else
#define MQTT_URL					"lbsmqtt.airm2m.com"
#define MQTT_PORT					(1884)
#endif
/*************************************************************/
const MQTT_SubscribeStruct DemoSub[2] =
{
		{
				.Char = "air202/gprs/tx",
				.Qos = MQTT_SUBSCRIBE_QOS2,
		},
		{
				.Char = "air202/ctrl",
				.Qos = MQTT_SUBSCRIBE_QOS2,
		}
};
const int8_t *DemoPublishTopicGPRS = "air202/gprs/rx";//模块发布到服务器的主题
const int8_t *DemoClientID = NULL;	//如果填写NULL，则由服务器自动分配
//以下变量，可根据实际需要填写NULL，但是必须同时修改CONNECT中的连接标识位，否则会出错
const int8_t *DemoWillTopic = "air202/gprs/rx";
const int8_t *DemoWillMsg = "error offline";
const int8_t *DemoUser = "user";
const int8_t *DemoPasswd = "password";
/*************************************************************/
static HANDLE hTimer;
static HANDLE hSocketTask;
static E_OPENAT_NETWORK_STATE NWState;				//网络状态
static uint8_t MQTTRxBuf[MQTT_MSG_LEN_MAX];			//MQTT接收报文缓存
static uint8_t MQTTTxBuf[MQTT_MSG_LEN_MAX];			//MQTT发送报文缓存
static uint8_t MQTTTempBuf[MQTT_PAYLOAD_MAX];		//MQTT临时数据缓存
static uint8_t MQTTPayload[MQTT_PAYLOAD_MAX];		//MQTT有效载荷缓存
static uint8_t MQTTAnalyzeBuf[MQTT_MSG_LEN_MAX * 2];//MQTT接收需要解析的数据缓存

static Buffer_Struct TxBuffer;
static Buffer_Struct PayloadBuffer;
static uint16_t gPackID = 0;						//全局报文标识符
static uint8_t ToFlag = 0;
static int MQTT_MessageAnalyze(MQTT_HeadStruct *Rxhead, int32_t Socketfd, uint8_t *QuitFlag);
static uint32_t MQTT_Gethostbyname(void)
{
    //域名解析
	ip_addr_t *IP;
    struct hostent *hostentP = NULL;
    char *ipAddr = NULL;

    //获取域名ip信息
    hostentP = gethostbyname(MQTT_URL);

    if (!hostentP)
    {
        DBG_ERROR("gethostbyname %s fail", MQTT_URL);
        return 0;
    }

    // 将ip转换成字符串
    ipAddr = ipaddr_ntoa((const ip_addr_t *)hostentP->h_addr_list[0]);

    DBG_ERROR("gethostbyname %s ip %s", MQTT_URL, ipAddr);
    IP = (ip_addr_t *)hostentP->h_addr_list[0];
    return IP->addr;
}

static int32_t Socket_ConnectServer(void)
{
	uint32_t IP;
    int connErr;
    struct sockaddr_in TCPServerAddr;
	int32_t Socketfd;
	IP = MQTT_Gethostbyname();
	if (IP)
	{
		Socketfd = socket(AF_INET,SOCK_STREAM,0);
	    if (Socketfd < 0)
	    {
	        DBG_ERROR("create tcp socket error");
	        return -1;
	    }
	    // 建立TCP链接
	    memset(&TCPServerAddr, 0, sizeof(TCPServerAddr)); // 初始化服务器地址
	    TCPServerAddr.sin_family = AF_INET;
	    TCPServerAddr.sin_port = htons((unsigned short)MQTT_PORT);
	    TCPServerAddr.sin_addr.s_addr = IP;
	    connErr = connect(Socketfd, (const struct sockaddr *)&TCPServerAddr, sizeof(struct sockaddr));
	    if (connErr < 0)
	    {
	    	DBG_ERROR("tcp connect error %d", socket_errno(Socketfd));
	        close(Socketfd);
	        return -1;
	    }
	    DBG_INFO("[socket] tcp connect success");
	    return Socketfd;
	}
	else
	{
		return -1;
	}
}

static int32_t MQTT_TCPTx(int32_t Socketfd, uint16_t TxLen, uint32_t TimeoutSec)
{
    struct timeval tm;
    fd_set WriteSet;
	int32_t Result;
	Result = send(Socketfd, MQTTTxBuf, TxLen, 0);

	if (Result < 0)
	{
		DBG_ERROR("TCP %d %d", Result, socket_errno(Socketfd));
		return Result;
	}
    FD_ZERO(&WriteSet);
    FD_SET(Socketfd, &WriteSet);
    tm.tv_sec = TimeoutSec;
    tm.tv_usec = 0;
    Result = select(Socketfd + 1, NULL, &WriteSet, NULL, &tm);
    if(Result > 0)
    {
		DBG_INFO("TCP TX OK! %dbyte", TxLen);
		return Result;
    }
    else
    {
        DBG_ERROR("TCP TX ERROR");
        return -1;
    }
}

static int32_t MQTT_TCPRx(int32_t Socketfd, uint32_t TimeoutSec)
{
    struct timeval tm;
    fd_set ReadSet;
	int32_t Result;
    FD_ZERO(&ReadSet);
    FD_SET(Socketfd, &ReadSet);
    tm.tv_sec = TimeoutSec;
    tm.tv_usec = 0;
    Result = select(Socketfd + 1, &ReadSet, NULL, NULL, &tm);
    if(Result > 0)
    {
    	Result = recv(Socketfd, MQTTRxBuf, sizeof(MQTTRxBuf), 0);
        if(Result == 0)
        {
        	DBG_ERROR("socket close!");
            return -1;
        }
        else if(Result < 0)
        {
        	DBG_ERROR("recv error %d", socket_errno(Socketfd));
            return -1;
        }
		return Result;
    }
    return Result;
}

//MQTT报文预先处理
static int32_t MQTT_RxPreDeal(MQTT_HeadStruct *Rxhead, int32_t RxLen)
{
	uint8_t *Payload = NULL;
	uint32_t PayloadLen;
	uint32_t DealLen;
	Rxhead->Data = MQTTTempBuf;
	Payload = MQTT_DecodeMsg(Rxhead, MQTT_HEAD_LEN_MAX, &PayloadLen, MQTTRxBuf, RxLen, &DealLen);
	if ((uint32_t)Payload != INVALID_HANDLE_VALUE)
	{
		if (DealLen != RxLen)
		{
			DBG_INFO("more data need deal,but demo do not deal! %u %u", DealLen, RxLen);
		}
		Rxhead->Data[Rxhead->DataLen] = 0;
		if (Payload && PayloadLen)
		{
			memcpy(PayloadBuffer.Data, Payload, PayloadLen);
			PayloadBuffer.Pos = PayloadLen;
		}
	}
	else
	{
		DBG_ERROR("MQTT MSG ERROR!");
		return -1;
	}
	return 0;
}

//MQTT CONNECT过程
static int32_t MQTT_Connect(int32_t Socketfd)
{
	uint32_t TxLen;
	int32_t RxLen;
	MQTT_HeadStruct Rxhead;

	TxBuffer.Pos = 0;
	PayloadBuffer.Pos = 0;
	TxLen = MQTT_ConnectMsg(&TxBuffer, &PayloadBuffer,
			MQTT_CONNECT_FLAG_CLEAN|MQTT_CONNECT_FLAG_WILL|MQTT_CONNECT_FLAG_WILLQOS1|MQTT_CONNECT_FLAG_USER|MQTT_CONNECT_FLAG_PASSWD,
			MQTT_HEAT_TO * 2, DemoClientID, DemoWillTopic, DemoUser, DemoPasswd, (uint8_t *)DemoWillMsg, strlen(DemoWillMsg));
	if (MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO) < 0)
	{
		return -1;
	}
	RxLen = MQTT_TCPRx(Socketfd, MQTT_TCP_TO);
	if (RxLen <= 0)
	{
		return -1;
	}
	if (MQTT_RxPreDeal(&Rxhead, RxLen) < 0)
	{
		return -1;
	}

	if (Rxhead.Cmd != MQTT_CMD_CONNACK)
	{
		DBG_ERROR("UNEXPECT CMD %02x", Rxhead.Cmd);
		return -1;
	}
	if (Rxhead.Data[1])
	{
		DBG_ERROR("CONNACK FAIL %02x %02x", Rxhead.Data[0], Rxhead.Data[1]);
		return -1;
	}
	return 0;
}

//MQTT订阅主题过程
static int32_t MQTT_Subscribe(int32_t Socketfd)
{
	int i;
	uint32_t TxLen;
	int32_t RxLen;
	MQTT_HeadStruct Rxhead;
	TxBuffer.Pos = 0;
	PayloadBuffer.Pos = 0;
	gPackID++;
	TxLen = MQTT_SubscribeMsg(&TxBuffer, &PayloadBuffer, gPackID, (MQTT_SubscribeStruct *)DemoSub, sizeof(DemoSub)/sizeof(MQTT_SubscribeStruct));
	if (MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO) < 0)
	{
		return -1;
	}
	RxLen = MQTT_TCPRx(Socketfd, MQTT_TCP_TO);
	if (RxLen <= 0)
	{
		return -1;
	}
	if (MQTT_RxPreDeal(&Rxhead, RxLen) < 0)
	{
		return -1;
	}
	if (Rxhead.Cmd != MQTT_CMD_SUBACK)
	{
		DBG_ERROR("UNEXPECT CMD %02x", Rxhead.Cmd);
		return -1;
	}
	if (Rxhead.PackID != gPackID)
	{
		DBG_ERROR("gPackID ERROR %u %u", (uint32_t)Rxhead.PackID, (uint32_t)gPackID);
		return -1;
	}

	//订阅了多少个主题，就有多少个回复字节
	for (i = 0; i < sizeof(DemoSub)/sizeof(MQTT_SubscribeStruct);i++)
	{
		switch (PayloadBuffer.Data[i])
		{
		case 0:
		case 1:
		case 2:
			DBG_INFO("Subscribe %d ok %02x", i,PayloadBuffer.Data[0]);
			break;
		default:
			DBG_ERROR("Subscribe fail %02x", PayloadBuffer.Data[0]);
			return -1;
		}
	}
	return 1;
}

//MQTT 取消订阅主题过程
static int32_t MQTT_Unsubscribe(int32_t Socketfd)
{
	uint32_t TxLen;
	int32_t RxLen;
	MQTT_HeadStruct Rxhead;
	TxBuffer.Pos = 0;
	PayloadBuffer.Pos = 0;
	gPackID++;
	TxLen = MQTT_UnSubscribeMsg(&TxBuffer, &PayloadBuffer, gPackID, (MQTT_SubscribeStruct *)DemoSub, sizeof(DemoSub)/sizeof(MQTT_SubscribeStruct));
	if (MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO) < 0)
	{
		return -1;
	}
	RxLen = MQTT_TCPRx(Socketfd, MQTT_TCP_TO);
	if (RxLen <= 0)
	{
		return -1;
	}
	if (MQTT_RxPreDeal(&Rxhead, RxLen) < 0)
	{
		return -1;
	}
	if (Rxhead.Cmd != MQTT_CMD_UNSUBACK)
	{
		DBG_ERROR("UNEXPECT CMD %02x", Rxhead.Cmd);
		return -1;
	}
	if (Rxhead.PackID != gPackID)
	{
		DBG_ERROR("gPackID ERROR %u %u", (uint32_t)Rxhead.PackID, (uint32_t)gPackID);
		return -1;
	}
	DBG_INFO("Unsubscribe ok");
	return 0;
}

//MQTT 心跳过程
static int32_t MQTT_Heart(int32_t Socketfd)
{
	uint32_t TxLen;
	int32_t RxLen;
	MQTT_HeadStruct Rxhead;
	TxBuffer.Pos = 0;
	TxLen = MQTT_SingleMsg(&TxBuffer, MQTT_CMD_PINGREQ);
	if (MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO) < 0)
	{
		return -1;
	}
	return 0;
}

int MQTT_MessageAnalyze(MQTT_HeadStruct *Rxhead, int32_t Socketfd, uint8_t *QuitFlag)
{
	int32_t TxLen;
	uint8_t Flag;
	switch (Rxhead->Cmd)
	{
	case MQTT_CMD_PUBLISH:
		Flag = Rxhead->Flag;
		if (!memcmp(DemoSub[0].Char, Rxhead->Data, Rxhead->DataLen))
		{
			DBG_INFO("from sub 0");
			TxBuffer.Pos = 0;
			gPackID++;
			TxLen = MQTT_PublishMsg(&TxBuffer, MQTT_PUBLISH_DEFAULT_QOS, gPackID,
					DemoPublishTopicGPRS, PayloadBuffer.Data, PayloadBuffer.Pos);
			if (TxLen <= 0)
			{
				return -1;
			}
			else
			{
				MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO);
			}
		}
		else if (!memcmp(DemoSub[1].Char, Rxhead->Data, Rxhead->DataLen))
		{
			DBG_INFO("from sub 1");
			if (!memcmp(PayloadBuffer.Data, "quit", 4))
			{
				*QuitFlag = 1;
			}
		}

		switch (Flag & MQTT_MSG_QOS_MASK)
		{
		case 0:
			break;
		case MQTT_MSG_QOS1:

			TxBuffer.Pos = 0;
			TxLen = MQTT_PublishCtrlMsg(&TxBuffer, MQTT_CMD_PUBACK, Rxhead->PackID);
			if (TxLen <= 0)
			{
				return -1;
			}
			else
			{
				return MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO);
			}
			break;
		case MQTT_MSG_QOS2:

			TxBuffer.Pos = 0;
			TxLen = MQTT_PublishCtrlMsg(&TxBuffer, MQTT_CMD_PUBREC, Rxhead->PackID);
			if (TxLen <= 0)
			{
				return -1;
			}
			else
			{
				return MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO);
			}
			break;
		}
		break;
	case MQTT_CMD_PUBACK:
		//qos1 OK
		DBG_INFO("PUBACK");
		break;

	case MQTT_CMD_PUBREC:
		DBG_INFO("PUBREC");
		DBG_INFO("%d",Rxhead->PackID);
		TxBuffer.Pos = 0;
		TxLen = MQTT_PublishCtrlMsg(&TxBuffer, MQTT_CMD_PUBREL, Rxhead->PackID);
		if (TxLen <= 0)
		{
			return -1;
		}
		else
		{
			return MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO);
		}
		break;

	case MQTT_CMD_PUBREL:
		TxBuffer.Pos = 0;
		DBG_INFO("PUBREL");
		DBG_INFO("%d",Rxhead->PackID);
		TxLen = MQTT_PublishCtrlMsg(&TxBuffer, MQTT_CMD_PUBCOMP, Rxhead->PackID);
		if (TxLen <= 0)
		{
			return -1;
		}
		else
		{
			return MQTT_TCPTx(Socketfd, TxLen, MQTT_TCP_TO);
		}
		break;
	case MQTT_CMD_PUBCOMP:
		DBG_INFO("PUBCOMP");
		break;
	case MQTT_CMD_PINGRESP:
		break;

	case MQTT_CMD_CONNACK:

		if (Rxhead->Data[1])
		{
			DBG_ERROR("CONNACK FAIL %02x %02x", Rxhead->Data[0], Rxhead->Data[1]);
		}
		else
		{

		}
		break;

	case MQTT_CMD_SUBACK:

		switch (PayloadBuffer.Data[0])
		{
		case 0:
		case 1:
		case 2:
			DBG_INFO("Subscribe ok %02x", PayloadBuffer.Data[0]);

			break;
		default:
			DBG_ERROR("Subscribe fail %02x", PayloadBuffer.Data[0]);
			break;
		}
		break;

	case MQTT_CMD_UNSUBACK:
		switch (PayloadBuffer.Data[0])
		{
		case 0:
		case 1:
		case 2:
			DBG_INFO("UnSubscribe ok %02x", PayloadBuffer.Data[0]);

			break;
		default:
			DBG_ERROR("UnSubscribe fail %02x", PayloadBuffer.Data[0]);
			break;
		}
		break;
	}
	return 0;
}

static void MQTT_Task(PVOID pParameter)
{
	USER_MESSAGE*    msg;
	uint8_t ReConnCnt, Error, Quit;
	int32_t RxLen = 0;
	int32_t Socketfd = -1;
	int32_t TopicSN, Result;

	MQTT_HeadStruct Rxhead;
	uint8_t *Payload = NULL;
	uint32_t PayloadLen;
	uint32_t DealLen = 0;
	uint32_t DummyLen = 0xffffffff;
	uint32_t LastRxByte;
	int8_t Topic[MQTT_TOPIC_LEN];
	uint8_t QuitFlag;
	TxBuffer.Data = MQTTTxBuf;
	TxBuffer.MaxLen = sizeof(MQTTTxBuf);
	TxBuffer.Pos = 0;
	PayloadBuffer.Data = MQTTPayload;
	PayloadBuffer.MaxLen = sizeof(MQTTPayload);
	PayloadBuffer.Pos = 0;
	ReConnCnt = 0;

	Quit = 0;
	while (!Quit)
	{
		SOCKET_CLOSE(Socketfd);
		iot_os_sleep(5000);	//这里最好使用timer来延迟，demo简化使用
		iot_os_stop_timer(hTimer);
		iot_os_start_timer(hTimer, 90*1000);//90秒内如果没有激活APN，重启模块
		ToFlag = 0;
		while (NWState != OPENAT_NETWORK_LINKED)
		{
			iot_os_wait_message(hSocketTask, (PVOID)&msg);
	        switch(msg->Type)
	        {
			case USER_MSG_TIMER:
				DBG_ERROR("network wait too long!");
				iot_os_sleep(500);
				iot_os_restart();
				break;
			default:
				break;
	        }
	        iot_os_free(msg);
		}
		iot_os_stop_timer(hTimer);
		DBG_INFO("start connect server");
		Socketfd = Socket_ConnectServer();
		if (Socketfd > 0)
		{
			ReConnCnt = 0;
		}
		else
		{
			ReConnCnt++;
			DBG_ERROR("retry %dtimes", ReConnCnt);
			if (ReConnCnt > MQTT_RECONNECT_MAX)
			{
				iot_os_restart();
				while (1)
				{
					iot_os_sleep(5000);
				}
			}
			continue;
		}

		DBG_INFO("MQTT CONNECT Start");
		if (MQTT_Connect(Socketfd) < 0)
		{
			DBG_INFO("MQTT CONNECT Fail");
			continue;
		}
		DBG_INFO("MQTT SUBSCRIBE Start");
		if (MQTT_Subscribe(Socketfd) < 0)
		{
			DBG_INFO("MQTT SUBSCRIBE Fail");
			continue;
		}

		DBG_INFO("MQTT PUBLISH hello Start");
		strcpy(PayloadBuffer.Data, "hello, this is air202 mqtt demo!");
		PayloadBuffer.Pos = strlen(PayloadBuffer.Data);
		TxBuffer.Pos = 0;
		gPackID++;
		MQTT_PublishMsg(&TxBuffer, MQTT_PUBLISH_DEFAULT_QOS, gPackID,
							DemoPublishTopicGPRS, PayloadBuffer.Data, PayloadBuffer.Pos);
		if (MQTT_TCPTx(Socketfd, TxBuffer.Pos, MQTT_TCP_TO) <= 0)
		{
			DBG_INFO("MQTT send first message fail");
			continue;
		}

		iot_os_start_timer(hTimer, MQTT_HEAT_TO*1000);//启动心跳计时
		ToFlag = 0;
		Error = 0;
		LastRxByte = 0;
		while(!Error && !Quit)
		{
			RxLen = 0;
			RxLen = MQTT_TCPRx(Socketfd, 1);
			if (RxLen > 0)
			{
				DBG_INFO("%d", LastRxByte);
				memcpy(MQTTAnalyzeBuf + LastRxByte, MQTTRxBuf, RxLen);
				DealLen = 0;

				RxLen += LastRxByte;
				LastRxByte = 0;
				QuitFlag = 0;
				DBG_INFO("%d", RxLen);
				do
				{
					DummyLen = 0xffffffff;
					memset(Topic, 0, sizeof(Topic));
					Rxhead.Data = Topic;
					Payload = MQTT_DecodeMsg(&Rxhead, MQTT_HEAD_LEN_MAX, &PayloadLen, MQTTAnalyzeBuf + DealLen, RxLen - DealLen, &DummyLen);
					//DBG_INFO("%d %d %d", DummyLen, DealLen, RxLen - DealLen);
					if ((uint32_t)Payload != INVALID_HANDLE_VALUE)
					{
						Rxhead.Data[Rxhead.DataLen] = 0;
						if (Payload && PayloadLen)
						{
							memcpy(PayloadBuffer.Data, Payload, PayloadLen);
							PayloadBuffer.Pos = PayloadLen;
						}
						if (MQTT_MessageAnalyze(&Rxhead, Socketfd, &QuitFlag) < 0)
						{
							Error = 1;
							break;
						}
						DealLen += DummyLen;

						if (QuitFlag)
						{
							DBG_INFO("MQTT PUBLISH goodbyte");
							strcpy(PayloadBuffer.Data, "goodbyte");
							PayloadBuffer.Pos = strlen(PayloadBuffer.Data);
							TxBuffer.Pos = 0;
							MQTT_PublishMsg(&TxBuffer, 0, gPackID,
												DemoPublishTopicGPRS, PayloadBuffer.Data, PayloadBuffer.Pos);
							MQTT_TCPTx(Socketfd, TxBuffer.Pos, MQTT_TCP_TO);
							MQTT_Unsubscribe(Socketfd);
							MQTT_SingleMsg(&TxBuffer, MQTT_CMD_DISCONNECT);
							MQTT_TCPTx(Socketfd, TxBuffer.Pos, MQTT_TCP_TO);
							Quit = 1;
							break;
						}
					}
					else
					{
						if (DummyLen)
						{
							DBG_ERROR("MQTT MSG ERROR!");
						}
						else
						{
							LastRxByte = RxLen - DealLen;
							memcpy(MQTTAnalyzeBuf, MQTTAnalyzeBuf + DealLen, LastRxByte);
							DealLen = RxLen;
							DBG_INFO("%d", LastRxByte);
						}
					}
				}while (DealLen != RxLen);
				continue;
			}
			else if (RxLen < 0)
			{
				Error = 1;
				continue;
			}
			else
			{
				if (ToFlag)
				{
					iot_os_wait_message(hSocketTask, (PVOID)&msg);
					switch(msg->Type)
					{
					case USER_MSG_TIMER:
						ToFlag = 0;
						Result = MQTT_Heart(Socketfd);
						if (Result < 0)
						{
							Error = 1;
						}
						else
						{
							iot_os_start_timer(hTimer, MQTT_HEAT_TO*1000);//启动心跳计时
						}
						break;
					default:
						if (NWState != OPENAT_NETWORK_LINKED)
						{
							Error = 1;
						}
						break;
					}
					iot_os_free(msg);
				}
			}
		}
	}
	DBG_INFO("MQTT QUIT!");
	iot_os_stop_timer(hTimer);
	SOCKET_CLOSE(Socketfd);
	while (1)
	{
		iot_os_sleep(43200 * 1000);
	}
}

static void MQTT_NetworkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
	USER_MESSAGE * Msg = iot_os_malloc(sizeof(USER_MESSAGE));
    T_OPENAT_NETWORK_CONNECT networkparam;
    if (state == OPENAT_NETWORK_READY)
    {
    	memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    	memcpy(networkparam.apn, "CMNET", strlen("CMNET"));
    	iot_network_connect(&networkparam);
    }

    Msg->Type = USER_MSG_NETWORK;
    DBG_INFO("%d", state);
    if (NWState != state)
    {
    	DBG_INFO("network ind state %d -> %d", NWState, state);
    	NWState = state;
    }
    iot_os_send_message(hSocketTask, (PVOID)Msg);

}

static void MQTT_TimerHandle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
	USER_MESSAGE *Msg = iot_os_malloc(sizeof(USER_MESSAGE));
	ToFlag = 1;
	Msg->Type = USER_MSG_TIMER;
	iot_os_send_message(hSocketTask, (PVOID)Msg);
	iot_os_stop_timer(hTimer);

}

VOID app_main(VOID)
{
    iot_network_set_cb(MQTT_NetworkIndCallBack);
	hSocketTask = iot_os_create_task(MQTT_Task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_socket_mqtt");
	NWState = OPENAT_NETWORK_DISCONNECT;
	hTimer = iot_os_create_timer(MQTT_TimerHandle, NULL);
	iot_pmd_exit_deepsleep();
}
