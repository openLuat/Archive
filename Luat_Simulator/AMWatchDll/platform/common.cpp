/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    common.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *     PIO functions
 *
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/


#include "stdafx.h" 
#include "platform.h"
#include "platform_conf.h"


int platform_pio_has_port( unsigned port )
{
  return port < NUM_PIO;
}

const char* platform_pio_get_prefix( unsigned port )
{
  static char c[ 3 ];
  
  sprintf( c, "P%c", ( char )( port + PIO_PREFIX ) );
  return c;
}

int platform_pio_has_pin( unsigned port, unsigned pin )
{
#if defined( PIO_PINS_PER_PORT )
  return port < NUM_PIO && pin < PIO_PINS_PER_PORT;
#elif defined( PIO_PIN_ARRAY )
  const UINT8 pio_port_pins[] = PIO_PIN_ARRAY;
  return port < NUM_PIO && pin < pio_port_pins[ port ];
#else
  #error "You must define either PIO_PINS_PER_PORT of PIO_PIN_ARRAY in platform_conf.h"
#endif
}

// ****************************************************************************
// CPU functions

UINT32 platform_cpu_get_frequency()
{
  return CPU_FREQUENCY;
}


// ****************************************************************************

// ****************************************************************************
// Misc support

unsigned int intlog2( unsigned int v )
{
	unsigned r = 0;

	while (v >>= 1)
	{
		r++;
	}
	return r;
}