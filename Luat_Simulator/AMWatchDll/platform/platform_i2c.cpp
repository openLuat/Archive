/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_i2c.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Module for interfacing with the I2C interface.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "platform.h"
#include "platform_conf.h"
#include "platform_i2c.h"

static const E_AMOPENAT_I2C_PORT i2cIdMap[OPENAT_I2C_QTY] =
{
    OPENAT_I2C_1, //id = 0
    OPENAT_I2C_2, //id = 1
};

static UINT16 i2cSlaveAddr[OPENAT_I2C_QTY];

#define PLATFORM_G_SENSOR_SLI3108_ID 0x21
#define PLATFORM_G_SENSOR_SLI3108_REG 0x00
#define PLATFORM_G_SENSOR_LIS2DS12_ID 0x43
#define PLATFORM_G_SENSOR_LIS2DS12_ID_REG 0x0F
#define PLATFORM_A13_SLAVE_ADDR 0x1E
#define PLATFORM_A14_SLAVE_ADDR 0x1D
#define PLATFORM_A15_SLAVE_ADDR PLATFORM_A14_SLAVE_ADDR

void platform_i2c_gSensorParam_get(UINT8 id, UINT8 *slave_addr, UINT8 *slave_id)
{
  UINT8 pRegAddr;

  /*版本A13  型号LIS2DS12*/
  pRegAddr = PLATFORM_G_SENSOR_LIS2DS12_ID_REG;
  *slave_addr = PLATFORM_A13_SLAVE_ADDR;
  //read_i2c

  if (*slave_id == PLATFORM_G_SENSOR_LIS2DS12_ID)
  {
	LogWriter::LOGX("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }
  
  /*版本A14  型号LIS2DS12*/
  pRegAddr = PLATFORM_G_SENSOR_LIS2DS12_ID_REG;
  *slave_addr = PLATFORM_A14_SLAVE_ADDR;
  //read_i2c

  if (*slave_id == PLATFORM_G_SENSOR_LIS2DS12_ID)
  {
    LogWriter::LOGX("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }

  /*版本A15 型号SLI3108*/
  
  pRegAddr = PLATFORM_G_SENSOR_SLI3108_REG;
  *slave_addr = PLATFORM_A15_SLAVE_ADDR;
  //read_i2c

  if (*slave_id == PLATFORM_G_SENSOR_SLI3108_ID)
  {
    LogWriter::LOGX("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }

  /*版本A15 型号其他*/
  *slave_id = 0;
  *slave_addr = PLATFORM_A15_SLAVE_ADDR;
    
  LogWriter::LOGX("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
   
  return;
}

int platform_i2c_open(void)
{    
    return PLATFORM_OK;
}

int platform_i2c_close(void)
{
    return PLATFORM_OK;
}

int platform_i2c_send_data( unsigned id, UINT16 slave_addr, PCSTR pRegAddr, PCSTR buf, UINT32 len)
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = i2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return 0; //write_i2c
}

int platform_i2c_recv_data( unsigned id, UINT16 slave_addr, PCSTR pRegAddr, PSTR buf, UINT32 len)
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = i2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return 0; //read_i2c
}


