//---------------------------------------------------------------------------

#ifndef __COMMUTIL_H__
#define __COMMUTIL_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h> 
#include "TypeDef.h"


GBOOL IsHex(const GCHAR* szStr);
GBOOL IsInteger(GCHAR* szStr);
GBOOL IsFloat(GCHAR* szStr);

unsigned long u4HexCharToDecInt(char w);
unsigned long u4HexToDecInt(const char *szHex, short int i2Size);

GDOUBLE commUtilAtoF(const char* s);
unsigned short commUtilFormatFloat(char* buf, unsigned short buflen, const double n, unsigned char prec);



#ifdef __cplusplus
 }
#endif

#endif
