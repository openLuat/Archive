/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Define some APIs and constants for AMWatch-Platform.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define _C( x ) { #x, x }

typedef INT msg_type;

// Error / status codes
enum {
	PLATFORM_ERR = (0),
	PLATFORM_OK = (1),
	PLATFORM_UNDERFLOW = (-1)
};

#define WM_USER_OFFSET (76)

#define MAX_APN_LEN             100
#define MAX_PDP_ADDR_LEN        16

#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define CUSTOM_DTCNT_PROF_MAX_USER_LEN (MAX_GPRS_USER_NAME_LEN - 1)
#define CUSTOM_DTCNT_PROF_MAX_PW_LEN (MAX_GPRS_PASSWORD_LEN - 1)

#define LOCAL_PARA_HDR \
   UINT8	ref_count; \
   UINT8    lp_reserved; \
   UINT16	msg_len;


#define ELUA_INT_EMPTY_SLOT             0
#define ELUA_INT_FIRST_ID               1
#define ELUA_INT_INVALID_INTERRUPT      0xFF

typedef UINT32 pio_type;
typedef UINT32 pio_code;
#define PLATFORM_IO_PORTS                     16
#define PLATFORM_IO_PORTS_BITS                4
#define PLATFORM_IO_PINS                      32
#define PLATFORM_IO_PINS_BITS                 5
#define PLATFORM_IO_FULL_PORT_BIT             14
#define PLATFORM_IO_FULL_PORT_MASK            ( 1 << PLATFORM_IO_FULL_PORT_BIT )
#define PLATFORM_IO_ENCODE( port, pin, full ) ( ( ( port ) << PLATFORM_IO_PINS_BITS ) | ( pin ) | ( ( full ) ? PLATFORM_IO_FULL_PORT_MASK : 0 ) )
#define PLATFORM_IO_GET_PORT( code )          ( ( ( code ) >> PLATFORM_IO_PINS_BITS ) & ( ( 1 << PLATFORM_IO_PORTS_BITS ) - 1 ) )
#define PLATFORM_IO_GET_PIN( code )           ( ( code ) & ( ( 1 << PLATFORM_IO_PINS_BITS ) - 1 ) )
#define PLATFORM_IO_IS_PORT( code )           ( ( ( code ) & PLATFORM_IO_FULL_PORT_MASK ) != 0 )
#define PLATFORM_IO_ALL_PINS                  0xFFFFFFFFUL
#define PLATFORM_IO_UNKNOWN_PIN               0xFFFFFFFEUL
#define PLATFORM_IO_ENC_PORT                  1
#define PLATFORM_IO_ENC_PIN                   0

#define PLATFORM_IO_READ_IN_MASK              0
#define PLATFORM_IO_READ_OUT_MASK             1

enum
{
	// Pin operations
	PLATFORM_IO_PIN_SET,
	PLATFORM_IO_PIN_CLEAR,
	PLATFORM_IO_PIN_GET,
	PLATFORM_IO_PIN_DIR_INPUT,
	PLATFORM_IO_PIN_DIR_OUTPUT,
	PLATFORM_IO_PIN_DIR_OUTPUT1,
	PLATFORM_IO_PIN_DIR_INT, 
	PLATFORM_IO_PIN_PULLUP,
	PLATFORM_IO_PIN_PULLDOWN,
	PLATFORM_IO_PIN_NOPULL,
	PLATFORM_IO_PIN_CLOSE,
	// Port operations
	PLATFORM_IO_PORT_SET_VALUE,
	PLATFORM_IO_PORT_GET_VALUE,
	PLATFORM_IO_PORT_DIR_INPUT,
	PLATFORM_IO_PORT_DIR_OUTPUT
};

// The platform I/O functions
int platform_pio_has_port( unsigned port );
const char* platform_pio_get_prefix( unsigned port );
int platform_pio_has_pin( unsigned port, unsigned pin );
pio_type platform_pio_op( unsigned port, pio_type pinmask, int op );

// *****************************************************************************

// *****************************************************************************
// UART subsection

// There are 4 "virtual" UART ports (UART0...UART3).
#define PLATFORM_UART_TOTAL                   4

// Parity
enum
{
  PLATFORM_UART_PARITY_EVEN,
  PLATFORM_UART_PARITY_ODD,
  PLATFORM_UART_PARITY_NONE
};

// Stop bits
enum
{
  PLATFORM_UART_STOPBITS_1,
  PLATFORM_UART_STOPBITS_1_5,
  PLATFORM_UART_STOPBITS_2
};

// "Infinite timeout" constant for recv
#define PLATFORM_UART_INFINITE_TIMEOUT        (-1)

// Flow control types (this is a bit mask, one can specify PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS )
#define PLATFORM_UART_FLOW_NONE               0
#define PLATFORM_UART_FLOW_RTS                1
#define PLATFORM_UART_FLOW_CTS                2

// The platform UART functions
int platform_uart_exists( unsigned id );
UINT32 platform_uart_setup( unsigned id, UINT32 baud, int databits, int parity, int stopbits, UINT32 mode );
UINT32 platform_uart_close( unsigned id );
int platform_uart_set_buffer( unsigned id, unsigned size );
UINT32 platform_uart_send( unsigned id, UINT8 data );
UINT32 platform_s_uart_send( unsigned id, UINT8 data );
UINT32 platform_uart_send_buff( unsigned id, UINT8* buff, UINT16 len );
UINT32 platform_s_uart_send_buff( unsigned id, UINT8* buff, UINT16 len );
int platform_uart_recv( unsigned id, unsigned timer_id, INT32 timeout );
int platform_s_uart_recv( unsigned id, INT32 timeout );
int platform_uart_set_flow_control( unsigned id, int type );
int platform_s_uart_set_flow_control( unsigned id, int type );
int platform_uart_close_all(void);

// *****************************************************************************
// Timer subsection

// There are 16 "virtual" timers (TMR0...TMR15)
#define PLATFORM_TIMER_TOTAL                  16

// Data types
typedef UINT32 timer_data_type;

// Interrupt types
#define PLATFORM_TIMER_INT_ONESHOT            1
#define PLATFORM_TIMER_INT_CYCLIC             2

// Match interrupt error codes
#define PLATFORM_TIMER_INT_OK                 0
#define PLATFORM_TIMER_INT_TOO_SHORT          1
#define PLATFORM_TIMER_INT_TOO_LONG           2
#define PLATFORM_TIMER_INT_INVALID_ID         3  

// Timer operations
enum
{
  PLATFORM_TIMER_OP_START,
  PLATFORM_TIMER_OP_READ,
  PLATFORM_TIMER_OP_SET_CLOCK,
  PLATFORM_TIMER_OP_GET_CLOCK,
  PLATFORM_TIMER_OP_GET_MAX_DELAY,
  PLATFORM_TIMER_OP_GET_MIN_DELAY
};

// The platform timer functions
int platform_timer_exists( unsigned id );
void platform_timer_delay( unsigned id, UINT32 delay_us );
void platform_s_timer_delay( unsigned id, UINT32 delay_us );
UINT32 platform_timer_op( unsigned id, int op, UINT32 data );
UINT32 platform_s_timer_op( unsigned id, int op, UINT32 data );
int platform_timer_set_match_int( unsigned id, UINT32 period_us, int type );
int platform_s_timer_set_match_int( unsigned id, UINT32 period_us, int type );
UINT32 platform_timer_get_diff_us( unsigned id, timer_data_type end, timer_data_type start );


// *****************************************************************************
// CPU specific functions

#define PLATFORM_CPU_DISABLE            0
#define PLATFORM_CPU_ENABLE             1

// Interrupt functions return status
#define PLATFORM_INT_OK                 0
#define PLATFORM_INT_GENERIC_ERROR      ( -1 )
#define PLATFORM_INT_INVALID            ( -2 )
#define PLATFORM_INT_NOT_HANDLED        ( -3 )
#define PLATFORM_INT_BAD_RESNUM         ( -4 )

typedef UINT8 elua_int_id;
// The resource number is a 16-bit integer because it must be able to hold a 
// port/pin combination coded as specified in platform.h 
typedef UINT16 elua_int_resnum;

int platform_cpu_set_global_interrupts( int status );
int platform_cpu_get_global_interrupts();
//int platform_cpu_set_interrupt( elua_int_id id, elua_int_resnum resnum, int status );
//int platform_cpu_get_interrupt( elua_int_id id, elua_int_resnum resnum );
//int platform_cpu_get_interrupt_flag( elua_int_id id, elua_int_resnum resnum, int clear );
UINT32 platform_cpu_get_frequency();

// *****************************************************************************
// The platform ADC functions

enum
{
  PLATFORM_ADC_GET_MAXVAL,
  PLATFORM_ADC_SET_SMOOTHING,
  PLATFORM_ADC_SET_BLOCKING,
  PLATFORM_ADC_SET_FREERUNNING,
  PLATFORM_ADC_IS_DONE,
  PLATFORM_ADC_OP_SET_TIMER,
  PLATFORM_ADC_OP_SET_CLOCK,
};

// Functions requiring platform-specific implementation
int platform_adc_update_sequence();
int platform_adc_start_sequence();
void platform_adc_stop( unsigned id );
UINT32 platform_adc_setclock( unsigned id, UINT32 frequency);

// ADC Common Functions
int platform_adc_exists( unsigned id );
int platform_adc_check_timer_id( unsigned id, unsigned timer_id );
UINT32 platform_adc_op( unsigned id, int op, UINT32 data );
/*+\NEW\RUFEI\2015.8.27\Add adc fuction*/
int platform_adc_open(unsigned id, unsigned mode);
int platform_adc_read(unsigned id, int *adc, int *volt);
int platform_adc_close(void);
/*-\NEW\RUFEI\2015.8.27\Add adc fuction*/

/*+\NEW\RUFEI\2015.9.8\Add pwm function */
int platform_pwm_open(unsigned id);
int platform_pwm_close(unsigned id);
int platform_pwm_set(unsigned id, int freq, int duty);
/*-\NEW\RUFEI\2015.9.8\Add pwm function */

// *****************************************************************************
// rtos support
void platform_os_sleep(UINT32 ms);

/************************************************
* I2C
*************************************************/
typedef enum E_AMOPENAT_I2C_PORT_TAG
{
	OPENAT_I2C_1,
	OPENAT_I2C_2,
	OPENAT_I2C_3,
	OPENAT_I2C_4,
	OPENAT_I2C_5,
	OPENAT_I2C_6,
	OPENAT_I2C_7,
	OPENAT_I2C_QTY
}E_AMOPENAT_I2C_PORT;

// Virtual timers data
#define VTMR_FIRST_ID           ( 32 )
#define VTMR_GET_ID( x )        ( ( x ) - VTMR_FIRST_ID )
#define TIMER_IS_VIRTUAL( x )   ( ( VTMR_NUM_TIMERS > 0 ) && ( ( x ) >= VTMR_FIRST_ID ) && ( ( x ) < VTMR_NUM_TIMERS + VTMR_FIRST_ID ) )

// Functions exported by the common platform layer
void cmn_platform_init();
void cmn_virtual_timer_cb();
void cmn_int_handler( elua_int_id id, elua_int_resnum resnum );
// Timer-specific functions
int cmn_tmr_int_set_status( elua_int_resnum resnum, int status );
int cmn_tmr_int_get_status( elua_int_resnum resnum );
int cmn_tmr_int_get_flag( elua_int_resnum resnum, int clear );
void cmn_uart_setup_sermux();

// *****************************************************************************
// console
unsigned int intlog2( unsigned int v );
void platform_set_console_port( unsigned char id );
unsigned char platform_get_console_port(void);

// *****************************************************************************
// GPIO
void gpio_init(void);

// *****************************************************************************
// EVENT
void init_event(void);

#endif //__PLATFORM_H__
