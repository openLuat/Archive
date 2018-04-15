#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_gpio.h"

static void light_init()
{
    T_AMOPENAT_GPIO_CFG cfg = {0};
    cfg.mode = OPENAT_GPIO_OUTPUT;

    //Air20x(RDA8955)
    iot_gpio_config(OPENAT_GPIO_33, &cfg);
    //Air80x(RDA8955)
    iot_gpio_config(OPENAT_GPIO_28, &cfg);
}

static void light_flash()
{
    static BOOL value = TRUE;
    iot_gpio_set(OPENAT_GPIO_33, value);
    iot_gpio_set(OPENAT_GPIO_28, value);
    value = !value;
}

void app_main(void)
{
    
    iot_debug_print("[mini_system] app_main");

    light_init();


    while(1)
    {
        iot_os_sleep(1000);
        light_flash();
        iot_debug_print("[mini system] loop");
    }
}