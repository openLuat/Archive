#include "string.h"
#include "iot_debug.h"
#include "iot_uart.h"

#define uart_print iot_debug_print
#define DEMO_UART_PORT1 OPENAT_UART_1
#define DEMO_UART_PORT2 OPENAT_UART_2
#define DEMO_UART_RECV_TIMEOUT (5 * 1000) // 2S

//中断方式读串口1数据
//注: 中断中有复杂的逻辑,要发送消息到task中处理
void uart_recv_handle(T_AMOPENAT_UART_MESSAGE* evt)
{
    INT8 recv_buff[64] = {0};
    int32 recv_len;
    int32 dataLen = evt->param.dataLen;

    switch(evt->evtId)
    {
        case OPENAT_DRV_EVT_UART_RX_DATA_IND:
    
            recv_len = iot_uart_read(DEMO_UART_PORT1, recv_buff, dataLen , DEMO_UART_RECV_TIMEOUT);

            uart_print("[uart] uart1 OPENAT_DRV_EVT_UART_RX_DATA_IND");
            uart_print("[uart] uart1 recv_len %d, recv_buff %s", recv_len, recv_buff);

            break;

        case OPENAT_DRV_EVT_UART_TX_DONE_IND:
            uart_print("[uart] uart1 OPENAT_DRV_EVT_UART_TX_DONE_IND");
            break;
        default:
            break;
    }
}

VOID demo_uart_read(VOID)
{
    char read_buff[64];
    INT32 read_len;
    
    read_len = iot_uart_read(DEMO_UART_PORT2, read_buff, sizeof(read_buff), DEMO_UART_RECV_TIMEOUT);

    if (read_len <= 0)
        uart_print("[uart] uart2 read timeout");
    else 
        uart_print("[uart] uart2 read_len %d, read_buff %s", read_len, read_buff);
}

VOID demo_uart_write(VOID)
{
    char *write_buff1 = "uart1 hello world";
    char *write_buff2 = "uart2 hello world";
    int32 write_len;
    
    write_len = iot_uart_write(DEMO_UART_PORT1, write_buff1, strlen(write_buff1));
    uart_print("[uart] uart1 write_len %d, write_buff %s", write_len, write_buff1);

    write_len = iot_uart_write(DEMO_UART_PORT2, write_buff2, strlen(write_buff2));
    uart_print("[uart] uart2 write_len %d, write_buff %s", write_len, write_buff2);
}

VOID demo_uart_open(VOID)
{
    BOOL err;
    T_AMOPENAT_UART_PARAM uartCfg;
    
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_115200; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控
    uartCfg.txDoneReport = TRUE; // 设置TURE可以在回调函数中收到OPENAT_DRV_EVT_UART_TX_DONE_IND
    uartCfg.uartMsgHande = uart_recv_handle; //回调函数

    // 配置uart1 使用中断方式读数据
    err = iot_uart_config(DEMO_UART_PORT1, &uartCfg);
    uart_print("[uart] DEMO_UART_PORT1 open err %d", err);

    uartCfg.txDoneReport = FALSE; 
    uartCfg.uartMsgHande = NULL;
    // 配置uart2 使用轮训方式读数据
    err = iot_uart_config(DEMO_UART_PORT2, &uartCfg);
    uart_print("[uart] DEMO_UART_PORT1 open err %d", err);
}

VOID demo_uart_close(VOID)
{
    iot_uart_close(DEMO_UART_PORT1);
    iot_uart_close(DEMO_UART_PORT2);
    uart_print("[uart] close");
}

VOID demo_uart_init(VOID)
{   
    demo_uart_open(); // 打开串口1和串口2 (串口1中断方式读数据, 串口2轮训方式读数据)
    demo_uart_write(); // 向串口1和串口2 写数据

    while(1)
    {
        demo_uart_read(); // 轮训方式读取串口2数据
    }
}

VOID app_main(VOID)
{
    uart_print("[uart] app_main");

    demo_uart_init();
}