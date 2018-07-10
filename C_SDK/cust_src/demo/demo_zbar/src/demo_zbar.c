#include "string.h"
#include "iot_debug.h"
#include "iot_lcd.h"
#include "iot_camera.h"
#include "iot_pmd.h"
#include "demo_lcd_9341.h"
#include "iot_zbar.h"
#include "iot_uart.h"
#include "demo_zbar.h"


HANDLE g_zbar_handle;
ZBAR_CONTEXT g_zbar_context;

static char zbar_uart_wirte_buff[512];
static int scannerCount = 0;

static void zbar_uart_write(int width, int height, char *scannerData, char* fmt, int time2)
{  
    memset(zbar_uart_wirte_buff, 0, sizeof(zbar_uart_wirte_buff));
    sprintf(zbar_uart_wirte_buff, "(%d*%d)%d,data:%s:%d,code:%s, time %d\r\n", 
        width, height, scannerCount++,scannerData, strlen(scannerData), fmt, time2);
    iot_uart_write(OPENAT_UART_1, zbar_uart_wirte_buff, strlen(zbar_uart_wirte_buff));
}

static void zbar_uart_init(void)
{
    T_AMOPENAT_UART_PARAM uartCfg;
    BOOL result;
    
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_115200; 
    uartCfg.dataBits = 8;   
    uartCfg.stopBits = 1; 
    uartCfg.parity = OPENAT_UART_NO_PARITY; 
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE;
    uartCfg.txDoneReport = TRUE; 
 
    result = iot_uart_config(OPENAT_UART_1, &uartCfg);
    iot_debug_print("[zbar] DEMO_UART_PORT1 open result %d", result);

    iot_uart_write(OPENAT_UART_1, "zbar init\r\n", strlen("zbar init\r\n")); 
}

void zbar_send_message(ZBAR_MESSAGE*  msg)
{
    zbar_scannerStatusSet(ZBAR_SCANNER_STATUS_RUNNING);
    iot_os_send_message(g_zbar_handle, msg);
}

static void zbar_scanner_run(int width, int height, int size, char *dataInput)
{
	int len;
	char *data;
    static int tick1 = 0;
    int tick2, tick3;
    int time1, time2;
    
    if (tick1 == 0)
    {
        tick1 = iot_os_get_system_tick();
    }
    
    tick2 = iot_os_get_system_tick();
    
	//创建句柄， handle != 0 表示解码成功
	int handle = iot_zbar_scannerOpen(width, height, size, dataInput);
    
	if (handle)
	{
		do
		{        
			// 解码成功获取二维码信息
			      data = iot_zbar_getData(handle, &len);
			      data[len] = 0;

            // 将获取的数据通过串口显示
            tick3 = iot_os_get_system_tick();
            time1 = (tick3 - tick1) * 1000 / 16384;
            time2 = (tick3 - tick2) * 1000 / 16384;
            tick1 = iot_os_get_system_tick();
            zbar_uart_write(width, height, data, iot_zbar_getType(handle), time2);
            
			iot_debug_print("[zbar] zbar_scanner_run come in handle_data %s", data);

		 // 判断是否有下一个数据
		}while(iot_zbar_findNextData(handle) > 0);

		// 释放句柄
		iot_zbar_scannerClose(handle);
	}
}

void zbar_scannerStatusSet(ZBAR_SCANNER_STATUS status)
{
    g_zbar_context.status = status;
}

ZBAR_SCANNER_STATUS zbar_scannerStatusGet(void)
{
    return g_zbar_context.status;
}

static void zbar_task_main(PVOID pParameter)
{
    ZBAR_MESSAGE*    msg;

    iot_debug_print("[zbar] zbar_task_main");

    while(1)
    {
        iot_os_wait_message(g_zbar_handle, (PVOID)&msg);

        switch(msg->type)
        {
            case ZBAR_TYPE_CAMERA_DATA:
            {
                // 二维码解码
                if (msg->data)
                   zbar_scanner_run(msg->width, msg->height, msg->dataLen, msg->data);

                zbar_scannerStatusSet(ZBAR_SCANNER_STATUS_IDLE);
                break;
            }
            default:
                break;
        }

        if (msg)
        {
			iot_os_free(msg);
			msg = NULL;
		}
    }
}

void zbar_task(void)
{
    iot_debug_print("[zbar] zbar_task");
    
    zbar_uart_init();
    
    g_zbar_handle = iot_os_create_task((PTASK_MAIN)zbar_task_main, NULL,  15*1024, 0, OPENAT_OS_CREATE_DEFAULT, "zbar task");
    zbar_scannerStatusSet(ZBAR_SCANNER_STATUS_IDLE);
}
