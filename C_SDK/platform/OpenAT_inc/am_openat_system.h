/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: lifei
  Description: AMOPENAT 开放平台
  Others:
  History: 
    Version： Date:       Author:   Modification:
    V0.1      2012.12.14  lifei     创建文件
*********************************************************/
#ifndef AM_OPENAT_SYSTEM_H
#define AM_OPENAT_SYSTEM_H

#include "am_openat_common.h"

/****************************** SYSTEM ******************************/
#define OPENAT_CUST_TASKS_PRIORITY_BASE 201
#define OPENAT_SEMAPHORE_TIMEOUT_MIN_PERIOD 5 //5ms

/* 线程主入口函数，参数 pParameter 为 create_task 接口传入的参数 */
typedef VOID (*PTASK_MAIN)(PVOID pParameter);

typedef enum E_AMOPENAT_OS_CREATION_FLAG_TAG
{
    OPENAT_OS_CREATE_DEFAULT = 0,   /* 线程创建后，立即启动 */
    OPENAT_OS_CREATE_SUSPENDED = 1, /* 线程创建后，先挂起 */
}E_AMOPENAT_OS_CREATION_FLAG;

typedef struct T_AMOPENAT_TASK_INFO_TAG
{
    UINT16 nStackSize;
    UINT16 nPriority;
    CONST UINT8 *pName;
}T_AMOPENAT_TASK_INFO;

/*+\NEW\liweiqiang\2013.7.1\[OpenAt]增加系统主频设置接口*/
typedef enum E_AMOPENAT_SYS_FREQ_TAG
{
    OPENAT_SYS_FREQ_32K    = 32768,
    OPENAT_SYS_FREQ_13M    = 13000000,
    OPENAT_SYS_FREQ_26M    = 26000000,
    OPENAT_SYS_FREQ_39M    = 39000000,
    OPENAT_SYS_FREQ_52M    = 52000000,
    OPENAT_SYS_FREQ_78M    = 78000000,
    OPENAT_SYS_FREQ_104M   = 104000000,
    OPENAT_SYS_FREQ_156M   = 156000000,
    OPENAT_SYS_FREQ_208M   = 208000000,
    OPENAT_SYS_FREQ_250M   = 249600000,
    OPENAT_SYS_FREQ_312M   = 312000000,
}E_AMOPENAT_SYS_FREQ;
/*-\NEW\liweiqiang\2013.7.1\[OpenAt]增加系统主频设置接口*/

/****************************** TIME ******************************/
typedef struct T_AMOPENAT_SYSTEM_DATETIME_TAG
{
    UINT16 nYear;
    UINT8  nMonth;
    UINT8  nDay;
    UINT8  nHour;
    UINT8  nMin;
    UINT8  nSec;
    UINT8  DayIndex; /* 0=Sunday */
}T_AMOPENAT_SYSTEM_DATETIME;

typedef struct
{
  uint8               alarmIndex;
  bool                alarmOn; /* 1 set,0 clear*/
  uint8               alarmRecurrent; /* 1 once,bit1:Monday...bit7:Sunday */
  T_AMOPENAT_SYSTEM_DATETIME alarmTime;
}T_AMOPENAT_ALARM_PARAM;

/****************************** TIMER ******************************/
#define OPENAT_TIMER_MIN_PERIOD 5 //5ms

typedef struct T_AMOPENAT_TIMER_PARAMETER_TAG
{
    HANDLE hTimer;      /* create_timer 接口返回的 HANDLE */
    UINT32 period;      /* start_timer 接口传入的 nMillisecondes */
    PVOID  pParameter;  /* create_timer 接口传入的 pParameter */
}T_AMOPENAT_TIMER_PARAMETER;

/* 定时器到时回调函数，参数 pParameter 为栈变量指针，客户程序中不需要释放该指针 */
typedef VOID (*PTIMER_EXPFUNC)(T_AMOPENAT_TIMER_PARAMETER *pParameter);

typedef VOID (*PMINUTE_TICKFUNC)(VOID);
/**
设置软件异常情况下，设备重启还是进入调试模式，默认为OPENAT_FAULT_RESET
*/
typedef enum openatFaultModeE
{
	/*!< 异常重启，默认状态*/
	OPENAT_FAULT_RESET,
	/*!< 异常调试模式*/
	OPENAT_FAULT_HANG
}E_OPENAT_FAULT_MODE;


int OPENAT_GetEnvUsage(void);
VOID OPENAT_SetFaultMode(E_OPENAT_FAULT_MODE mode);

void OPENAT_Decode(UINT32* v, INT32 n);

#define OPENAT_NETWORK_ISP_LENGTH (64)
#define OPENAT_NETWORK_IMSI_LENGHT (64)
#define OPENAT_NETWORK_APN_LENGTH (64)
#define OPENAT_NETWORK_PASSWORD_LENGTH	(64)
#define OPENAT_NETWORK_USER_NAME_LENGTH (64)

typedef enum
{
    /*!< 网络断开 表示GPRS网络不可用澹，无法进行数据连接，有可能可以打电话*/
	OPENAT_NETWORK_DISCONNECT            		= 0x00,
    /*!< 网络已连接 表示GPRS网络可用，可以进行链路激活*/
	OPENAT_NETWORK_READY,
	/*!< 链路正在激活 */
	OPENAT_NETWORK_LINKING,
    /*!< 链路已经激活 PDP已经激活，可以通过socket接口建立数据连接*/
	OPENAT_NETWORK_LINKED,
	/*!< 链路正在去激活 */
	OPENAT_NETWORK_GOING_DOWN,
}E_OPENAT_NETWORK_STATE;

typedef struct
{
	/*!< 网络状态 */
	E_OPENAT_NETWORK_STATE state;
	/*!< 网络信号：0-31 (值越大，信号越好) */
	UINT8 csq;
	/*!< SIM卡是否存在 */
	BOOL  simpresent;
}T_OPENAT_NETWORK_STATUS;

typedef struct
{
	char apn[OPENAT_NETWORK_APN_LENGTH];
	char username[OPENAT_NETWORK_USER_NAME_LENGTH];
	char password[OPENAT_NETWORK_PASSWORD_LENGTH];
}T_OPENAT_NETWORK_CONNECT;


typedef VOID(*F_OPENAT_NETWORK_IND_CB)(E_OPENAT_NETWORK_STATE );



#endif /* AM_OPENAT_SYSTEM_H */

