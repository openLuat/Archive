#ifndef __IOT_ADC_H__
#define __IOT_ADC_H__

#include "iot_os.h"

/**
 * @ingroup iot_sdk_device 外设接口
 * @{
 */
/**
 * @defgroup iot_sdk_adc adc接口
 * @{
 */

/**ADC初始化 
*@param		chanle:		adc通道
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_adc_init(
                        E_AMOPENAT_ADC_CHANNEL chanle
                );


/**读取ADC数据
*@note ADC值，可以为空, 电压值，可以为空
*@param		chanle:		adc通道
*@param		adcValue:	ADC值，可以为空
*@param		voltage:	电压值，可以为空
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_adc_read(
                        E_AMOPENAT_ADC_CHANNEL chanle,    
                        UINT16* adcValue,                
                        UINT16* voltage                    
                );

/** @}*/
/** @}*/





#endif

