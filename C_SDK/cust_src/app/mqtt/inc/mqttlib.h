#ifndef __MQTT_H__
#define __MQTT_H__

#include "am_openat.h"
#include "iot_types.h"
#define MQTT_MSG_DUP 					(0x08)
#define MQTT_MSG_QOS_MASK				(0x06)
#define MQTT_MSG_QOS2 					(0x04)
#define MQTT_MSG_QOS1 					(0x02)
#define MQTT_MSG_RETAIN 				(0x01)
#define MQTT_CONNECT_FLAG_USER			(0x80)
#define MQTT_CONNECT_FLAG_PASSWD		(0x40)
#define MQTT_CONNECT_FLAG_WILLRETAIN	(0x20)
#define MQTT_CONNECT_FLAG_WILLQOS2		(0x10)
#define MQTT_CONNECT_FLAG_WILLQOS1		(0x08)
#define MQTT_CONNECT_FLAG_WILL			(0x04)
#define MQTT_CONNECT_FLAG_CLEAN			(0x02)
#define MQTT_SUBSCRIBE_QOS2				(0x02)
#define MQTT_SUBSCRIBE_QOS1				(0x01)

enum MQTTENUM
{
	MQTT_CMD_CONNECT = 1,
	MQTT_CMD_CONNACK,
	MQTT_CMD_PUBLISH,
	MQTT_CMD_PUBACK,
	MQTT_CMD_PUBREC,
	MQTT_CMD_PUBREL,
	MQTT_CMD_PUBCOMP,
	MQTT_CMD_SUBSCRIBE,
	MQTT_CMD_SUBACK,
	MQTT_CMD_UNSUBSCRIBE,
	MQTT_CMD_UNSUBACK,
	MQTT_CMD_PINGREQ,
	MQTT_CMD_PINGRESP,
	MQTT_CMD_DISCONNECT,
};

//带长度控制的缓冲结构
typedef struct
{
	uint8_t *Data;		//数据指针
	uint32_t Pos;		//包含的字节数
	uint32_t MaxLen;	//该缓冲区容量
}Buffer_Struct;

//带长度控制的主题结构，用于订阅/取消订阅
typedef struct
{
	uint8_t *Char;		//主题字符串
	uint8_t Qos;		//主题的QOS，只能是0，MQTT_SUBSCRIBE_QOS2，MQTT_SUBSCRIBE_QOS1
}MQTT_SubscribeStruct;

//MQTT的头部数据
typedef struct
{
	uint8_t *Data;		//附加数据指针，发送时只用于CONNECT报文的可变报头，接收时，存放CONNECT和PUBLISH报文的可变报头
	uint8_t *String;	//附加主题类数据指针，发送时只用于PUBLISH报文的可变报头，不能用于接收
	uint32_t DataLen;	//附加数据长度，CONNECT报文10，PUBLISH报文，主题长度+2
	uint16_t PackID;	//报文标识符，小端格式
	uint8_t Cmd;		//MQTT控制报文的类型
	uint8_t Flag;		//用于指定控制报文类型的标志位
}MQTT_HeadStruct;

/**
 * @defgroup iot_mqtt_api MQTT协议API
 * @{
 */
/**在编码MQTT报文的过程中，将字符串以MQTT要求添加进报文，不支持带'\0'的字符串，比如unicode-16的英文
*@param     Buf:   		存放添加字符串报文的缓存
*@param     String:		字符串
*@return    >0:    		成功，返回编码后的报文长度
            <=0:   		失败
**/
uint32_t MQTT_AddUFT8String(Buffer_Struct *Buf, const int8_t *String);

/**编码MQTT报文
*@param     Head:   	需要编码的MQTT报头，包括了可变部分和固定部分
*@param     Payload:	有效载荷数据指针
*@param     PayloadLen:	有效载荷长度
*@param     TxBuf:		存放编码后报文的缓存
*@return    >0:    		成功，返回编码后的报文长度
            <=0:   		失败
**/
uint32_t MQTT_EncodeMsg(MQTT_HeadStruct *Head, uint8_t *Payload, uint32_t PayloadLen, Buffer_Struct *TxBuf);

/**解码MQTT报文
*@param     Head:   		存放解码后的MQTT报头，包括了可变部分和固定部分
*@param     HeadDataLenMax:	需要解码报文的报头最大长度，根据实际情况，用户需要根据订阅的主题长度作出调整，
*@param     PayloadLen:		解码后有效载荷长度
*@param     RxBuf:			需要解码的报文指针
*@param     RxLen:			需要解码的报文长度
*@param     DealLen:		本次解码的实际长度，注意如果有粘包，则会小于接收的长度
*@return    >=0:    		成功，返回有效载荷数据指针，如果不存在有效载荷，则返回0
            <0:   			失败
**/
uint8_t* MQTT_DecodeMsg(MQTT_HeadStruct *Head, uint32_t HeadDataLenMax, uint32_t *PayloadLen, uint8_t *RxBuf, uint32_t RxLen, uint32_t *DealLen);

/**编码CONNECT报文
*@param     TxBuf:   		存放编码后报文的缓存
*@param     PayloadBuf:		临时存放有效载荷数据的缓存，外部提供，函数内部使用
*@param     Flag:			连接标识，0或者以下宏定义的组合
							MQTT_CONNECT_FLAG_USER
							MQTT_CONNECT_FLAG_PASSWD
							MQTT_CONNECT_FLAG_WILLRETAIN
							MQTT_CONNECT_FLAG_WILLQOS2
							MQTT_CONNECT_FLAG_WILLQOS1
							MQTT_CONNECT_FLAG_WILL
							MQTT_CONNECT_FLAG_CLEAN
*@param     KeepTime:		保持连接时间
*@param     ClientID:		客户端标识符字符串，可以为空
*@param     WillTopic:		遗嘱主题字符串
*@param     User:			用户名字符串
*@param     Passwd:			密码字符串
*@param     WillMsgData:	遗嘱消息数据指针
*@param     WillMsgLen:		遗嘱消息长度
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint8_t Flag, uint16_t KeepTime,
		const int8_t *ClientID,
		const int8_t *WillTopic,
		const int8_t *User,
		const int8_t *Passwd,
		uint8_t *WillMsgData, uint16_t WillMsgLen);

/**编码PUBLISH报文
*@param     TxBuf:   		存放编码后报文的缓存
*@param     Flag:			报文标识位，0或者以下宏定义的组合
							MQTT_MSG_DUP
							MQTT_MSG_QOS_MASK
							MQTT_MSG_QOS2
							MQTT_MSG_QOS1
							MQTT_MSG_RETAIN
*@param     PackID:			报文标识符，小端格式
*@param     Topic:			主题字符串
*@param     Payload:	 	有效载荷
*@param     PayloadLen:		有效载荷长度
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_PublishMsg(Buffer_Struct *TxBuf, uint8_t Flag, uint16_t PackID, const int8_t *Topic,
		uint8_t *Payload, uint32_t PayloadLen);

/**编码PUBLISH过程中QOS控制类型报文，包括PUBACK，PUBREC，PUBREL，PUBCOMP
*@param     TxBuf:   		存放编码后报文的缓存
*@param     Cmd:			报文类型
*@param     PackID:			报文标识符，小端格式
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, uint8_t Cmd, uint16_t PackID);

/**编码SUBSCRIBE报文
*@param     TxBuf:   		存放编码后报文的缓存
*@param     PayloadBuf:		临时存放有效载荷数据的缓存，外部提供，函数内部使用
*@param     PackID:			报文标识符，小端格式
*@param     Topic:			需要订阅的主题缓存指针，允许有多个主题
*@param     TopicNum:		主题数量
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);

/**编码UNSUBSCRIBE报文
*@param     TxBuf:   		存放编码后报文的缓存
*@param     PayloadBuf:		临时存放有效载荷数据的缓存，外部提供，函数内部使用
*@param     PackID:			报文标识符，小端格式
*@param     Topic:			需要取消订阅的主题缓存指针，允许有多个主题
*@param     TopicNum:		主题数量
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_UnSubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);

/**编码其他简单报文，只用于PINGREQ和DISCONNECT
*@param     TxBuf:   		存放编码后报文的缓存
*@param     PayloadBuf:		临时存放有效载荷数据的缓存，外部提供，函数内部使用
*@param     Cmd:			报文类型
*@return    >0:    			成功，返回编码后的报文长度
            <=0:   			失败
**/
uint32_t MQTT_SingleMsg(Buffer_Struct *TxBuf, uint8_t Cmd);

/** @}*/

#endif
