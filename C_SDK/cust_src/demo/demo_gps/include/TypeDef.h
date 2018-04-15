/******************************************************************************
*[File]            TypeDef.h
*[Description]
*    
*[Copyright]
*    
******************************************************************************/

#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "iot_os.h"

#ifndef true
#define true                    1
#endif

#ifndef false
#define false                   0
#endif


typedef unsigned char           GBOOL;
typedef char                    GCHAR;
typedef unsigned char           GUCHAR;
typedef short                   GINT16;
typedef unsigned short          GUINT16;
typedef int                     GINT32;
typedef unsigned int            GUINT32;
typedef void                    GVOID;
typedef double                  GDOUBLE;
typedef float                   GFLOAT;
typedef unsigned char  		    Byte;  /* 8 bits */
typedef unsigned short       	Word;


#ifdef __cplusplus
 }
#endif

#endif /* __TYPEDEF_H__ */
