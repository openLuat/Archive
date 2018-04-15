#include "iot_lcd.h"


/**写入 lcd命令
*@param		:	命令
*@return	:
**/
VOID iot_lcd_write_cmd(                          
                        UINT8 cmd 
                   )
{
    return  IVTBL(send_color_lcd_command)(cmd);
}

/**lcd 写入lcd数据 
*@param	 	:	数据
*@return	:
**/
VOID iot_lcd_write_data(                               
                        UINT8 data                
                )
{
    return IVTBL(send_color_lcd_data)(data);
}

/**lcd初始化
*@param		:lcd初始化参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/	
BOOL iot_lcd_color_init(T_AMOPENAT_COLOR_LCD_PARAM *param )
{
    return IVTBL(init_color_lcd)(  param );
}

/**  刷新lcd
*@param		:彩屏初始化参数
*@param		:刷新的缓冲区
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
VOID iot_lcd_update_color_screen(
				T_AMOPENAT_LCD_RECT_T* rect,        
				UINT16 *pDisplayBuffer    )
{
    return IVTBL(update_color_lcd_screen)(                       
                            rect,      
                            pDisplayBuffer       
                                   );
}
/** 解码jpg格式图片
*@param		:文件路径包括文件名
*@param		:文件格式
*@return	:解码状态码
**/
INT32 iot_decode_jpeg(
                    CONST char * filename,
                    T_AMOPENAT_IMAGE_INFO *imageinfo
                    )
{
    return IVTBL(imgs_decode_jpeg)(filename,imageinfo);
}

/** 释放jpg格式解码数据
*@param		:缓存显示buffer
*@return	:释放状态码
**/
INT32 iot_free_jpeg_decodedata(
                    INT16* buffer
                    )
{
    return IVTBL(imgs_free_jpeg_decodedata)(buffer);
}
    

