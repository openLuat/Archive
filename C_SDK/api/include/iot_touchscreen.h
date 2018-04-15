#ifndef __IOT_TOUCHSCREEN_H__
#define __IOT_TOUCHSCREEN_H__

#include "iot_os.h"

/**
 * @ingroup iot_sdk_device 外设接口
 * @{
 */
/**
 * @defgroup iot_sdk_lcd lcd接口
 * @{
 */
 
/**@example demo_ui/src/demo_ui.c
* LCD&触摸屏接口示例
*/ 

/**初始化触摸屏
*@param		pTouchScreenMessage:	触摸屏回调函数
*@return	TRUE: 	    成功
*             FALSE:      失败
**/

BOOL iot_init_touchScreen(PTOUCHSCREEN_MESSAGE pTouchScreenMessage);


/**触摸屏睡眠接口
*@param		sleep:	是否要睡眠
*@return	: 	   
**/

VOID iot_touchScreen_sleep(BOOL sleep);

/** @}*/
/** @}*/
#endif

