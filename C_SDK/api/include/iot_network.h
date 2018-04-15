#ifndef __IOT_NETWORK_H__
#define __IOT_NETWORK_H__

#include "am_openat.h"

/**
 * @defgroup iot_sdk_network 网络接口
 * @{
 */
/**获取网络状态
*@param     status:   返回网络状态
*@return    TRUE:    成功
            FLASE:   失败            
**/                                
BOOL iot_network_get_status (
                            T_OPENAT_NETWORK_STATUS* status
                            );
/**设置网络状态回调函数
*@param     indCb:   回调函数
*@return    TRUE:    成功
            FLASE:   失败
**/                            
BOOL iot_network_set_cb    (
                            F_OPENAT_NETWORK_IND_CB indCb
                          );
/**建立网络连接，实际为pdp激活流程
*@param     connectParam:  网络连接参数，需要设置APN，username，passwrd信息
*@return    TRUE:    成功
            FLASE:   失败
@note      该函数为异步函数，返回后不代表网络连接就成功了，indCb会通知上层应用网络连接是否成功，连接成功后会进入OPENAT_NETWORK_LINKED状态
           创建socket连接之前必须要建立网络连接
           建立连接之前的状态需要为OPENAT_NETWORK_READY状态，否则会连接失败
**/                          
BOOL iot_network_connect     (
                            T_OPENAT_NETWORK_CONNECT* connectParam
                          );
/**断开网络连接，实际为pdp去激活
*@param     flymode:   暂时不支持，设置为FLASE

*@return    TRUE:    成功
            FLASE:   失败
@note      该函数为异步函数，返回后不代表网络连接立即就断开了，indCb会通知上层应用
           连接断开后网络状态会回到OPENAT_NETWORK_READY状态
           此前创建socket连接也会失效，需要close掉
**/                                        
BOOL iot_network_disconnect  (
                            BOOL flymode
                          );

/** @}*/

#endif

