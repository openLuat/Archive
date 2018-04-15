/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_pmd.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *          Implement 'PMD' class.
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/
#include "stdafx.h"

#include "platform.h"
#include "platform_pmd.h"

int platform_pmd_init(PlatformPmdCfg *pmdCfg)
{
#define PRINT_FIELD(fIELD) LogWriter::LOGX("[platform_pmd_init]: " #fIELD " %d", pmdCfg->fIELD)

    PRINT_FIELD(ccLevel);
    PRINT_FIELD(cvLevel);
    PRINT_FIELD(ovLevel);
    PRINT_FIELD(pvLevel);

    PRINT_FIELD(poweroffLevel);
    PRINT_FIELD(ccCurrent);
    PRINT_FIELD(fullCurrent);
    PRINT_FIELD(batdetectEnable);

    return PLATFORM_OK;
}

int platform_ldo_set(PlatformLdoId id, int level)
{
    //LogWriter::LOGX("[platform_ldo_set]: %d %d", id, level);

    return PLATFORM_OK;
}

int platform_pmd_powersave(int sleep_wake)
{
    //LogWriter::LOGX("[platform_pmd_powersave]: %d", sleep_wake);

    return PLATFORM_OK;
}

int platform_pmd_get_charger(void)
{
    static int charger = 0;

    charger = !charger;

    return 1;
}

UINT32 platform_pmd_getChargingCurrent(void)
{
	return 350; //350mA,OPENAT_pmd_getChargingCurrent()
}