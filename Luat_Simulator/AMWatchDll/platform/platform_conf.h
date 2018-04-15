/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_conf.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Define some configurations for 'AMLuaDebug'.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"

//#define WIN32_PLATFORM_CONF

#define FS_PATH_LEN (512) 

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               2 // port 0:gpio0-31; port 1:gpio32-gpio55; port 2: gpio ex;
#define NUM_SPI               0
#define NUM_UART              4
#define NUM_TIMER             2
#define NUM_PWM               0
#define NUM_ADC               8
#define NUM_CAN               0
#define NUM_I2C               3

#define PIO_PIN_ARRAY         {32 /* gpio_num 32 */, 24/* gpo_num 56 */}

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'

//–Èƒ‚at√¸¡ÓÕ®µ¿
#define PLATFORM_UART_ID_ATC              0x7f

#define PLATFORM_PORT_ID_DEBUG            0x80

#define LUA_PLATFORM_LIBS_ROM \
    _ROM( AUXLIB_BIT, luaopen_bit, bit_map ) \
    _ROM( AUXLIB_BITARRAY, luaopen_bitarray, bitarray_map ) \
    _ROM( AUXLIB_PACK, luaopen_pack, pack_map ) \
	_ROM( AUXLIB_PIO, luaopen_pio, pio_map ) \
    _ROM( AUXLIB_UART, luaopen_uart, uart_map ) \
    _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map ) \
    _ROM( AUXLIB_RTOS, luaopen_rtos, rtos_map ) \
	_ROM( AUXLIB_QRENCODE, luaopen_qr_encode, qr_encode_map ) \
    _ROM( AUXLIB_TTSPLYCORE, luaopen_ttsplycore, ttsplycore_map) \
	_ROM( AUXLIB_DISP, luaopen_disp, disp_map ) \
	_ROM( AUXLIB_PMD, luaopen_pmd, pmd_map ) \
    _ROM( AUXLIB_ADC, luaopen_adc, adc_map ) \
    _ROM( AUXLIB_AUDIOCORE, luaopen_audiocore, audiocore_map ) \
	_ROM( AUXLIB_WATCHDOG, luaopen_watchdog, watchdog_map ) \
	_ROM( AUXLIB_ICONV, luaopen_iconv, iconv_map ) \
	_ROM( AUXLIB_JSON, luaopen_cjson, json_map ) \
	_ROM( AUXLIB_CPU, luaopen_cpu, cpu_map) \
	_ROM( AUXLIB_TCPIPSOCK, luaopen_tcpipsock, tcpipsock_map) \
	_ROM( AUXLIB_GPSCORE, luaopen_gpscore, gpscore_map )

// Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

#define CPU_FREQUENCY         (260 * 1000 * 1000)

// Interrupt list
#define INT_GPIO_POSEDGE      ELUA_INT_FIRST_ID
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 1 )
#define INT_ELUA_LAST         INT_GPIO_NEGEDGE
    
#define PLATFORM_CPU_CONSTANTS \
     _C( INT_GPIO_POSEDGE ),\
     _C( INT_GPIO_NEGEDGE )

#endif //__PLATFORM_CONF_H__