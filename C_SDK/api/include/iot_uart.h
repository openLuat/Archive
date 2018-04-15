#ifndef __IOT_UART_H__
#define __IOT_UART_H__

#include "iot_os.h"
/**
 * @ingroup iot_sdk_device 外设接口
 * @{
 */
/**
 * @defgroup iot_sdk_uart 串口接口
 * @{
 */
/**@example demo_uart/src/demo_uart.c
* uart接口示例
*/ 

/**打开uart
*@param		port:		UART 编号
*@param		cfg:		配置信息
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_config(
                        E_AMOPENAT_UART_PORT port,       
                        T_AMOPENAT_UART_PARAM *cfg         
                   );

/**关闭uart
*@param		port:		UART 编号
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_close(
                        E_AMOPENAT_UART_PORT port          
                   );


/**读uart
*@param		port:		UART 编号
*@param		buf:		存储数据地址
*@param		bufLen:		存储空间长度
*@param		timeoutMs:	读取超时 ms
*@return	UINT32:     实际读取长度
**/
UINT32 iot_uart_read(                                       
                        E_AMOPENAT_UART_PORT port,         
                        UINT8* buf,                       
                        UINT32 bufLen,                      
                        UINT32 timeoutMs                  
                   );

/**写uart
*@param		port:		UART 编号
*@param		buf:		写入数据地址
*@param		bufLen:		写入数据长度
*@return	UINT32:     实际读取长度
**/
UINT32 iot_uart_write(                                      
                        E_AMOPENAT_UART_PORT port,        
                        UINT8* buf,                         
                        UINT32 bufLen                       
                    );

/**uart接收中断使能
*@param		port:		UART 编号
*@param		enable:		是否使能
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_enable_rx_int(
                        E_AMOPENAT_UART_PORT port,       
                        BOOL enable                      
                            );


/**host uart初始化
*@param		hostCallback:		host uart回调函数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_host_init(PHOST_MESSAGE hostCallback);

/**host uart写数据
*@param		data:		写数据地址
*@param		len:		写数据长度
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_host_send_data(uint8 *data, uint32 len);

/** @}*/
/** @}*/
#endif

