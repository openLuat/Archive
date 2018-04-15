/*******************************************************
**
** This is just demo code for custom application.
** Your can implement your own main entry file instead 
** of this demo file, but two things you must pay attention
** 1. disable cust_main module compilation
**     modify module_list.mk, remark LOCAL_MODULE_DEPENDS += cust_main
** 2. copy code from this file
**     pls find "ATTENTION" in this file.
*******************************************************/
/*******************************************************
* Date        What
* 2013.01.15  SPI测试代码用宏 SPI_TEST_SUPPORT 控制
*             (因为与LCD的PIN存在冲突)
* 2013.01.15  匹配新的LCD初始化接口
*
* 2013.01.15  增加 AUDIO 接口测试例程
* 2013.01.17  增加T卡测试代码
* 2013.01.17  合并彩屏与黑白屏测试代码,触摸屏测试代码
* 2013.01.30  add psam card test code with PSAM_TEST_SUPPORT
* 2013.02.03  add camera test code with CAMERA_TEST_SUPPORT
* 2013.02.06  Adapte to new power on interface
* 2013.02.10  add video play test code with VIDEO_PLAYER_TEST_SUPPORT
* 2013.03.05  add GPIO keypad support
* 2013.03.21  adapte GPIO keypad support
* 2013.03.21  adapte default lcd cs pin to  unkown
* 2013.03.30  adapte pmd cfg  support
* 2013.04.10  adapte pmd default type and add  UART_PRINT_SUPPORT
* 2013.04.12  adapte pmd signal handle
* 2013.04.19  adapte pmd cfg
* 2013.05.20  adapte pmd cfg
* 2013.07.15  添加uart口cfg.uartMsgHande = NULL;，防止客户遗漏，导致程序死机
* 2013.07.18  add  audio hw channel interface  with set_hw_channel
* 2013.08.08  add adc key report data
* 2013.09.24  添加NV接口，使用该接口需要8K的空间
********************************************************/

#define CUST_MONO_LCD 0
#define CUST_COLOR_LCD 1
#define CUST_NONE_LCD  0xff

#define SPI_TEST_SUPPORT 0
#define TFLASH_TEST      0
#define TOUCH_SCREEN_SUPORT 0
#define CUST_LCD_SUPPORT CUST_MONO_LCD
#define PSAM_TEST_SUPPORT 0
#define CAMERA_TEST_SUPPORT 0
#define VIDEO_PLAYER_TEST_SUPPORT 0
#define UART_PRINT_SUPPORT  0
#define NV_SUPPORT        0
#define BT_SUPPORT        0
#define GPS_SUPPORT      0

#include "string.h"
#include "stdio.h"

#include "iot_os.h"
#include "iot_enc.h"
#include "iot_debug.h"
#include "iot_pmd.h"


typedef struct {
    UINT8 type;
    UINT16 len;
    PVOID data;
}CUST_MESSAGE;


#define ASSERT(condition) iot_debug_assert(condition, (CHAR*)__FUNCTION__, __LINE__)


static VOID cust_task_main(PVOID pParameter);

static void cust_pm_message(T_AMOPENAT_PM_MSG* pmMessage);
static VOID cust_poweron_reason_init(VOID);


void cust_gps_init(void);

/* 加密卡密钥等信息 */
UINT8 enc_data[] = {
    0,/* 加密卡类型 0-一般v4加密卡 1-v4加密卡(不进行sim卡refresh) 2-一般v2加密卡 ) 3-江苏v2 4-江苏鉴权卡 */
    0x03,/* 厂商代码 */
    0x01,/* 密钥版本号 */
    1,/* 是否支持普通卡 */
    0,/* 是否只发送一次terminal profile */
    
    /* 密钥 */
    0xC0,0x7A,0x82,0x09,0xB9,0x58,0x5C,0x8D,0x36,0x42,0xF5,0x0A,0xAA,0x01,0xCE,0x85,
    0xA4,0xC9,0xA8,0x50,0x94,0x9A,0x22,0x78,0xBD,0x67,0xCD,0x07,0x11,0x47,0xD4,0x12,
    0xB1,0x09,0x47,0xA6,0x24,0x64,0x67,0xF5,0x66,0x10,0x34,0x7B,0xB8,0x15,0xA5,0x83,
    0xF1,0x3A,0xE6,0x3C,0x1B,0xEB,0x06,0x6D,0x2A,0x0C,0x7F,0x33,0x04,0x1C,0x55,0x61,
    0x6B,0xFA,0xEE,0x2B,0x4F,0xF2,0x74,0x2C,0x3A,0xB0,0xA1,0xBA,0x3A,0xBC,0x4B,0x85,

    /* 江苏鉴权卡密钥 */
	0x11,0x60,0x59,0x04,0x9B,0x8C,0x75,0x1B,0x5A,0x7E,0xB9,0x54,0xC0,0xA7,0x2D,0xFB,
    0x72,0x7D,0xE5,0x8B,0xDA,0x56,0x3A,0xF7,0x48,0x9A,0x0A,0xDE,0xAA,0x49,0x58,0x51,
    0xAC,0x58,0x33,0xF7,0xCF,0x24,0x89,0x3B,0xDD,0xEF,0xFF,0x68,0xB8,0xED,0x4A,0xC6,
    0xE6,0x5E,0xB2,0xC5,0x7C,0xCD,0x00,0xDA,0xDB,0x34,0xA9,0x82,0x0B,0xD8,0x00,0x63,
    0x6A,0x0A,0x8B,0x5B,0x86,0x78,0xA9,0xF7,0x3B,0xEB,0xBD,0x9E,0xD3,0xD5,0x99,0xBE,
    0x09,0xDE,0x1E,0x3A,0xD5,0xC6,0xB5,0xBC,0x39,0xBA,0x51,0x75,0x7A,0x6B,0x73,0x50,
    0x81,0x79,0x41,0x68,0xA8,0x69,0x80,0x07,0x78,0x02,0x09,0x53,0x1C,0x76,0x3B,0x97,
    0x24,0xD6,0x78,0x13,0x91,0xBC,0xE1,0x05,0x11,0x31,0x13,0x9E,0xDF,0x7B,0xA6,0xDB,
    0x61,0xB7,0x7F,0x22,0xB8,0x9B,0x19,0x5C,0x30,0x74,0x1D,0x11,0x3A,0xFB,0x30,0xF3,
    0x8A,0xA0,0x34,0xE2,0x37,0x97,0xD1,0x67,0xAA,0x36,0x11,0xD1,0xC2,0x0C,0x9F,0x97,
    0x5E,0x5A,0xE5,0x6D,0x8F,0xB2,0xFF,0x4E,0x5D,0x2F,0x3A,0x81,0x56,0x02,0xAA,0x42,
    0x39,0x49,0x7A,0xC3,0x7E,0x42,0xD9,0xD2,0x46,0x4B,0xB4,0xA2,0x06,0x0C,0xE6,0x6B,
    0xE9,0x6F,0xC7,0xEA,0xED,0xF2,0x93,0xB1,0x15,0x5D,0xBC,0xB3,0xAF,0x7B,0xB3,0x24,
    0x0E,0x8C,0x50,0xCA,0x89,0x69,0x82,0xF5,0x25,0x17,0xA3,0x45,0x55,0xC8,0x9A,0xF6,
    0xD7,0xD0,0x92,0x26,0x3B,0x47,0x00,0xBF,0x91,0xDC,0x81,0xF6,0xDF,0x35,0xB1,0xF5,
    0x06,0x43,0xB7,0x8D,0x3A,0x9A,0x13,0x8C,0x36,0x15,0xFC,0x22,0x15,0xF1,0xD8,0x00
};
/*******************************************************
** ATTENTION: For our cust, this part must used. If you 
** have your own main entry file(instead of cust_main.c),
** pls copy this part!
**
** START .. START .. START .. START .. START .. START ..
********************************************************/
/* CUSTOM APP CODE INFOMATION */
UINT8 cust_init(T_AMOPENAT_INTERFACE_VTBL *pVtable);
VOID cust_main(VOID);
extern char _am_openat_ro_lma;
extern char _am_openat_ro_vma;
extern char _am_openat_ro_size;
extern char _am_openat_rw_lma;
extern char _am_openat_rw_vma;
extern char _am_openat_rw_size;
extern char _am_openat_zi_base;
extern char _am_openat_zi_size;
static const volatile T_AMOPENAT_CUST_VTBL __attribute__((section (".am_openat_custvtbl")))
    g_s_AMOpenAtCustVtbl =
{
    0x87654321,
    (UINT32)&_am_openat_ro_lma,
    (UINT32)&_am_openat_ro_vma,
    (UINT32)&_am_openat_ro_size,
    (UINT32)&_am_openat_rw_lma,
    (UINT32)&_am_openat_rw_vma,
    (UINT32)&_am_openat_rw_size,
    (UINT32)&_am_openat_zi_base,
    (UINT32)&_am_openat_zi_size,

    cust_init,  /* cannot changed to other name */
    cust_main,  /* cannot changed to other name */
};
/*******************************************************
** ATTENTION:
** END .. END .. END .. END .. END .. END .. END ..
********************************************************/

/* Function table from OpenAT platform */
T_AMOPENAT_INTERFACE_VTBL * g_s_InterfaceVtbl = 0;
HANDLE g_CustTaskHandle;

struct
{
    HANDLE poweronSyncSemaphore;
    E_AMOPENAT_POWERON_REASON reason;
}g_PowronInfo;

/* The first function called by OpenAT platform */
/* Return value see more E_AMOPENAT_CUST_INIT_RESUL */
UINT8 cust_init(T_AMOPENAT_INTERFACE_VTBL *pVtable)
{
    g_s_InterfaceVtbl = pVtable;
    g_PowronInfo.reason = OPENAT_PM_POWERON_BY_INVALID;
    g_PowronInfo.poweronSyncSemaphore = 0;
    
    return (UINT8)OPENAT_CUST_INIT_RES_OK;
}

/* never removed even if not used */
const T_AMOPENAT_CUST_VTBL* cust_getcustvtbl(VOID)
{
    return (const T_AMOPENAT_CUST_VTBL*)&g_s_AMOpenAtCustVtbl;
}

/* Main function call by OpenAT platform */
VOID cust_main(VOID)
{
    T_AMOPENAT_PMD_CFG pmdcfg;
    E_AMOPENAT_PM_CHR_MODE pmdmode;
    
    /* 更新加密卡密钥信息 长度必须为341*/
    iot_enc_set_info(enc_data, sizeof(enc_data));
    
    /* 密钥已经准备好 需要发出通知 */
    iot_enc_set_data_ok();                   
    
    cust_poweron_reason_init();
  
    memset(&pmdcfg, 0, sizeof(T_AMOPENAT_PMD_CFG));
    /*模块内置充电方案*/
    pmdmode = OPENAT_PM_CHR_BY_DEFAULT;
    /*和硬件设计有关*/
    pmdcfg.deFault.batdetectEnable = TRUE; 
    pmdcfg.deFault.tempdetectEnable = FALSE;
    pmdcfg.deFault.templowLevel = 0;
    pmdcfg.deFault.temphighLevel = 0;
    pmdcfg.deFault.batLevelEnable = FALSE;
    pmdcfg.deFault.batfullLevel = 4200;
    pmdcfg.deFault.batPreChargLevel = 4050;
    pmdcfg.deFault.poweronLevel = 3450;
    pmdcfg.deFault.poweroffLevel = 3400;
    pmdcfg.deFault.batAdc = OPENAT_ADC_7;/*adc_sense  or adc_vbat:OPENAT_ADC_0*/
    pmdcfg.deFault.tempAdc = OPENAT_ADC_1; 
    /*level:  poweron-----levelFirst-----levelSecond-----levelFull*/
    /*current:----currentFirst----currentSecond---currentThird----*/
    pmdcfg.deFault.currentControlEnable = FALSE; 
    pmdcfg.deFault.currentFirst = OPENAT_PM_CHARGER_500MA;
    pmdcfg.deFault.intervalTimeFirst = 9*60; /*9分钟*/
    pmdcfg.deFault.batLevelFirst = 4150;
    pmdcfg.deFault.currentSecond = OPENAT_PM_CHARGER_300MA;
    pmdcfg.deFault.intervalTimeSecond = 6*60;/*6分钟*/
    pmdcfg.deFault.batLevelSecond = 4190;
    pmdcfg.deFault.currentThird = OPENAT_PM_CHARGER_100MA;
    pmdcfg.deFault.intervalTimeThird = 3*60; /*3分钟*/  
    pmdcfg.deFault.chargTimeOutEnable = FALSE;
    pmdcfg.deFault.TimeOutMinutes = 240;
    pmdcfg.deFault.disableCharginCall = FALSE;

    ASSERT(iot_pmd_init(pmdmode, &pmdcfg, cust_pm_message));

    /* 创建custom app线程 */
    g_CustTaskHandle = iot_os_create_task((PTASK_MAIN)cust_task_main, NULL,  15/*8*/*1024, 0, OPENAT_OS_CREATE_DEFAULT, "cust task");

    if(OPENAT_INVALID_HANDLE == g_CustTaskHandle)
    {
        ASSERT(0);
    }
}



static void cust_pm_message(T_AMOPENAT_PM_MSG* pmMessage)
{
    switch(pmMessage->evtId)
    {
        case OPENAT_DRV_EVT_PM_POWERON_ON_IND:
            if(OPENAT_PM_POWERON_BY_INVALID == g_PowronInfo.reason)
            {
                g_PowronInfo.reason = pmMessage->param.poweronind.powerOnReason;
                if(0 != g_PowronInfo.poweronSyncSemaphore)
                {
                    iot_os_release_semaphore(g_PowronInfo.poweronSyncSemaphore);
                }
            }
            else
            {
                /* received poweron reason message again */
            }
            //IVTBL(print)("[custPM]:poweron reason: %d",pmMessage->param.poweronind.powerOnReason);
            break;
        case OPENAT_DRV_EVT_CHR_PRESENT_IND:
            //IVTBL(print)("[custPM]:chr present: %d",pmMessage->param.chrpresentind.present);
            break;
        case OPENAT_DRV_EVT_BAT_PRESENT_IND:
            //IVTBL(print)("[custPM]:bat present: %d",pmMessage->param.batpresentind.present);
            break;
        case OPENAT_DRV_EVT_BAT_LEVEL_IND:
            //IVTBL(print)("[custPM]:bat level: %d",pmMessage->param.batlevelind.batteryLevel);
            break;
        case OPENAT_DRV_EVT_BAT_CHARGING:
            //IVTBL(print)("[custPM]:charging level: %d",pmMessage->param.chargingind.batteryLevel);
            break;
        case OPENAT_DRV_EVT_BAT_CHR_FULL:
            //IVTBL(print)("[custPM]:full level: %d",pmMessage->param.chrfullind.batteryLevel);
            break;
        case OPENAT_DRV_EVT_BAT_CHR_STOP:
            //IVTBL(print)("[custPM]:chr stop reason: %d",pmMessage->param.chrstopind.chrStopReason);
            break;
        default:
            break;
    }
}



static VOID cust_poweron_reason_init(VOID)
{
    if(0 == g_PowronInfo.poweronSyncSemaphore)
    {
        g_PowronInfo.poweronSyncSemaphore = iot_os_create_semaphore(0);
        ASSERT(0 != g_PowronInfo.poweronSyncSemaphore);
    }
}

static VOID cust_wait_for_poweron_reason(BOOL bDeleteSema)
{
    ASSERT(0 != g_PowronInfo.poweronSyncSemaphore);

    iot_os_wait_semaphore(g_PowronInfo.poweronSyncSemaphore, 0);

    if(TRUE == bDeleteSema)
    {
        iot_os_delete_semaphore(g_PowronInfo.poweronSyncSemaphore);
        g_PowronInfo.poweronSyncSemaphore = 0;
    }
}

bool cust_task_send_message(UINT8 type)
{
    CUST_MESSAGE *pMessage = NULL;

    pMessage = (CUST_MESSAGE *)iot_os_malloc(sizeof(CUST_MESSAGE));

    if (!pMessage)
        return FALSE;

    pMessage->type = type;
    iot_os_send_message(g_CustTaskHandle, pMessage);

    return TRUE;
}

            

/* custom app main */
static VOID cust_task_main(PVOID pParameter)
{
    /* wait for power on message from pm interface */
    cust_wait_for_poweron_reason(TRUE);

    iot_pmd_poweron_system(OPENAT_PM_STARTUP_MODE_DEFAULT, OPENAT_PM_STARTUP_MODE_DEFAULT);

    extern VOID app_main(VOID);
    app_main();
    while(1)
    {
        //sleep 5s
        iot_os_sleep(5000*16384/1000);
        iot_debug_print("idle task run....");
    }
}

