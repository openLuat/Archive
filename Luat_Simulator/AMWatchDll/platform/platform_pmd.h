/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_pmd.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Power Management Definition(PMD).
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_PMD_H__
#define __PLATFORM_PMD_H__

typedef enum PlatformLdoIdTag
{
    PLATFORM_LDO_KEYPAD,
    PLATFORM_LDO_LCD,

    PLATFORM_LDO_KP_LEDR,
    PLATFORM_LDO_KP_LEDG,
    PLATFORM_LDO_KP_LEDB,

    PLATFORM_LDO_VIB,

    PLATFORM_LDO_VLCD,

    PLATFORM_LDO_VASW,
    PLATFORM_LDO_VMMC,

    PLATFORM_LDO_VCAM,

    PLATFORM_LDO_SINK,

    PLATFORM_LDO_VSIM1,
    PLATFORM_LDO_VSIM2,
	PLATFORM_LDO_VMC,

    PLATFORM_LDO_QTY
}PlatformLdoId;

#define PMD_CFG_INVALID_VALUE           (0xffff)

typedef struct PlatformPmdCfgTag
{
    UINT16             ccLevel;/*恒流阶段:4.1*/
    UINT16             cvLevel;/*恒压阶段:4.2*/
    UINT16             ovLevel;/*充电限制：4.3*/
    UINT16             pvLevel;/*回充4.1*/
    UINT16             poweroffLevel;/*关机电压：3.4，仅用于计算电量百分比，实际由上层控制关机*/
    UINT16             ccCurrent;/*恒流阶段电流*/
    UINT16             fullCurrent;/*恒压充满电流：30*/
    UINT16             batdetectEnable;
}PlatformPmdCfg;

INT platform_pmd_init(PlatformPmdCfg *pmdCfg);
INT platform_ldo_set(PlatformLdoId id, INT level);

//sleep_wake: 1 sleep 0 wakeup
INT platform_pmd_powersave(INT sleep_wake);

INT platform_pmd_get_charger(void);

UINT32 platform_pmd_getChargingCurrent(void);

#endif //__PLATFORM_PMD_H__
