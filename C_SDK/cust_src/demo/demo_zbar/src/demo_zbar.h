#ifndef __DEMO_ZBAR_H__
#define __DEMO_ZBAR_H__

#include "iot_os.h"


typedef enum
{
    ZBAR_TYPE_CAMERA_DATA,
    ZBAR_TYPE_CAMERA_DATA_END,
}ZBAR_MESSAGE_TYPE;

typedef enum
{
    ZBAR_SCANNER_STATUS_NULL,
    ZBAR_SCANNER_STATUS_IDLE,
    ZBAR_SCANNER_STATUS_START,
    ZBAR_SCANNER_STATUS_RUNNING,
    ZBAR_SCANNER_STATUS_STOP
}ZBAR_SCANNER_STATUS;

typedef struct {
    unsigned char *data;
    int dataLen;
    int width;
    int height;
    ZBAR_MESSAGE_TYPE type;
}ZBAR_MESSAGE;


typedef struct
{
    ZBAR_SCANNER_STATUS status;
}ZBAR_CONTEXT;

extern void zbar_task(void);

extern ZBAR_SCANNER_STATUS zbar_scannerStatusGet(void);
extern void zbar_send_message(ZBAR_MESSAGE*  msg);
#endif
