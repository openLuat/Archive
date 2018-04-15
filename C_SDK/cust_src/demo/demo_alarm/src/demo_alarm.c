#include "string.h"
#include "iot_debug.h"
#include "iot_gpio.h"
#include "iot_os.h"

#define alarm_print iot_debug_print

VOID demo_alarm_handle(T_AMOPENAT_ALARM_MESSAGE *pAlarmMessage)
{
    T_AMOPENAT_SYSTEM_DATETIME curTime;

    // 4. 闹钟时间到了
    if (pAlarmMessage->evtId == OPENAT_DRV_EVT_ALARM_IND)
    {
        alarm_print("[alarm] handle index = %x", pAlarmMessage->param.alarmIndex);

        iot_os_get_system_datetime(&curTime);

        alarm_print("[alarm] handle now %u:%u:%u %u:%u:%u", 
            curTime.nYear, curTime.nMonth, curTime.nDay,
            curTime.nHour, curTime.nMin, curTime.nSec);
    }
}


VOID demo_alarm_init(VOID)
{
    T_AMOPENAT_ALARM_CONFIG alarmCfg;
    T_AMOPENAT_ALARM_PARAM pAlarmSet;
    T_AMOPENAT_SYSTEM_DATETIME curTime;
    BOOL err;
    
    //1. 初始化闹钟, 设置闹钟回调函数
    alarmCfg.pAlarmMessageCallback = demo_alarm_handle;
    iot_os_init_alarm(&alarmCfg);

    //2. 获取当前时间, 
    /*   注:如果没有调用iot_os_set_system_datetime接口设置时间,
    *       那时间默认为2012.6.1 10.0.0
    */
    iot_os_get_system_datetime(&curTime);
    alarm_print("[alarm] now %u:%u:%u %u:%u:%u", 
        curTime.nYear, curTime.nMonth, curTime.nDay,
        curTime.nHour, curTime.nMin, curTime.nSec);

    //3. 设置闹钟, 10s后触发闹钟回调
    /*注:alarmRecurrent 1个字节,  第一位置1表示单次定时, 1-7位表示周定时, 
    *      单词定时和周定时不能同时设置
    *      if (((alarmRecurrent & 0xfe) != 0)
    *             && ((pAlarm->nRecurrent & 0x1) != 0))
    *           return FALSE;
    */
    pAlarmSet.alarmIndex = 0; // 定时索引范围(0-14)
    pAlarmSet.alarmOn = TRUE; // TURE设置, FALSE 清除
    pAlarmSet.alarmRecurrent = 1; // 设置1单次定时
    memcpy(&pAlarmSet.alarmTime, &curTime, sizeof(T_AMOPENAT_SYSTEM_DATETIME));
    pAlarmSet.alarmTime.nSec += 10; // 设置闹钟时间
    err = iot_os_set_alarm(&pAlarmSet);

    if (!err)
        return;
    
    alarm_print("[alarm] set alarm %u:%u:%u %u:%u:%u", 
        pAlarmSet.alarmTime.nYear, pAlarmSet.alarmTime.nMonth, pAlarmSet.alarmTime.nDay,
        pAlarmSet.alarmTime.nHour, pAlarmSet.alarmTime.nMin, pAlarmSet.alarmTime.nSec);
}

VOID app_main(VOID)
{
    alarm_print("[alarm] app_main");

    demo_alarm_init();
}