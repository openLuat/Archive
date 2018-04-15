#include "iot_sys.h"


/**ota设置new app的文件，用来告知底层需要从文件读取升级新的程序
*@param		newAPPFile:		新程序文件 
*@return	TRUE: 成功   FALSE: 失败
**/
BOOL iot_ota_newapp(              
                    CONST char* newAPPFile
               )
{
    return IVTBL(flash_set_newapp)(newAPPFile);
}


/**将char类型转换为WCHAR，结果用来作为iot_fs_open_file等接口的文件名参数
*@param     dst:        转换输出结果
*@param     src:        等待转换的字符串
*@return    返回dst首地址
**/ 
WCHAR* iot_strtows(WCHAR* dst, const char* src)
{
   WCHAR* rlt = dst;
   while(*src)
   {
       *dst++ = *src++;
   }
   *dst = 0;
   
   return (rlt);
}


/**用来设置虚拟AT通道的回调函数
*@param		vatHandle:  虚拟AT主动上报或者AT命令结果返回的回调函数
*@return	TRUE: 成功   FALSE: 失败
**/
BOOL iot_vat_init(PAT_MESSAGE vatHandle)
{
	return IVTBL(init_at)(vatHandle);
}

/**用来发送AT命令
*@param		cmdStr:  AT命令字符串
*@return	cmdLen:  AT命令长度
*@note      注意，AT命令字符串cmdStr中需要包含\r\n或者\r结尾
**/
BOOL iot_vat_sendATcmd(UINT8* cmdStr, UINT16 cmdLen)
{
	return IVTBL(send_at_command)(cmdStr, cmdLen);
}

