/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: lifei
  Description: AMOPENAT 开放平台
  Others:
  History: 
    Version： Date:       Author:   Modification:
    V0.1      2012.12.14  lifei     创建文件
*********************************************************/
//#ifdef AM_OPENAT_CRYPTO_FUNC_SUPPORT
#ifndef AM_OPENAT_CRYPTO_H
#define AM_OPENAT_CRYPTO_H

#include "am_openat_common.h"

typedef struct T_AMOPENAT_SHA256_CTX_TAG
{
    UINT32 total[2];
    UINT32 state[8];
    UINT8 buffer[64];
} AMOPENAT_SHA256_CTX;


#endif /* AM_OPENAT_CRYPTO_H */
//#endif

