#ifndef __IOT_KEYPAD_H__
#define __IOT_KEYPAD_H__

#include "iot_os.h"

/**
 * @ingroup iot_sdk_device 外设接口
 * @{
 */
/**
 * @defgroup iot_sdk_keypad 按键接口
 * @{
 */

/**@example demo_lcd/src/demo_lcd.c
* LCD&键盘接口示例
*/ 

/**键盘初始化 
*@param		pConfig: 键盘配置参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_keypad_init(                         
                        T_AMOPENAT_KEYPAD_CONFIG *pConfig
                  );

/** @}*/
/** @}*/

#endif

