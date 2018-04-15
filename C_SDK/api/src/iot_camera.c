#include "iot_camera.h"

/**摄像头初始化
*@param		cameraParam:		初始化参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_camera_init(T_AMOPENAT_CAMERA_PARAM *cameraParam)
{
  return IVTBL(camera_init)(cameraParam);
}

/**打开摄像头
*@param		videoMode:		是否视频模式
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_camera_poweron(BOOL videoMode)
{
  return IVTBL(camera_poweron)(videoMode);
}
/**关闭摄像头
*@return  TRUE:       成功
*           FALSE:      失败
**/
BOOL iot_camera_poweroff(void)
{
  return IVTBL(camera_poweroff)();
}
/**开始预览
*@param  previewParam:       预览参数
*@return	TRUE: 	    成功
*           FALSE:      失败

**/
BOOL iot_camera_preview_open(T_AMOPENAT_CAM_PREVIEW_PARAM *previewParam)
{
  return IVTBL(camera_preview_open)(previewParam);
}
/**退出预览
*@return	TRUE: 	    成功
*           FALSE:      失败

**/
BOOL iot_camera_preview_close(void)
{
  return IVTBL(camera_preview_close)();
}
/**拍照
*@param  captureParam:       预览参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_camera_capture(T_AMOPENAT_CAM_CAPTURE_PARAM *captureParam)
{ 
  return IVTBL(camera_capture)(captureParam);
}
/**保存照片
*@param  iFd:       待保存的照片文件句柄
*@return  TRUE:       成功
*           FALSE:      失败
**/
BOOL iot_camera_save_photo( INT32 iFd)
{
  return IVTBL(camera_save_photo)(iFd);
}