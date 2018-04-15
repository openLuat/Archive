/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_i2c.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          I2C platform interface
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_I2C_H__
#define __PLATFORM_I2C_H__


// I2C speed
enum
{
    PLATFORM_I2C_SPEED_SLOW = 100000,
    PLATFORM_I2C_SPEED_FAST = 400000
};

#define I2C_NULL_SLAVE_ADDR         (0xffff)

typedef struct PlatformI2CParamTag
{
    UINT32	speed;
    UINT16	slaveAddr;    
}PlatformI2CParam;

void platform_i2c_gSensorParam_get(UINT8 id, UINT8 *slave_addr, UINT8 *slave_id);

int platform_i2c_exists( unsigned id );
int platform_i2c_setup( unsigned id, PlatformI2CParam *pParam );
int platform_i2c_send_data( unsigned id, UINT16 slave_addr, PCSTR pRegAddr, PCSTR buf, UINT32 len);
int platform_i2c_recv_data( unsigned id, UINT16 slave_addr, PCSTR pRegAddr, PSTR buf, UINT32 len);

#endif //__PLATFORM_I2C_H__
