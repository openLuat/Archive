/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_SLI3108.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          SLI3108.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_SLI3108_H__
#define __PLATFORM_SLI3108_H__

typedef enum SLI3108_STATUS_TAG
{
	SLI3108_STATUS_INVALID,
	SLI3108_STATUS_LOW,                          /*超出最低阀值，表示未佩戴*/
	SLI3108_STATUS_HIGH,                         /*超出最高阀值，表示已佩戴*/
}SLI3108_STATUS; 

void platform_SLI3108_init(void);
void platform_SLI3108_set_work_mode(BOOL factory_mode);
void platform_SLI3108_open(void);
void platform_SLI3108_close(void);

#endif //__PLATFORM_SLI3108_H__

