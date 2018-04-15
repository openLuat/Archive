#include "iot_keypad.h"



/**键盘初始化 
*@param		:键盘配置参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/	
BOOL iot_keypad_init(                         
                        T_AMOPENAT_KEYPAD_CONFIG *pConfig
                  )
{
   return IVTBL(init_keypad)(  pConfig );
}

