#include "iot_touchscreen.h"


/**触摸屏初始化接口
*@param		:	触屏消息回调函数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/

BOOL iot_init_touchScreen(
				PTOUCHSCREEN_MESSAGE pTouchScreenMessage
				)
{
	return IVTBL(init_touchScreen)(pTouchScreenMessage);
}


/**触摸屏睡眠接口
*@param		:	是否要睡眠
*@return		: 	   
**/

VOID iot_touchScreen_sleep(
						BOOL sleep
						)
{
	
	return IVTBL(touchScreenSleep)(sleep);
}

