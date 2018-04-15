#ifndef __FTP_PAL_H__
#define __FTP_PAL_H__

#include "iot_os.h"
#include "iot_sys.h"
#include "iot_debug.h"

#undef stdout
#undef stderr
#undef stdin
#undef printf
#undef exit
#undef malloc
#undef free
#undef strdup
#undef sscanf

#define stdout	11
#define stderr	12

#define malloc iot_os_malloc


#define free iot_os_free


#define perror(str) do{iot_debug_print("[ftp][perror] %s",  str);}while(0)

#define fprintf(l, arg, ...) do{ if(l == stdout) { \
                                  iot_debug_print("[ftp][info]"arg, ##__VA_ARGS__); \
                                }else{ \
                                  iot_debug_print("[ftp][error]"arg, ##__VA_ARGS__); \
                                } \
                             }while(0)
#define printf(arg, ...) do{ \
                            iot_debug_print("[ftp][info]"arg, ##__VA_ARGS__); \
                         }while(0)

#define exit(n) do{iot_os_sleep(1000); iot_debug_assert(0, (CHAR*)__func__, __LINE__);}while(0)


extern char *mystrdup(const char *src);
#define strtows iot_strtows
#define strdup mystrdup

#define sscanf IVTBL(sscanf)



#endif
