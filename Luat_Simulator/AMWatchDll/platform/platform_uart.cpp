/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_uart.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          UART.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_uart.h"
#include "time.h"

static BOOL ceng_enbled = FALSE;
static UINT8 ceng_seq = 0;

static SimCom* _find_com_dev(int id);

custom_cmd_mode_enum custom_find_cmd_mode(custom_cmdLine *cmd_line)
{
	custom_cmd_mode_enum result;

	if (cmd_line->position < cmd_line->length - 1)
	{
		switch (cmd_line->character[cmd_line->position])
		{
		case '?':  /* AT+...? */
			{
				cmd_line->position++;
				result = CUSTOM_READ_MODE;
				break;
			}
		case '=':  /* AT+...= */
			{
				cmd_line->position++;
				if ((cmd_line->position < cmd_line->length - 1 ) &&
					(cmd_line->character[cmd_line->position] == '?'))
				{
					cmd_line->position++;
					result = CUSTOM_TEST_MODE;
				}
				else
				{
					result = CUSTOM_SET_OR_EXECUTE_MODE;
				}
				break;
			}
		default:  /* AT+... */
			{
				result = CUSTOM_ACTIVE_MODE;
				break;
			}
		}
	}
	else
	{
		result = CUSTOM_ACTIVE_MODE;
	}
	return (result);
}

custom_rsp_type_enum custom_atc_ate0_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cmee_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_ver_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+VER: SW_MT6261_V9188_AirM2M");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_blver_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+BLVER: 9188");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_atwmft_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_wisn_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+WISN: 0000000000");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cgsn_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "865616029998256");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_aud_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+AUD=0");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_creg_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
		break;
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+CREG: 1, \"0C75\", \"00009F12\", 0");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_atx4_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_clip_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_csq_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+CSQ: 18,99");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cpin_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+CPIN: READY");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cgatt_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
		break;
	case CUSTOM_READ_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+CGATT: 1");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cimi_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "460011880313619");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_iccid_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+ICCID: 89860115831018724972");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cclk_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			struct tm *local; 
			time_t t; 
			t=time(NULL); 
			local=localtime(&t);
			sprintf_s(resp, MAX_UART_LEN, "%d/%d/%d,%d:%d:%d\r\n",\
				local->tm_year-100,local->tm_mon+1,local->tm_mday, \
				local->tm_hour,local->tm_min,local->tm_sec);
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cnmi_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cpms_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+CPMS: 15,15,15,15,15,15");
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cscs_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_csmp_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_atc_cmgf_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

void srv_on_nbr_info(void)
{
	char cell_info_buf[MAX_UART_LEN] = {0};
	unsigned len = 0;
	SimCom* dev = NULL;

	if(ceng_enbled == FALSE) return;

	if(ceng_seq == 0)
	{
		sprintf(&cell_info_buf[len],"\r\n+CENG:0,1cc,1,3189,40722,48,26,255\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:1,1cc,1,3189,38513,4,23\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:2,1cc,1,3189,20522,1,19\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:3,1cc,1,3189,34502,13,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:4,1cc,1,3189,40723,48,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:5,1cc,1,3189,38511,53,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:6,1cc,1,3189,40733,34,13\r\n");
		ceng_seq = 1;
	}
	else if(ceng_seq == 1)
	{
		sprintf(&cell_info_buf[len],"\r\n+CENG:0,1cc,1,3189,40722,48,26,255\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:1,1cc,1,3189,38513,4,23\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:2,1cc,1,3189,20522,1,19\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:3,1cc,1,3189,40723,48,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:4,1cc,1,3189,38511,53,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:5,1cc,1,3189,34502,13,18\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:6,1cc,1,3189,40733,34,12\r\n");
		ceng_seq = 2;
	}
	else if(ceng_seq == 2)
	{
		sprintf(&cell_info_buf[len],"\r\n+CENG:0,1cc,1,3189,40722,48,24,255\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:1,1cc,1,3189,38513,4,22\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:2,1cc,1,3189,20522,1,19\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:3,1cc,1,3189,40723,48,19\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:4,1cc,1,3189,38511,53,17\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:5,1cc,1,3189,34502,13,16\r\n");
		len = strlen(cell_info_buf);
		sprintf(&cell_info_buf[len],"+CENG:6,1cc,1,3189,38512,22,12\r\n");
		ceng_seq = 0;
	}

	len = strlen(cell_info_buf);

	sprintf(&cell_info_buf[len], "OK\r\n");
	len = strlen(cell_info_buf);
	dev = _find_com_dev(PLATFORM_UART_ID_ATC);
	if(dev != NULL)
		dev->platform_s_uart_send_buff(PLATFORM_UART_ID_ATC, (UINT8*)cell_info_buf, len);
}

custom_rsp_type_enum custom_atc_ceng_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			ceng_enbled = ceng_enbled == TRUE ? FALSE : TRUE;
			ceng_seq = 0;
			if(ceng_enbled == TRUE)
				WinTimer::GetWinTimer()->start_nbr_timer(NBR_TIMER_TIMEOUT);
			else if(ceng_enbled == FALSE)
				WinTimer::GetWinTimer()->stop_nbr_timer();
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

const custom_atcmd atc_cmd_table[ ] =
{    
	{"ATE0",custom_atc_ate0_func},
	{"AT+CMEE",custom_atc_cmee_func},
	{"AT+VER",custom_atc_ver_func},
	{"AT+BLVER",custom_atc_blver_func},
	{"AT+ATWMFT",custom_atc_atwmft_func},
	{"AT+WISN",custom_atc_wisn_func},
	{"AT+CGSN",custom_atc_cgsn_func},
	{"AT+AUD",custom_atc_aud_func},
	{"AT+CREG",custom_atc_creg_func},
	{"ATX4",custom_atc_atx4_func},
	{"AT+CLIP",custom_atc_clip_func},
	{"AT+CSQ",custom_atc_csq_func},
	{"AT+CPIN",custom_atc_cpin_func},
	{"AT+CGATT",custom_atc_cgatt_func},
	{"AT+CIMI",custom_atc_cimi_func},
	{"AT+ICCID",custom_atc_iccid_func},
	{"AT+CCLK",custom_atc_cclk_func},
	{"AT+CNMI",custom_atc_cnmi_func},
	{"AT+CPMS",custom_atc_cpms_func},
	{"AT+CSCS",custom_atc_cscs_func},
	{"AT+CSMP",custom_atc_csmp_func},
	{"AT+CMGF",custom_atc_cmgf_func},
	{"AT+CENG",custom_atc_ceng_func},
	{NULL,NULL},
};

/******************************************************************************/
const custom_atcmd bt_cmd_table[ ] =
{    
	{NULL,NULL},
};

/******************************************************************************/

custom_rsp_type_enum custom_wifi_ate1_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_wifi_cwmode_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		break;

	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_wifi_cwlap_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_wifi_amsleep_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		{
			break;
		}

	case CUSTOM_READ_MODE:
		{
			sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "AT+AMSLEEP=2,240");
			break;
		}

	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

custom_rsp_type_enum custom_wifi_ver_func(custom_cmdLine *commandBuffer_p, char* resp)
{
	custom_cmd_mode_enum result;
	custom_rsp_type_enum ret_value  = CUSTOM_RSP_OK;

	result = custom_find_cmd_mode(commandBuffer_p);		
	switch (result)
	{
	case CUSTOM_SET_OR_EXECUTE_MODE:
	case CUSTOM_ACTIVE_MODE:
		sprintf_s(resp, MAX_UART_LEN, "%s\r\n", "+VER: AirM2M_ESP8266_V0020_A6501");
		break;

	default:
		ret_value  = CUSTOM_RSP_ERROR;
		break;
	}

	return ret_value;
}

const custom_atcmd wifi_cmd_table[ ] =
{    
	{"ATE1",custom_wifi_ate1_func},
	{"AT+CWMODE",custom_wifi_cwmode_func},
	{"AT+CWLAP",custom_wifi_cwlap_func},
	{"AT+AMSLEEP",custom_wifi_amsleep_func},
	{"AT+VER",custom_wifi_ver_func},
	{NULL,NULL},
};

/******************************************************************************/

const custom_atcmd uart2_cmd_table[ ] =
{    
	{NULL,NULL},
};

/******************************************************************************/

const custom_atcmd uart3_cmd_table[ ] =
{    
	{NULL,NULL},
};

/******************************************************************************/

const custom_atcmd default_cmd_table[ ] =
{    
	{NULL,NULL},
};

/******************************************************************************/

static ComDev comDev[] = 
{
	{PLATFORM_UART_ID_ATC, "COM127", NULL},
	{					0, "COM1", NULL},
	{					1, "COM2", NULL},  
	{					2, "COM3", NULL},
	{					3, "COM4", NULL},
};

static SimCom* _find_com_dev(int id)
{
	SimCom* dev;
    INT i;

    for(i = 0; i < sizeof(comDev)/sizeof(comDev[0]); i++)
    {
		if(comDev[i].COM != NULL)
			dev = comDev[i].COM;
		else
			continue;
        if(id == comDev[i].id && dev->GetSimComId())
        {
            return dev;
        }
    }

    return NULL;
}

static void _destroy_com_dev(int id)
{
	int i;

	for(i = 0; i < sizeof(comDev)/sizeof(comDev[0]); i++)
	{
		if(id == comDev[i].id)
		{
			comDev[i].COM = NULL;
			comDev[i].dwReadThread = INVALID_HANDLE_VALUE;
			comDev[i].dwReadThreadId = NULL;
			comDev[i].dwWriteThread = INVALID_HANDLE_VALUE;
			comDev[i].dwWriteThreadId = NULL;
		}
	}
}

static SimCom* _new_com_dev(int id)
{
	INT i;
	SimCom* dev = NULL;

	for(i = 0; i < sizeof(comDev)/sizeof(comDev[0]); i++)
	{
		if(id == comDev[i].id)
		{
			dev = comDev[i].COM = new SimCom(id, comDev[i].name);
		}
	}

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] _new_com_dev dev[%d] is invalid.",id);
	}

	return dev;
}

SimCom::SimCom(unsigned sId, CHAR* sName)
{
	id = sId;

	strcpy_s(name, COM_NAME_SIZE, sName);

	platform_uart_setup(sId,115200,8,0,1,0);

	semWrite = CreateSemaphore(NULL, 0, 1, NULL); //NULL, 1, 10, NULL
}

unsigned SimCom::GetSimComId()
{
	return id;
}

CHAR* SimCom::GetSimComName()
{
	return name;
}

void SimCom::SetReadThreadId(HANDLE thread, DWORD threadId)
{
	dwReadThread = thread;
	dwReadThreadId = threadId;
}

void SimCom::SetWriteThreadId(HANDLE thread, DWORD threadId)
{
	dwWriteThread = thread;
	dwWriteThreadId = threadId;
}

void send_uart_rec_message(int uart_id)
{
	MSG msg;

	PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));

	pMsg->id = MSG_ID_RTOS_UART_RX_DATA;
	pMsg->data.uart_id = uart_id;

	msg.message = SIMU_UART_ATC_RX_DATA;
	msg.wParam = (WPARAM)pMsg;
	SendToLoadLuaMessage(&msg);
}

void send_uart_message(int uart_id)
{
    MSG msg;
    
	PlatformMessage *pMsg = (PlatformMessage*)WinUtil::L_MALLOC(sizeof(PlatformMessage));
    
    pMsg->id = RTOS_MSG_UART_RX_DATA;
    pMsg->data.uart_id = uart_id;
    
    msg.message = SIMU_RTOS_MSG_ID;
    msg.wParam = (WPARAM)pMsg;
    SendToLoadLuaMessage(&msg);
}

void SimCom::simulate_uart_rx_thread(void)
{
}

void simulate_uart_rx_thread(LPVOID lparam)
{
	SimCom* dev = (SimCom*)lparam;
	unsigned result;
	MSG msg;

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] simulate_uart_rx_thread(READ) dev=NULL.");
		return;
	}
    
	while(1)
	{
		result = GetMessage(&msg, (HWND)-1, 0, 0);
		if(result == -1)
		{
			//TO DO ...
			break;
		}
		if(msg.message == WM_QUIT)
		{
			break;
		}
		dev->simulate_uart_rx_thread();
	}

}

void SimCom::simulate_uart_tx_thread(void)
{
	UINT8 txbuf[1024];
	unsigned len = 0,result = -1, sId = -1;
	char buffer[MAX_UART_LEN+1];
	char resp[MAX_UART_LEN+1];
	char *cmd_name, *cmdString;
	UINT8 index = 0; 
	UINT16 length;
	custom_cmdLine command_line;
	custom_atcmd *atcmd = NULL;

	memset(txbuf, 0, sizeof(txbuf));
	len = QueueDelete(&txq, txbuf, sizeof(txbuf));
	cmd_name = buffer;

	length = strlen((CHAR*)txbuf);
	length = length > MAX_UART_LEN ? MAX_UART_LEN : length;    
	while ((txbuf[index] != '=' ) &&  //might be TEST command or EXE command
		(txbuf[index] != '?' ) && // might be READ command
		(txbuf[index] != 13 ) && //carriage return
		index < length)  
	{
		cmd_name[index] = CUSTOM_TO_UPPER(txbuf[index]) ;
		index ++;
	}
	cmd_name[index] = '\0';

	switch(id)
	{
	case PLATFORM_UART_ID_ATC:
		atcmd = (custom_atcmd*)atc_cmd_table;
		break;

	/* BT */
	case SUART0:
		atcmd = (custom_atcmd*)bt_cmd_table;
		break;

	/* WIFI */
	case SUART1:
		atcmd = (custom_atcmd*)wifi_cmd_table;
		break;

	case SUART2:
		atcmd = (custom_atcmd*)uart2_cmd_table;
		break;

	case SUART3:
		atcmd = (custom_atcmd*)uart3_cmd_table;
		break;

	default:
		atcmd = (custom_atcmd*)default_cmd_table;
		break;
	}

	while(atcmd->commandString != NULL)
	{
		cmdString = atcmd->commandString;
		if (strcmp(cmd_name, cmdString) == 0 )
		{
			strncpy(command_line.character, (CHAR*)txbuf, COMMAND_LINE_SIZE + NULL_TERMINATOR_LENGTH);
			command_line.character[COMMAND_LINE_SIZE] = '\0';
			command_line.length = strlen(command_line.character);
			command_line.position = index;
			memset(resp, 0, sizeof(resp));
			if (atcmd->commandFunc(&command_line, resp) == CUSTOM_RSP_OK) 
			{
				if(strlen(resp) != 0)
				{
					len = strlen(resp);
					QueueInsert(&rxq, (UINT8*)resp, len);
					send_uart_rec_message(id);
				}
				sprintf(buffer, "OK");
				len = strlen(buffer);
				QueueInsert(&rxq, (UINT8*)buffer, len);
				send_uart_rec_message(id);
			}
			else if(atcmd->commandFunc(&command_line, resp) == CUSTOM_RSP_LATER)
			{
			}
			else
			{
				sprintf(buffer, "ERROR");
				len = strlen(buffer);
				QueueInsert(&rxq, (UINT8*)buffer, len);
				send_uart_rec_message(id);
			}
			return;
		}

		atcmd ++;
	}

	LogWriter::LOGX("[platform_uart[SimCom]] simulate_uart_tx_thread(WRITE) Unknown ATC.");
}

void simulate_uart_tx_thread(LPVOID lparam)
{
	SimCom* dev = (SimCom*)lparam;
	unsigned result = -1, sId = -1, message;
	MSG msg;

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] simulate_uart_tx_thread(WRITE) dev=NULL.");
		return;
	}

	while(1)
	{
		result = GetMessage(&msg, (HWND)-1, 0, 0);
		if(result == -1)
		{
			//TO DO ...
			break;
		}
		if(msg.message == WM_QUIT)
		{
			//TO DO ...
			break;
		}
		message = msg.message;
		sId = msg.wParam;

		dev = _find_com_dev(sId);

		dev->simulate_uart_tx_thread();
	}
	

}

UINT32 SimCom::platform_uart_setup( unsigned id, UINT32 baud, int databits, int parity, int stopbits, UINT32 mode )
{      
	LogWriter::LOGX("[platform_uart[SimCom]] platform_uart_setup id=%d,baud=%d,databits=%d,parity=%d,stopbits=%d,mode=%d.", \
		id,baud,databits,parity,stopbits,mode);

    memset(&rxq, 0, sizeof(rxq));
	rxq.buf = (UINT8*)WinUtil::L_MALLOC(COM_RX_BUF_SIZE);
    rxq.size = COM_RX_BUF_SIZE;
    QueueClean(&rxq);

    memset(&txq, 0, sizeof(txq));
	txq.buf = (UINT8*)WinUtil::L_MALLOC(COM_TX_BUF_SIZE);
    txq.size = COM_TX_BUF_SIZE;
    QueueClean(&txq);

	return baud;
}

UINT32 platform_uart_setup(unsigned id, UINT32 baud, int databits, int parity, int stopbits, UINT32 mode)
{
	SimCom* dev = _find_com_dev(id);

	if(dev == NULL)
	{
		dev = _new_com_dev(id);
	}
	else
	{
		LogWriter::LOGX("[platform_uart] platform_uart_setup dev[%d] has been set.",id);
		return baud;
	}

	return dev->platform_uart_setup(id, baud, databits, parity, stopbits, mode);
}

UINT32 SimCom::platform_uart_close(unsigned id)
{
	LogWriter::LOGX("[platform_uart[SimCom]] platform_uart_close id=%d.",id);

    
	if(txq.buf != NULL)WinUtil::L_FREE(txq.buf);
    QueueClean(&txq);
	if(rxq.buf != NULL)WinUtil::L_FREE(rxq.buf);
	QueueClean(&rxq);

    return PLATFORM_OK;
}

UINT32 platform_uart_close(unsigned id)
{
	SimCom *dev = _find_com_dev(id);

	if(dev == NULL) {
		LogWriter::LOGX("[platform_uart] platform_uart_close dev[%d]=NULL.",id);

		return PLATFORM_ERR;
	}

	dev->platform_uart_close(id);
	_destroy_com_dev(id);

	return PLATFORM_OK;
}

UINT32 SimCom::platform_s_uart_send( unsigned id, UINT8 data )
{
    DWORD dwWritten = 1;

    if(id == platform_get_console_port())
    {
		//TO DO ...
    }
	else
	{
		QueueInsert(&txq,&data,1);
	}

    return dwWritten;
}

UINT32 platform_s_uart_send(unsigned id, UINT8 data)
{
	SimCom *dev = _find_com_dev(id);

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] platform_s_uart_send dev[%d]=NULL.",id);

		return 0;
	}

	return dev->platform_s_uart_send(id, data);
}

UINT32 SimCom::platform_s_uart_send_buff(unsigned sId, UINT8* buff, UINT16 len)
{
	UINT8 str[MAX_UART_LEN] = {0};

    if(id == platform_get_console_port() || id == SUART0)
    {
		LogWriter::LOGX("[platform_uart] platform_s_uart_send_buff[ATC] sId=%d,buff=%s,len=%d.", \
			sId,buff,len);

		strcpy_s((CHAR*)str,MAX_UART_LEN,(CHAR*)buff);
		WinUtil::trim((CHAR*)str);
		QueueInsert(&txq, (UINT8*)str, len);

		PostThreadMessage(dwWriteThreadId,WM_UART_WRITE_THREAD,id,NULL);

        return len;
    }
	else if(id == SUART1) //WIFI
	{
		strcpy_s((CHAR*)str,MAX_UART_LEN,(CHAR*)buff);
		WinUtil::trim((CHAR*)str);

		QueueInsert(&txq, (UINT8*)str, len);

		PostThreadMessage(dwWriteThreadId,WM_UART_WRITE_THREAD,id,NULL);
		return len;
	}
	else if(id == SUART2)
	{
		//TO DO ...
	}
	else if(id == SUART3)
	{
		//TO DO ...
	}
	else
	{
		//TO DO ...
	}

    return len;
}

UINT32 platform_s_uart_send_buff(unsigned id, UINT8* buff, UINT16 len)
{
    SimCom* dev = _find_com_dev(id);

	if(dev == NULL)
    {
		LogWriter::LOGX("[platform_uart] platform_s_uart_send_buff dev[%d]=NULL",id);
        return 0;
    }

	return dev->platform_s_uart_send_buff(id, buff, len);
}

INT SimCom::platform_s_uart_recv(unsigned sId, INT32 sTimeout)
{
    UINT8 chRead;

	if(1 == QueueDelete(&rxq, &chRead, 1))
	{
		return chRead;
	}
	else
	{
		return -1;
	}
}

INT platform_s_uart_recv(unsigned id, INT32 timeout)
{
	SimCom* dev = _find_com_dev(id);

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] platform_s_uart_recv dev[%d]=NULL",id);
		return PLATFORM_ERR;
	}

	return dev->platform_s_uart_recv(id, timeout);
}

INT SimCom::platform_s_uart_set_flow_control(unsigned id, INT type)
{
	LogWriter::LOGX("[platform_uart[SimCom]] platform_s_uart_set_flow_control id=%d,type=%d.",id,type);

    return PLATFORM_OK;
}

INT platform_s_uart_set_flow_control(unsigned id, INT type)
{
	SimCom* dev = _find_com_dev(id);

	if(dev == NULL)
	{
		LogWriter::LOGX("[platform_uart] platform_s_uart_set_flow_control dev[%d]=NULL",id);
		return PLATFORM_ERR;
	}

	return dev->platform_s_uart_set_flow_control(id, type);
}

int platform_uart_init(void)
{
	INT i;

	LogWriter::LOGX("[platform_uart] platform_uart_init.");

	for(i = 0; i < sizeof(comDev)/sizeof(comDev[0]); i++)
	{
		if(strcmp(comDev[i].name, "NONE") != 0)
		{
			SimCom *dev = NULL;
			dev = comDev[i].COM = _new_com_dev(comDev[i].id);

			comDev[i].dwReadThread = CreateThread(NULL, 
				SUART_THREAD_STACK_SIZE,
				(LPTHREAD_START_ROUTINE)simulate_uart_rx_thread,
				(LPVOID)dev,
				0,
				&comDev[i].dwReadThreadId);
			dev->SetReadThreadId(comDev[i].dwReadThread, comDev[i].dwReadThreadId);

			comDev[i].dwWriteThread = CreateThread(NULL, 
				SUART_THREAD_STACK_SIZE, 
				(LPTHREAD_START_ROUTINE)simulate_uart_tx_thread,
				(LPVOID)dev,
				0,
				&comDev[i].dwWriteThreadId);
			dev->SetWriteThreadId(comDev[i].dwWriteThread, comDev[i].dwWriteThreadId);
		}
	}

	LogWriter::LOGX("[platform_uart] platform_uart_init(OK).");

	return PLATFORM_OK;
}

int platform_uart_close_all(void)
{
	int i, id;
	SimCom* dev;

	for(i = 0; i < sizeof(comDev)/sizeof(comDev[0]); i++)
	{
		id = comDev[i].id;
		dev = comDev[i].COM;
		CloseHandle(comDev[i].dwReadThread);
		CloseHandle(comDev[i].dwWriteThread);
		if(dev != NULL) dev->platform_uart_close(id);
		_destroy_com_dev(id);
	}

	return PLATFORM_OK;
}