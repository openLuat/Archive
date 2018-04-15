/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    qr_encode.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          QRCOD.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "lua.hpp"
#include "auxmods.h"
#include "platform.h"
#include "platform_conf.h"
#include "qrencode.h"
#include "Disp.h"

extern QRcode* qrencode(const unsigned char *intext, int length);

static int qr_encode_show(lua_State *L) {
    size_t  len      = 0;
    
    char* url_string = (char*)luaL_checklstring(L, 1, &len);

    QRcode* code;

    code  = qrencode((const unsigned char*)url_string, len);

	Disp::GetDisp()->put_qr_code_buff((unsigned char*)code->data, code->width);

    qrencode_free(code);
    
    return 0; 
}

// Module function map
#define MIN_OPT_LEVEL   2
const luaL_Reg qr_encode_map[] =
{ 
  {"show",  qr_encode_show},
  {NULL, NULL}
};

LUALIB_API int luaopen_qr_encode( lua_State *L )
{
    luaL_register( L, AUXLIB_QRENCODE, qr_encode_map );

    return 1;
}  

