#include "string.h"
#include "iot_debug.h"
#include "iot_uart.h"
#include "iot_pmd.h"
#include "NmeaParser.h"
#include "RxP.h"
#include "commUtil.h"

#define GPS_PRINT iot_debug_print


extern void example_init();
extern void example_proc();

/*以下函数用于获取GPS关键数据*/
extern NMEA_RMC_T* gpsDataGetRmc(void);
extern GNSS_GSV_T* gpsDataGetGSV(void);
extern GNSS_GSA_T* gpsDataGetGSA(void);
extern NMEA_GGA_T* gpsDataGetGGA(void);

static void rxDataInd(T_AMOPENAT_UART_MESSAGE* evt)
{
  char data[1024];
  int len;
  if(evt->evtId == OPENAT_DRV_EVT_UART_RX_DATA_IND)
  {
    len = iot_uart_read(OPENAT_UART_2, data, evt->param.dataLen, 0);

    if(len > 0)
    {
      int i;
      for(i = 0; i < len; i++)
      { 
        rxp_pcrx_nmea(data[i]);
      }
    }
  }
}

static BOOL gpsIsFixed()
{

  NMEA_RMC_T* rmc = gpsDataGetRmc();
 
  
  if(rmc->cStatus == 'A')
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static void demo_gps_task(PVOID pParameter)
{
  T_AMOPENAT_UART_PARAM uartCfg;

  UINT16 i, staUsed, staView;

  char latStr[32];
  char lonStr[32];

  NMEA_GGA_T* ggaPtr;
  GNSS_GSA_T* gsaPtr;
  GNSS_GSV_T* gsvPtr;

  memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
  uartCfg.baud = OPENAT_UART_BAUD_115200;
  uartCfg.dataBits = 8;
  uartCfg.stopBits = 1;
  uartCfg.parity = OPENAT_UART_NO_PARITY; 
  uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; 
  uartCfg.txDoneReport = FALSE;
  uartCfg.uartMsgHande = rxDataInd;

  iot_uart_config(OPENAT_UART_2, &uartCfg);


  example_init();
    

  while(1)
  {
    staUsed = 0;
    staView = 0;
    example_proc();

    iot_os_sleep(1000);

    /*输出GPS数据*/

    gsvPtr = gpsDataGetGSV();
    gsaPtr = gpsDataGetGSA();
    ggaPtr = gpsDataGetGGA();


    for(i = 0, staUsed = 0; i < gsaPtr->i2NumGNSS; i++)
    {
      staUsed += gsaPtr->u2ActiveGNSS[i];
    }

    for(i = 0; i < gsvPtr->i2NumGNSS; i++)
    {
      staView += gsvPtr->i2GnssStaView[i];
    }

    GPS_PRINT("[gps] satellites = (%d,%d)  signal = (%d-%d)", staUsed, staView, 
          gsvPtr->i2MinSNR, gsvPtr->i2MaxSNR);

    if(gpsIsFixed())
    {
      commUtilFormatFloat(latStr, 32, ggaPtr->dfLatitude, 6);
      commUtilFormatFloat(lonStr, 32, ggaPtr->dfLongitude, 6);
      GPS_PRINT("[gps] is fixed [mode = %cD LOC = (%s,%s)]", gsaPtr->cFixMode, 
              latStr, lonStr);
    }
    else
    {
      GPS_PRINT("[gps] is not fix");
    }
  }
  
}



VOID app_main(VOID)
{
    /*1, 打开GPS供电和32K时钟*/
    iot_pmd_poweron_ldo(OPENAT_LDO_POWER_CAM, 7);
    IVTBL(sys32k_clk_out)(1);
    /*2, 新建TASK解析GPS串口数据*/
    iot_os_create_task(demo_gps_task,
                        NULL,
                        40960,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_gps");
}
