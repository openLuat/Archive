#include "string.h"
#include "iot_debug.h"
#include "iot_sys.h"

static HANDLE g_s_vatTimer;
VOID vatAtIndHandle(UINT8 *pData, UINT16 length)
{
	iot_debug_print("[vat] AT RECV: %s", pData);
}


VOID vatTimerhandle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
	iot_debug_print("[vat] AT SEND: AT+CSQ");
    iot_vat_sendATcmd("AT+CSQ\r\n", strlen("AT+CSQ\r\n"));
	iot_os_start_timer(g_s_vatTimer, 2000);
}
VOID app_main(VOID)
{
    iot_debug_print("[vat] app_main");

    iot_vat_init(vatAtIndHandle);

	g_s_vatTimer = iot_os_create_timer(vatTimerhandle, NULL);
	
	iot_os_start_timer(g_s_vatTimer, 2000);
}