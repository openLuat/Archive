#include "string.h"
#include "mqttlib.h"
#include "iot_socket.h"
#include "iot_debug.h"
#define DBG_INFO(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define DBG_ERROR(X, Y...)	iot_debug_print("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
static uint32_t MQTT_Pow(uint32_t x, uint8_t y)
{
	uint32_t res = 1;
	uint8_t i;
	if (!y)
	{
		return 1;
	}
	for(i = 0; i < y; i++)
	{
		res = res * x;
	}
	return res;
}

uint32_t MQTT_AddUFT8String(Buffer_Struct *Buf, const int8_t *String)
{
	uint16_t Strlen = strlen(String);
	uint16_t wTemp = htons(Strlen);
	if (Buf->MaxLen >= (Buf->Pos + Strlen + 2))
	{
		memcpy(Buf->Data + Buf->Pos, &wTemp, 2);
		memcpy(Buf->Data + Buf->Pos + 2, String, Strlen);
		Buf->Pos += Strlen + 2;
		return Buf->Pos;
	}
	else
	{
		DBG_ERROR("TxBuf len is not enough %u %u", Buf->MaxLen, Buf->Pos + Strlen + 2);
		return 0;
	}
}

uint32_t MQTT_EncodeMsg(MQTT_HeadStruct *Head, uint8_t *Payload, uint32_t PayloadLen, Buffer_Struct *TxBuf)
{
	uint32_t MsgLen = Head->DataLen + PayloadLen;
	uint8_t AddPackID = 0;
	if (TxBuf->MaxLen < (Head->DataLen + PayloadLen + 7))
	{
		DBG_ERROR("TxBuf len is not enough %u %u", TxBuf->MaxLen, Head->DataLen + PayloadLen + 7);
		return 0;
	}

	if (MsgLen > 256 * 1024 * 1024)
	{
		DBG_ERROR("%u", MsgLen);
		return 0;
	}
	switch (Head->Cmd)
	{
	case MQTT_CMD_PUBLISH:
		TxBuf->Data[0] = (Head->Cmd << 4) | Head->Flag;
		if (TxBuf->Data[0] & MQTT_MSG_QOS_MASK)
		{
			AddPackID = 1;
			MsgLen += 2;
		}
		break;
	case MQTT_CMD_PUBREL:
	case MQTT_CMD_SUBSCRIBE:
	case MQTT_CMD_UNSUBSCRIBE:
		TxBuf->Data[0] = (Head->Cmd << 4) | MQTT_MSG_QOS1;
		AddPackID = 1;
		MsgLen += 2;
		break;
	case MQTT_CMD_PUBACK:
	case MQTT_CMD_PUBREC:
	case MQTT_CMD_PUBCOMP:
		TxBuf->Data[0] = (Head->Cmd << 4) ;
		AddPackID = 1;
		MsgLen += 2;
		break;
	default:
		TxBuf->Data[0] = (Head->Cmd << 4);
		break;
	}

	TxBuf->Pos = 1;
	do
	{
		TxBuf->Data[TxBuf->Pos] = MsgLen % 128;
		MsgLen = MsgLen / 128;
		if (MsgLen > 0)
		{
			TxBuf->Data[TxBuf->Pos] |= 0x80;
		}
		TxBuf->Pos++;
	}while((MsgLen > 0) && (TxBuf->Pos <= 4));

	if (Head->DataLen)
	{
		if (Head->Data)
		{
			memcpy(TxBuf->Data + TxBuf->Pos, Head->Data, Head->DataLen);
			TxBuf->Pos += Head->DataLen;
		}
		else if (Head->String)
		{
			if (!MQTT_AddUFT8String(TxBuf, Head->String))
			{
				DBG_ERROR("!");
				return 0;
			}
		}
		else
		{
			DBG_ERROR("datalen > 0,but no data");
			return 0;
		}
	}

	if (AddPackID)
	{
		memcpy(TxBuf->Data + TxBuf->Pos, &Head->PackID, 2);
		TxBuf->Pos += 2;
	}


	if (Payload && PayloadLen)
	{
		memcpy(TxBuf->Data + TxBuf->Pos, Payload, PayloadLen);
		TxBuf->Pos += PayloadLen;
	}

	return TxBuf->Pos;
}

uint8_t* MQTT_DecodeMsg(MQTT_HeadStruct *Head, uint32_t HeadDataLenMax, uint32_t *PayloadLen, uint8_t *RxBuf, uint32_t RxLen, uint32_t *DealLen)
{
	uint32_t MsgLen = 0;
	uint32_t HeadDataLen = 0;
	uint32_t Pos;
	uint8_t *Payload = NULL;
	if (HeadDataLenMax < 2)
	{
		DBG_ERROR("too few head data len max %u", HeadDataLenMax);
		return (uint8_t *)INVALID_HANDLE_VALUE;
	}

	Head->Cmd = RxBuf[0] >> 4;
	Head->Flag = RxBuf[0] & 0x0f;
	if ( (Head->Flag & MQTT_MSG_QOS_MASK) == MQTT_MSG_QOS_MASK)
	{
		DBG_ERROR("MSG QOS %02x", Head->Flag);
		return (uint8_t *)INVALID_HANDLE_VALUE;
	}
	Pos = 1;
	do
	{
		MsgLen += (RxBuf[Pos] & 0x7f) * MQTT_Pow(128, Pos - 1);
		if (RxBuf[Pos] & 0x80)
		{
			if ( (Pos >= RxLen) || (Pos >= 4) )
			{
				DBG_ERROR("%u %u %02x", RxLen, Pos, RxBuf[Pos]);
				return (uint8_t *)INVALID_HANDLE_VALUE;
			}
			else
			{
				Pos++;
			}
		}
		else
		{
			Pos++;
			break;
		}

	}while ( (Pos < RxLen) && (Pos <= 4) );

	if ( (MsgLen + Pos) <= RxLen)
	{
		*DealLen = MsgLen + Pos;
	}
	else
	{
		DBG_ERROR("%u %u %u", MsgLen, Pos, RxLen);
		*DealLen = 0;
		return (uint8_t *)INVALID_HANDLE_VALUE;
	}

	switch (Head->Cmd)
	{


	case MQTT_CMD_PUBLISH:
		//获取主题
		HeadDataLen = RxBuf[Pos];
		HeadDataLen = (HeadDataLen << 8) + RxBuf[Pos + 1];
		Pos += 2;
		if (HeadDataLen > HeadDataLenMax)
		{
			return (uint8_t *)INVALID_HANDLE_VALUE;
		}
		Head->DataLen = HeadDataLen;
		HeadDataLen = 2;

		memcpy(Head->Data, &RxBuf[Pos], Head->DataLen);
		Pos += Head->DataLen;
		HeadDataLen += Head->DataLen;

		if (Head->Flag & MQTT_MSG_QOS_MASK)
		{
			memcpy(&Head->PackID, RxBuf + Pos, 2);
			Head->PackID = htons(Head->PackID);
			Pos += 2;
			HeadDataLen += 2;
		}

		if (MsgLen > HeadDataLen)
		{
			Payload = &RxBuf[Pos];
			*PayloadLen = (MsgLen - HeadDataLen);
		}
		else
		{
			*PayloadLen = 0;
		}
		break;

	case MQTT_CMD_PINGRESP:
		if (MsgLen)
		{
			return (uint8_t *)INVALID_HANDLE_VALUE;
		}
		Head->DataLen = 0;
		*PayloadLen = 0;
		break;

	case MQTT_CMD_CONNACK:
		Head->DataLen = 2;
		memcpy(Head->Data, &RxBuf[Pos], Head->DataLen);
		*PayloadLen = 0;
		break;
	case MQTT_CMD_PUBACK:
	case MQTT_CMD_PUBREC:
	case MQTT_CMD_PUBREL:
	case MQTT_CMD_PUBCOMP:
	case MQTT_CMD_UNSUBACK:
		if ( (MsgLen != 2) || (Pos != 2))
		{
			DBG_ERROR("%u %u", MsgLen, Pos);
		}

		memcpy(&Head->PackID, RxBuf + Pos, 2);
		Head->PackID = htons(Head->PackID);
		Head->DataLen = 0;
		*PayloadLen = 0;
		break;
	case MQTT_CMD_SUBACK:
		if ( (Pos != 2) )
		{
			DBG_ERROR("%u", Pos);
		}
		memcpy(&Head->PackID, RxBuf + Pos, 2);
		Head->PackID = htons(Head->PackID);
		Head->DataLen = 0;
		Pos += 2;
		Payload = &RxBuf[Pos];
		*PayloadLen = (MsgLen - 2);
		break;
	default:
		return (uint8_t *)INVALID_HANDLE_VALUE;
	}
	return Payload;
}

uint32_t MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint8_t Flag, uint16_t KeepTime,
		const int8_t *ClientID,
		const int8_t *WillTopic,
		const int8_t *User,
		const int8_t *Passwd,
		uint8_t *WillMsgData, uint16_t WillMsgLen)
{
	uint8_t MsgHeadBuf[10] = {0, 4, 'M', 'Q', 'T', 'T', 4, 0, 0, 0};
	MQTT_HeadStruct Head;
	uint16_t wTemp;
	PayloadBuf->Pos = 0;
	if (ClientID)
	{
		if (!MQTT_AddUFT8String(PayloadBuf, ClientID))
		{
			DBG_ERROR("!");
			return 0;
		}
	}
	else
	{
		memset(PayloadBuf->Data, 0, 2);
		PayloadBuf->Pos = 2;
	}

	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_CONNECT;
	Head.Data = MsgHeadBuf;
	Head.DataLen = 10;
	MsgHeadBuf[7] = Flag;
	MsgHeadBuf[8] = (KeepTime >> 8);
	MsgHeadBuf[9] = KeepTime & 0x00ff;

	if (Flag & MQTT_CONNECT_FLAG_WILL)
	{
		if (!WillTopic || !WillMsgData || !WillMsgLen)
		{
			DBG_ERROR("no will topic or msg!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, WillTopic))
		{
			DBG_ERROR("!");
			return 0;
		}

		wTemp = htons(WillMsgLen);
		if (PayloadBuf->MaxLen >= (PayloadBuf->Pos + WillMsgLen + 2))
		{
			memcpy(PayloadBuf->Data + PayloadBuf->Pos, &wTemp, 2);
			memcpy(PayloadBuf->Data + PayloadBuf->Pos + 2, WillMsgData, WillMsgLen);
			PayloadBuf->Pos += WillMsgLen + 2;
		}
		else
		{
			return 0;
		}
	}

	if (Flag & MQTT_CONNECT_FLAG_USER)
	{
		if (!User)
		{
			DBG_ERROR("no user string!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, User))
		{
			DBG_ERROR("!");
			return 0;
		}
	}

	if (Flag & MQTT_CONNECT_FLAG_PASSWD)
	{
		if (!Passwd)
		{
			DBG_ERROR("no password string!");
			return 0;
		}

		if (!MQTT_AddUFT8String(PayloadBuf, Passwd))
		{
			DBG_ERROR("!");
			return 0;
		}
	}

	return MQTT_EncodeMsg(&Head, PayloadBuf->Data, PayloadBuf->Pos, TxBuf);
}

uint32_t MQTT_PublishMsg(Buffer_Struct *TxBuf, uint8_t Flag, uint16_t PackID, const int8_t *Topic,
		uint8_t *Payload, uint32_t PayloadLen)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_PUBLISH;
	Head.Flag = Flag;
	Head.DataLen = strlen(Topic) + 2;
	Head.String = (uint8_t *)Topic;
	Head.PackID = htons(PackID);
	return MQTT_EncodeMsg(&Head, Payload, PayloadLen, TxBuf);
}

uint32_t MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, uint8_t Cmd, uint16_t PackID)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = Cmd;
	Head.DataLen = 0;
	Head.Data = NULL;
	Head.PackID = htons(PackID);
	return MQTT_EncodeMsg(&Head, NULL, 0, TxBuf);
}

uint32_t MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum)
{
	MQTT_HeadStruct Head;
	uint32_t i;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_SUBSCRIBE;
	Head.DataLen = 0;
	Head.Data = NULL;
	Head.PackID = htons(PackID);
	PayloadBuf->Pos = 0;
	for(i = 0; i < TopicNum; i++)
	{
		if (!MQTT_AddUFT8String(PayloadBuf, Topic[i].Char))
		{
			DBG_ERROR("!");
			return 0;
		}
		if (PayloadBuf->Pos >= PayloadBuf->MaxLen)
		{
			return 0;
		}
		PayloadBuf->Data[PayloadBuf->Pos++] = Topic[i].Qos;
	}
	return MQTT_EncodeMsg(&Head, PayloadBuf->Data, PayloadBuf->Pos, TxBuf);
}

uint32_t MQTT_UnSubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum)
{
	MQTT_HeadStruct Head;
	uint32_t i;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = MQTT_CMD_UNSUBSCRIBE;
	Head.DataLen = 0;
	Head.Data = NULL;
	Head.PackID = htons(PackID);
	PayloadBuf->Pos = 0;
	for(i = 0; i < TopicNum; i++)
	{
		if (!MQTT_AddUFT8String(PayloadBuf, Topic[i].Char))
		{
			DBG_ERROR("!");
			return 0;
		}
		if (PayloadBuf->Pos >= PayloadBuf->MaxLen)
		{
			return 0;
		}
	}
	return MQTT_EncodeMsg(&Head, PayloadBuf->Data, PayloadBuf->Pos, TxBuf);
}


uint32_t MQTT_SingleMsg(Buffer_Struct *TxBuf, uint8_t Cmd)
{
	MQTT_HeadStruct Head;
	memset(&Head, 0, sizeof(Head));
	Head.Cmd = Cmd;
	return MQTT_EncodeMsg(&Head, NULL, 0, TxBuf);
}
