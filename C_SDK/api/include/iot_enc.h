#ifndef __IOT_ENC_H__
#define __IOT_ENC_H__

#include "iot_os.h"


/**
 * @defgroup iot_sdk_enc 加密卡接口
 * @{
 */

/**设置密钥信息
*@param		encInfo:	设置信息的数据
*@param		len:	    设置信息的数据
*@return	TURE: 	    成功
*           FALSE:      失败
**/
BOOL iot_enc_set_info(                        
                    UINT8 *encInfo,
                    UINT32 len
          );

/**读取密钥信息
*@param		encInfo:	保存信息的空间
*@param		len:	    保存信息的空间长度
*@return	TURE: 	    成功
*           FALSE:      失败
**/
BOOL iot_enc_get_info(                   
                    UINT8 *encInfo,
                    UINT32 len
          );

/**读取加密校验结果
*@return	返回校验结果
**/
UINT8 iot_enc_get_result(                  
                    void
          );

/**读取卡类型
*@return 返回校验卡类型 
         0: 未知
         1: 加密卡
         2: 普通卡
**/
UINT8 iot_enc_get_cardtype(                   
                    void
          );
          

/**密钥通知接口
*@note iot_enc_set_info设置好密钥信息后, 要调用这个发出通知
*@return	TURE: 	    成功
*           FALSE:      失败
**/
BOOL iot_enc_set_data_ok(                    
                    void
          );

/** @}*/


#endif