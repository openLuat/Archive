/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_atc.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Implement 'atc' class.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_atc.h"


void rmmi_write_to_uart(CHAR *buffer, UINT16 length)
{
	UINT8 data[1024] = {0};

	strcat((CHAR*)data,buffer);
	platform_uart_send_buff(PLATFORM_UART_ID_ATC, data, strlen((CHAR*)data));
	send_uart_rec_message(PLATFORM_UART_ID_ATC);
}

void platform_sim_insert(void)
{

	//TO DO ...
}

void platform_sim_remove(void)
{

}