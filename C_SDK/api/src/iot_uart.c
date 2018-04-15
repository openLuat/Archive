#include "iot_uart.h"

/**打开uart
*@param		port:		UART 编号
*@param		cfg:		配置信息
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_config(
                        E_AMOPENAT_UART_PORT port,          
                        T_AMOPENAT_UART_PARAM *cfg         
                   )
{
    return IVTBL(config_uart)(port, cfg);
}

/**关闭uart
*@param		port:		UART 编号
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_close(
                        E_AMOPENAT_UART_PORT port          
                   )
{
    return IVTBL(close_uart)(port);
}
 
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
                   )
{
    return IVTBL(read_uart)(port, buf, bufLen, timeoutMs);
}

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
                    )
{
    return IVTBL(write_uart)(port, buf, bufLen);
}

/**uart接收中断使能
*@param		port:		UART 编号
*@param		enable:		是否使能
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_enable_rx_int(
                        E_AMOPENAT_UART_PORT port,          
                        BOOL enable                       
                            )
{
    return IVTBL(uart_enable_rx_int)(port, enable);
} 

/**host uart初始化
*@param		hostCallback:		host uart回调函数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_host_init(PHOST_MESSAGE hostCallback)
{
    return IVTBL(host_init)(hostCallback);
}

/**host uart写数据
*@param		data:		写数据地址
*@param		len:		写数据长度
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_uart_host_send_data(uint8 *data, uint32 len)
{
    return IVTBL(host_send_data)(data, len);
} 