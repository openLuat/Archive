#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"

typedef struct 
{
    UINT8 timer_name[20];
    UINT32 count;
} T_DEMO_TIMER_PARAM;

#define timer_print iot_debug_print
#define TIMER_1S (1000)


T_DEMO_TIMER_PARAM g_timer1Param;
T_DEMO_TIMER_PARAM g_timer2Param;

HANDLE g_demo_timer1;
HANDLE g_demo_timer2;

VOID timer1_handle(T_AMOPENAT_TIMER_PARAMETER *pParameter);

VOID demo_timer_create(VOID)
{
    /* 注: 创建定时器 
    *      pFunc可以设置的回调函数可以不同, 
    *      pParameter 可以传NULL
    */     
    timer_print("[timer] create timer1");
    g_demo_timer1 = iot_os_create_timer(timer1_handle, (PVOID)&g_timer1Param);

    timer_print("[timer] create timer2");
    g_demo_timer2 = iot_os_create_timer(timer1_handle, (PVOID)&g_timer2Param);
}


VOID demo_timer_start(VOID)
{
    memset(&g_timer1Param, 0, sizeof(T_DEMO_TIMER_PARAM));
    memset(&g_timer2Param, 0, sizeof(T_DEMO_TIMER_PARAM));
    
    memcpy(g_timer1Param.timer_name, "timer1", strlen("timer1"));
    g_timer1Param.count++;
    memcpy(g_timer2Param.timer_name, "timer2", strlen("timer2"));
    g_timer2Param.count++;

    timer_print("[timer] start timer1");
    iot_os_start_timer(g_demo_timer1, 1*TIMER_1S);

    timer_print("[timer] start timer2");
    iot_os_start_timer(g_demo_timer2, 2*TIMER_1S);
}

VOID demo_timer_check_available(VOID)
{
    timer_print("[timer] check time1 available %d", 
            iot_os_available_timer(g_demo_timer1));
    
    timer_print("[timer] check time1 available %d", 
            iot_os_available_timer(g_demo_timer2));
}

VOID demo_timer_stop_and_del(HANDLE handle)
{
    iot_os_stop_timer(handle);
    iot_os_delete_timer(handle);
}

VOID timer1_handle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    T_DEMO_TIMER_PARAM *timerParam = (T_DEMO_TIMER_PARAM *)pParameter->pParameter;

    timer_print("[timer] name %s, count %d ", timerParam->timer_name, timerParam->count);

    // 4.定时器1和定时器2各重复5次,停止并删除定时器
    if (timerParam->count < 5) 
    {
        timerParam->count++;
        iot_os_start_timer(pParameter->hTimer, pParameter->period);
    }
    else
    {   
        timer_print("[timer] stop and del %s ", timerParam->timer_name);
        demo_timer_stop_and_del(pParameter->hTimer);
    }
}

VOID demo_timer_init(VOID)
{
    //1. 创建定时器1和定时器2
    demo_timer_create();

    //2. 开启定时器1和定时器2
    demo_timer_start();

    //3. 检测定时器是否开启
    demo_timer_check_available();
}

VOID app_main(VOID)
{
    timer_print("[timer] app_main");

    demo_timer_init();
}