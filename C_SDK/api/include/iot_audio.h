#ifndef __IOT_AUDIO_H__
#define __IOT_AUDIO_H__

#include "iot_os.h"


/**
 * @defgroup iot_sdk_audio 音频接口
 * @{
 */
/**@example demo_audio/src/demo_audio.c
* audio接口示例
*/

/**打开语音
*@note  在通话开始时调用
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_open_tch(                                        
                        VOID
                );

/**关闭语音
*@note  通话结束时调用
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_close_tch(                                      
                        VOID
                 );

/**播放TONE音
*@param  toneType:      TONE音类型
*@param  duration:      播放时长
*@param  volume:        播放音量
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_tone(                                        
                        E_AMOPENAT_TONE_TYPE toneType,     
                        UINT16 duration,                   
                        E_AMOPENAT_SPEAKER_GAIN volume     
                 );

/**停止播放TONE音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_tone(                                        
                        VOID
                 );


/**播放DTMF音
*@param  dtmfType:      DTMF类型
*@param  duration:      播放时长
*@param  volume:        播放音量
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_dtmf(                                        
                        E_AMOPENAT_DTMF_TYPE dtmfType,     
                        UINT16 duration,                   
                        E_AMOPENAT_SPEAKER_GAIN volume     
                 );

/**停止播放DTMF音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_dtmf(                            
                        VOID
                 );

/**播放音频
*@param  playParam:     播放参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_music(T_AMOPENAT_PLAY_PARAM*  playParam);

/**停止音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_music(                                        
                        VOID
                  );

/**暂停音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_pause_music(                                     
                        VOID
                   );

/**恢复音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_resume_music(                                       
                        VOID
                    );

/**设置MP3播放音效
*@param  setEQ:        设置MP3音效
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_eq(                              
                        E_AMOPENAT_AUDIO_SET_EQ setEQ
                    );

/**开启MIC
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_open_mic(                                    
                        VOID
                );

/**关闭MIC
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_close_mic(                                         
                        VOID
                 );

/**设置MIC静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_mute_mic(                                          
                        VOID
                );

/**解除MIC静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_unmute_mic(                                  
                        VOID
                  );

/**设置MIC的增益
*@note  micGain值最大为20
*@param  micGain:       设置MIC的增益值
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_mic_gain(                              
                        UINT16 micGain                 
                    );

/**打开扬声器
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_open_speaker(                    
                        VOID
                    );

/**关闭扬声器
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_close_speaker(                       
                        VOID
                     );


/**设置扬声器静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_mute_speaker(                                     
                        VOID
                    );

/**解除扬声器静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_unmute_speaker(                                   
                        VOID
                      );

/**设置扬声器的增益
*@param     speakerGain:   设置扬声器增益值
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_speaker_gain(                                  
                        E_AMOPENAT_SPEAKER_GAIN speakerGain 
                        );

/**获取扬声器的增益值
*@return	E_AMOPENAT_SPEAKER_GAIN: 	 返回扬声器的增益值
**/
E_AMOPENAT_SPEAKER_GAIN iot_audio_get_speaker_gain(              
                        VOID
                                           );

/**设置音频通道
*@param     channel:    通道
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_channel(                                       
                        E_AMOPENAT_AUDIO_CHANNEL channel    
                   );

/**设置共用同一个MIC音频通道
*@param     channel_1:    通道1
*@param     channel_2:    通道2
**/
VOID iot_audio_set_channel_with_same_mic(                          
                    E_AMOPENAT_AUDIO_CHANNEL channel_1,    
                    E_AMOPENAT_AUDIO_CHANNEL channel_2      
               );

/**完善MIC音频通道设置
*@param     hfChanne:    手柄通道
*@param     erChanne:    耳机通道
*@param     ldChanne:    免提通道
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_hw_channel(
                      E_AMOPENAT_AUDIO_CHANNEL hfChanne,    
                      E_AMOPENAT_AUDIO_CHANNEL erChanne,    
                      E_AMOPENAT_AUDIO_CHANNEL ldChanne    
                     );

/**获取当前通道
*@return	E_AMOPENAT_AUDIO_CHANNEL: 	  返回通道值
**/
E_AMOPENAT_AUDIO_CHANNEL iot_audio_get_current_channel(            
                        VOID
                                               );

/**开始录音
*@note 录音暂时只支持amr格式
*@param     codec:                  录音格式
*@param     AUDIO_REC_CALLBACK:     获取录音数据回调
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_rec_start(
                    OPENAT_REC_MODE_T codec,
                            AUDIO_REC_CALLBACK recHanlder);

/**停止录音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_rec_stop(VOID);

/** @}*/


#endif
