#ifndef __IOT_SYS_H__
#define __IOT_SYS_H__

#include "iot_os.h"


/**
 * @defgroup iot_sdk_sys 系统接口
 * @{
 */
/**@example demo_ota/src/demo_ota.c
* fs接口示例
*/ 

/**ota设置new app的文件，用来告知底层需要从文件读取升级新的程序
*@param		newAPPFile:		新程序文件 
*@return	TRUE: 成功   FALSE: 失败
**/
BOOL iot_ota_newapp(              
                    CONST char* newAPPFile
               );




/**@example demo_vat/src/demo_vat.c
* vat接口示例
*/ 
/**用来设置虚拟AT通道的回调函数
*@param		vatHandle:  虚拟AT主动上报或者AT命令结果返回的回调函数
*@return	TRUE: 成功   FALSE: 失败
**/
BOOL iot_vat_init(PAT_MESSAGE vatHandle);

/**用来发送AT命令
*@param		cmdStr:  AT命令字符串
*@param   	cmdLen:  AT命令长度
*@return	TRUE: 成功   FALSE: 失败
*@note      注意，AT命令字符串cmdStr中需要包含"\r\n"或者"\r"结尾
**/
BOOL iot_vat_sendATcmd(UINT8* cmdStr, UINT16 cmdLen);

/** @}*/






#endif

