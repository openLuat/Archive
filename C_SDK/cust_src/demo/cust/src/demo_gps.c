#include "string.h"
#include "iot_debug.h"
#include "iot_uart.h"
#include "iot_pmd.h"

#define USART1_MSG_READY (0)
typedef struct {
    UINT8 type;
    UINT8 data;
}USART1_MESSAGE;
static HANDLE g_s_gps_task;

#define DEMO_UART_RECV_TIMEOUT (5 * 1000) // 2S
char buff_command[5]={0};
int32 recv1_len=0,recv2_len=0;
char buff[18]={0x24,0x50,0x47,0x4b,0x43,0x31,0x30,0x31,0x2c,0x33,0x30,0x30,0x30,0x2a,0x30,0x30,0x0d,0x0a};
char buff_error[5]={0x65,0x72,0x72,0x6f,0x72};

char buff_poweron[5]="A=1\n";
char flag_receive=0;
char buff_gps[320]={0};
char buff_position[24]={0x57};
char buff_lng[12]={0x4a};
char n=1,e=1,find=0;
INT32 i;
	
void uart1_recv_handle(T_AMOPENAT_UART_MESSAGE* evt)
{   
    //USART1_MESSAGE* msgptr = iot_os_malloc(sizeof(USART1_MESSAGE));
    int32 dataLen = evt->param.dataLen;
    switch(evt->evtId)
    {
        case OPENAT_DRV_EVT_UART_RX_DATA_IND:
            recv1_len = iot_uart_read(OPENAT_UART_1, buff_command, dataLen , DEMO_UART_RECV_TIMEOUT);
			//msgptr->type = USART1_MSG_READY;
			//ot_os_send_message(g_s_gps_task, (PVOID)msgptr);
            break;

        default:
            break;
    }
}

#if 0
void uart2_recv_handle(T_AMOPENAT_UART_MESSAGE* evt)
{    
    int32 dataLen = evt->param.dataLen;
    switch(evt->evtId)
    {
        case OPENAT_DRV_EVT_UART_RX_DATA_IND:
            recv2_len = iot_uart_read(OPENAT_UART_2, buff_gps, dataLen , DEMO_UART_RECV_TIMEOUT);
			//iot_debug_print("[uart]  %d,%s",recv_len,buff_gps);
            break;

        default:
            break;
    }
}
#endif

static void demo_gps_task(PVOID pParameter)
{
	
	IVTBL(sys32k_clk_out)(1);
	while(1)
	{
		//iot_os_wait_message(g_s_gps_task, (PVOID)&msg);
		// switch(msg->type)
		// {
			// case USART1_MSG_READY:
			// break;
		// }
		if(flag_receive==0)
		{
			if(buff_command[0]=='C')
			{
				switch(buff_command[2])
				{
					case '1':
						iot_pmd_poweron_ldo(OPENAT_LDO_POWER_CAM, 7);//开机
						iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VIB, 7);
						iot_uart_write(OPENAT_UART_1,buff_poweron,strlen(buff_poweron));
						memset(buff_command,0,sizeof(char)*5);
						iot_uart_write(OPENAT_UART_2,buff,sizeof(buff));
						flag_receive=1;
					break;
				}
			}
		}
		else
		{
			memset(buff_gps,0,sizeof(char)*320);
			recv2_len = iot_uart_read(OPENAT_UART_2, buff_gps, sizeof(buff_gps), 100);
			if (recv2_len <= 0)
			{
				
			}
			else
			{
				buff_gps[recv2_len] = 0;
				for(i=0;i<recv2_len;i++)
				{
					if(buff_gps[i]=='E')
					{
						break;
					}
				}
				iot_debug_print("[GPS] --%c  %d",buff_gps[i+2],recv2_len);
				if(buff_gps[i+2]!='1'||buff_gps[i+2]!='2'|| buff_gps[i+2]!='3'|| recv2_len<50)
				{	
					iot_debug_print("[GPS] you should wait the gps!");
					iot_uart_write(OPENAT_UART_1,buff_gps,sizeof(buff_gps));	
				}
				else
				{
					iot_debug_print("[GPS] gps ready!!");
					iot_uart_write(OPENAT_UART_1,buff_gps,sizeof(buff_gps));
					for(i=0;i<recv2_len;i++)
					{
						if(buff_gps[i]==',')
							find++;
						if(find==2)
							buff_position[n++]=buff_gps[i+1];
						if(find==4)
							buff_lng[e++]=buff_gps[i+1];
						if(find==5)
						{
							n--;
							for(i=0;i<e-1;i++)
							{
								buff_position[n+i]=buff_lng[i];
							}
							find=0;
							n=1;
							e=1;
							break;
						}
							
					}
					
					iot_uart_write(OPENAT_UART_1,buff_position,sizeof(buff_position));
					flag_receive=0;
					//iot_pmd_poweron_ldo(OPENAT_LDO_POWER_CAM, 0);
					iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VIB, 0);
				}
						
			}
			
		}
		
		iot_os_sleep(1000);
	}
}


// static void demo_gps_task(PVOID pParameter)
// {
    
	
	// char buff_usart1[5];
	// char buff_position[24]={0x57};
	// char buff_lng[12]={0x4a};
	
	// char flag_receive=0;
	// char n=1,e=1,find=0;
	// char buff_open[]="A=1\n";
	// char write_buff[]="$PGKC101,3000*00\r\n";
    // INT32 i,read_len,len1;
	// while (1)
	// {
		// if(flag_receive==0)
		// {
			// len1=iot_uart_read(OPENAT_UART_1, buff_usart1, sizeof(buff_usart1), 100);
			// //iot_uart_write(OPENAT_UART_1,buff_open,strlen(buff_open));
			// if(len1<=0)
			// {
				
			// }
			// else
			// {
				// #if 1
				// if(buff_usart1[0]=='C')
				// {
					// switch(buff_usart1[2])
					// {
						// case '1':
							// iot_pmd_poweron_ldo(OPENAT_LDO_POWER_CAM, 7);//开机
							// iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VIB, 7);
							// IVTBL(sys32k_clk_out)(1);
							// iot_uart_write(OPENAT_UART_1,buff_open,strlen(buff_open));
							// memset(buff_usart1,0,sizeof(char)*5);
							// //iot_uart_write(OPENAT_UART_2,write_buff,strlen(write_buff));
							// flag_receive=1;
						// break;
					// }
				// }
			// }
		// }
		// else
		// {
			// //memset(buff_usart2,0,sizeof(char)*1024);
			// read_len = iot_uart_read(OPENAT_UART_2, buff_usart2, sizeof(buff_usart2), 100);
			// if (read_len <= 0)
			// {
				
			// }
			// else
			// {
				// //buff_usart2[read_len] = 0;
				// for(i=0;i<read_len;i++)
				// {
					// if(buff_usart2[i]=='E')
					// {
						// break;
					// }
				// }
				// iot_debug_print("[GPS] %c  %d",buff_usart2[i+2],read_len);
				// if(buff_usart2[i+2]=='0' || read_len<50)
				// {	
					// iot_debug_print("[GPS] you should wait the gps!");
					// iot_uart_write(OPENAT_UART_1,buff_usart2,sizeof(buff_usart2));
				// }
				// else
				// {
					// iot_debug_print("[GPS] gps ready!!");
					// iot_uart_write(OPENAT_UART_1,buff_usart2,sizeof(buff_usart2));
					// for(i=0;i<1024;i++)
					// {
						// if(buff_usart2[i]==',')
							// find++;
						// if(find==2)
							// buff_position[n++]=buff_usart2[i+1];
						// if(find==4)
							// buff_lng[e++]=buff_usart2[i+1];
						// if(find==5)
						// {
							// n--;
							// for(i=0;i<e-1;i++)
							// {
								// buff_position[n+i]=buff_lng[i];
							// }
							// find=0;
							// n=1;
							// e=1;
							// break;
						// }
							
					// }
					
					// iot_uart_write(OPENAT_UART_1,buff_position,sizeof(buff_position));
					// flag_receive=0;
					// iot_pmd_poweron_ldo(OPENAT_LDO_POWER_CAM, 0);
					// iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VIB, 0);
				// }
						
			// }
			
		// }
		// #endif
	// }

// }
VOID app_main(VOID)
{
    BOOL err;
    T_AMOPENAT_UART_PARAM uartCfg;

    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_115200; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控
    uartCfg.txDoneReport = TRUE;
    uartCfg.uartMsgHande = uart1_recv_handle;
	iot_uart_config(OPENAT_UART_1, &uartCfg);
    // 配置uart1 使用中断方式读数据
    
	
	uartCfg.txDoneReport = FALSE;
	uartCfg.uartMsgHande = NULL;
	//uartCfg.txDoneReport = TRUE;
    //uartCfg.uartMsgHande = uart2_recv_handle; 
	err = iot_uart_config(OPENAT_UART_2, &uartCfg);
    iot_uart_write(OPENAT_UART_1,buff_error,sizeof(buff_error));	
    g_s_gps_task=iot_os_create_task(demo_gps_task,
                        NULL,
                        4096,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_gps");						
}
