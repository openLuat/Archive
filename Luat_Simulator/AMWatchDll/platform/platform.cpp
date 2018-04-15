/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_rtos.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'RTOS' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/
#include "stdafx.h"
#include "assert.h"
#include "platform.h"
#include "platform_conf.h"
#include "platform_i2c.h"
#include "platform_rtos.h"

#if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )
#define BUILD_INT_HANDLERS

#ifndef INT_TMR_MATCH
#define INT_TMR_MATCH         ELUA_INT_INVALID_INTERRUPT
#endif

extern const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ];

#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

int platform_init(void)
{
    //cmn_platform_init();
	init_event();
	gpio_init();
    
    return PLATFORM_OK;
}

// ****************************************************************************
// Timer
void platform_s_timer_delay( unsigned id, UINT32 delay_us )
{
    assert(0);
}
      
UINT32 platform_s_timer_op( unsigned id, int op, UINT32 data )
{
  UINT32 res = 0;
  
  switch( op )
  {
    case PLATFORM_TIMER_OP_START:
    case PLATFORM_TIMER_OP_READ:
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
      break;
  }
  return res;
}


int platform_cpu_set_global_interrupts( int status )
{
    return 0;
}

int platform_cpu_get_global_interrupts()
{
    return 0;
}

void platform_sendsound_end(void)
{
    //strcat(atcbuff,"+DTMFDET:69\r\n");
}

/* 兼容旧版本的sleep接口 */
void platform_os_sleep(UINT32 ms)
{
    Sleep(ms);
}

void platform_assert(const char *func, int line)
{
	assert("platform_assert",func,line);
}


//console
static unsigned char luaConsolePort = 0;
void platform_set_console_port( unsigned char id )
{
    luaConsolePort = id;
}

unsigned char platform_get_console_port(void)
{
    return luaConsolePort;
}

int platform_i2c_exists( unsigned id ) 
{
    if(id != 1) 
        return PLATFORM_ERR;

    return PLATFORM_OK;
}

int platform_i2c_setup( unsigned id, PlatformI2CParam *pParam ) 
{
    return pParam->speed;
}

int platform_i2c_recv_data( unsigned id, const UINT8 *pRegAddr, UINT8 *buf, UINT32 len  )
{
    UINT32 i;
    for(i = 0; i < len; i++)
    {
        buf[i] = i;
    }

    return len;
}

int platform_adc_exists( unsigned id ) 
{
    return id < NUM_ADC;
}

int platform_adc_open(unsigned id, unsigned mode)
{
    return PLATFORM_OK;
}

int platform_adc_close(void)
{
    return PLATFORM_OK;
}

int platform_adc_read(unsigned id, int *adc, int *volt)
{
    *adc = 523;
    *volt = 3800;
    return PLATFORM_OK;
}

