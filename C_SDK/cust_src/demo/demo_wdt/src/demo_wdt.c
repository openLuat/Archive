#include "string.h"
#include "iot_debug.h"
#include "iot_gpio.h"
#include "iot_pmd.h"

#define wdt_print iot_debug_print
#define DEMO_WDT_30 OPENAT_GPIO_30
#define DEMO_WDT_31 OPENAT_GPIO_31

int PinSetOutPutMode(E_AMOPENAT_GPIO_PORT pin,unsigned char value)
{

    T_AMOPENAT_GPIO_CFG  output_cfg;
    BOOL err;

    memset(&output_cfg, 0, sizeof(T_AMOPENAT_GPIO_CFG));

    output_cfg.mode = OPENAT_GPIO_OUTPUT; //配置输出
    output_cfg.param.defaultState = value; // 默认
    err = iot_gpio_close(pin);      
    if (!err)
    {
        wdt_print("ERROR %s %d iot_gpio_close",__FILE__,__LINE__);
        return -1;
    }

    err = iot_gpio_config(pin, &output_cfg);
    if (!err)
    {
        wdt_print("ERROR %s %d iot_gpio_config",__FILE__,__LINE__);
        return -1;
    }
    return 0;
}
int PinSetInPutMode(E_AMOPENAT_GPIO_PORT pin,unsigned char value)
{

    T_AMOPENAT_GPIO_CFG  input_cfg;

    BOOL err;

    memset(&input_cfg, 0, sizeof(T_AMOPENAT_GPIO_CFG));

    input_cfg.mode = OPENAT_GPIO_INPUT; //配置输入
    input_cfg.param.defaultState = value;
    
    err = iot_gpio_close(pin);      
    if (!err)
    {
        wdt_print("ERROR %s %d iot_gpio_close",__FILE__,__LINE__);
        return -1;
    }

    err = iot_gpio_config(pin, &input_cfg);
    if (!err)
    {
        wdt_print("ERROR %s %d iot_gpio_config",__FILE__,__LINE__);
        return -1;
    }
    return 0;
}
static VOID demo_wdt_task_main(PVOID pParameter)
{
    wdt_print("[os] demo_wdt_task_main");
    BOOL err;
    int ret = 0;
    
    ret = PinSetOutPutMode(OPENAT_GPIO_30,0);
    if (ret == -1)
    {
        wdt_print("ERROR %s %d PinSetOutPutMode",__FILE__,__LINE__);
        return;
    }
    ret = PinSetOutPutMode(OPENAT_GPIO_31,0);
    if (ret == -1)
    {
        wdt_print("ERROR %s %d PinSetOutPutMode",__FILE__,__LINE__);
        return;
    }
    
    iot_gpio_set(OPENAT_GPIO_30, 1); //设置为高电平
    iot_gpio_set(OPENAT_GPIO_31, 1); //设置为高电平
    
    wdt_print("[wdt] set OPENAT_GPIO_30 --> output");
    wdt_print("[wdt] set OPENAT_GPIO_31 --> output");
    
    while(1)
    {
        wdt_print("set  OPENAT_GPIO_31 --> 0");
        iot_gpio_set(OPENAT_GPIO_31, 0); //设置为低电平
        wdt_print("wdt.taskWdt AirM2M --> WATCHDOG : OK");
        //sleep 2s
        wdt_print("sleep 2000");
        iot_os_sleep(2000);
        wdt_print("set  OPENAT_GPIO_31 --> Input mode");
        ret = PinSetInPutMode(OPENAT_GPIO_31,1);
        if (ret == -1)
        {
            wdt_print("ERROR %s %d PinSetInPutMode",__FILE__,__LINE__);
            return;
        }
        for(int i=0;i<30;i++)
        {   
            UINT8 value=0xff;
            err = iot_gpio_read(OPENAT_GPIO_31, &value);        

            wdt_print("read  OPENAT_GPIO_31 value:%d",value);
            if (!err)
            {
                wdt_print("ERROR %s %d",__FILE__,__LINE__);
                return;
            }
            if(value != 0)
            {
                wdt_print("read  OPENAT_GPIO_31 value != 0");
                wdt_print("sleep 100");
                iot_os_sleep(100);
            }
            else
            {
                wdt_print("Wdt  AirM2M <-- WatchDog : OK value:%d",value);
                break;
            }

            if(i == 29)
            {
                wdt_print("i == 29");
                wdt_print("set  OPENAT_GPIO_30 --> 0");
                iot_gpio_set(OPENAT_GPIO_30, 0); 
                wdt_print("wdt.taskWdt WatchDog <--> AirM2M didn't respond : wdt reset");
                wdt_print("sleep 100");
                iot_os_sleep(100);
                wdt_print("set  OPENAT_GPIO_30 --> 1");
                iot_gpio_set(OPENAT_GPIO_30, 1); 
            }
        }
       
        wdt_print("sleep 120s");
        iot_os_sleep(120000); 
        ret = PinSetOutPutMode(OPENAT_GPIO_31,1);
        if (ret == -1)
        {
            wdt_print("ERROR %s %d PinSetInPutMode",__FILE__,__LINE__);
            return;
        }
        iot_gpio_set(OPENAT_GPIO_31, 1); //设置为高电平
    }
    
}

HANDLE demo_wdt_task;
VOID app_main(VOID)
{
    wdt_print("[wdt] app_main");

    //1. 创建wdt task , 
    demo_wdt_task =  iot_os_create_task(demo_wdt_task_main, NULL, 2048, 1, OPENAT_OS_CREATE_DEFAULT, "wdt_task");
}
