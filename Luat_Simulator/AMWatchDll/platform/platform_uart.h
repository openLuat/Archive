/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_uart.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Simulated COM.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#if !defined(__PLATFORM_UART_H__)
#define __PLATFORM_UART_H__

#include "cycle_queue.h"

#define SUART_THREAD_STACK_SIZE (8*1024)
#define COMMAND_LINE_SIZE    (180)
#define NULL_TERMINATOR_LENGTH (1)
#define MAX_UART_LEN	(256)

#define COM_RX_BUF_SIZE (1024)
#define COM_TX_BUF_SIZE (1024)
#define COM_NAME_SIZE   (30)

enum
{
	SUART0,
	SUART1,
	SUART2,
	SUART3,
};

#define CUSTOM_TO_UPPER(s) ((s >= 'a' && s <= 'z') ? (s-'a' +'A') : s)

typedef enum
{
	CUSTOM_RSP_ERROR = -1,
	CUSTOM_RSP_OK = 0,
	CUSTOM_RSP_LATER
} custom_rsp_type_enum;

typedef enum
{
	CUSTOM_WRONG_MODE,
	CUSTOM_SET_OR_EXECUTE_MODE,
	CUSTOM_READ_MODE,
	CUSTOM_TEST_MODE,
	CUSTOM_ACTIVE_MODE
} custom_cmd_mode_enum;

typedef struct 
{
	short  position;
	short  length;
	char   character[COMMAND_LINE_SIZE + NULL_TERMINATOR_LENGTH];
} custom_cmdLine;

typedef struct
{
	char *commandString;
	custom_rsp_type_enum (*commandFunc)(custom_cmdLine *commandBuffer_p, char* resp);
} custom_atcmd;

class SimCom
{
	unsigned id;
	CHAR name[COM_NAME_SIZE+1];
	HANDLE dwReadThread;
	HANDLE dwWriteThread;
	DWORD dwReadThreadId;
	DWORD dwWriteThreadId;
	HANDLE semWrite;
	CycleQueue txq;
	CycleQueue rxq;

public:
	SimCom(unsigned id, CHAR *name);
	unsigned GetSimComId();
	CHAR* GetSimComName();
	void SetReadThreadId(HANDLE thread, DWORD threadId);
	void SetWriteThreadId(HANDLE thread, DWORD threadId);
	UINT32 platform_uart_setup(unsigned id, UINT32 baud, int databits, int parity, int stopbits, UINT32 mode);
	INT platform_s_uart_set_flow_control(unsigned id, int type);
	UINT32 platform_s_uart_send(unsigned id, UINT8 data);
	UINT32 platform_s_uart_send_buff(unsigned id, UINT8* buff, UINT16 len);
	INT platform_s_uart_recv(unsigned id, INT32 timeout);
	UINT32 platform_uart_close(unsigned id);
	void simulate_uart_rx_thread(void); // read
	void simulate_uart_tx_thread(void); // write

private:

};

typedef struct ComDevTag
{
	int id;
	char name[COM_NAME_SIZE+1];
	SimCom *COM;
	HANDLE dwReadThread;
	DWORD dwReadThreadId;
	HANDLE dwWriteThread;
	DWORD dwWriteThreadId;
}ComDev;

#endif //__PLATFORM_UART_H__