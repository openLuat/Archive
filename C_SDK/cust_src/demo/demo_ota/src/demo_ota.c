#include "string.h"
#include "iot_sys.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_flash.h"
#include "iot_socket.h"
#include "iot_network.h"
#include "iot_fs.h"
#include "ftplib.h"

#define DEMO_OTA_FILE_NAME "demo_timer_flash.bin"
#define DEMO_OTA_FTP_SERVER_IP "36.7.87.100"
#define DEMO_OTA_FTP_USR   "user"
#define DEMO_OTA_FTP_PWD   "123456"

#define DEMO_OTA_ASSERT(c) iot_debug_assert(c, (CHAR*)__func__, __LINE__)

#define DEMO_OTA_MSG_NETWORK_READY (0)
#define DEMO_OTA_MSG_NETWORK_LINKED (1)
typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_OTA_MESSAGE;

static HANDLE g_s_ota_task;


static BOOL demo_ota_download(const char* file)
{
    netbuf* conn = NULL;
    unsigned int fsz;
    BOOL ret = FALSE;

    //FtpCallbackOptions opt;
    FtpInit();

    do{
        if(!FtpConnect(DEMO_OTA_FTP_SERVER_IP, &conn))
        {
            iot_debug_print("[ota] connect to %s failed", DEMO_OTA_FTP_SERVER_IP);
            break;
        }

        if(!FtpLogin(DEMO_OTA_FTP_USR, DEMO_OTA_FTP_PWD, conn))
        {
            iot_debug_print("[ota] login to %s failed", DEMO_OTA_FTP_SERVER_IP);
            break;
        }

        if (!FtpSize(file, (unsigned int*)&fsz, 'I', conn))
        {
            iot_debug_print("[ota] get file %s size error", file);
            break;
        }
        
        iot_debug_print("[ota] download file %s size=%u", file, fsz);


        iot_debug_print("[ota] download %s....", file);
        if(!FtpGet(file, file, 'I',conn))
        {
            iot_debug_print("[ota] download file %s error", file);
            break;
        }
        iot_debug_print("[ota] download %s end", file);
        ret = TRUE;
    }while(0);

    if(conn)
    {
        FtpClose(conn);
    }
   return ret;
}


static UINT32 demo_ota_verify(const char* file)
{
    INT32 fd, ret;
    INT32 iRet;

    T_AMOPENAT_CUST_VTBL vtbl;


    fd = iot_fs_open_file(DEMO_OTA_FILE_NAME, SF_RDONLY);

    if(fd < 0)
    {
        iot_debug_print("[ota] open file %s from fs system failed ", DEMO_OTA_FILE_NAME);
        return 0;
    }

    ret = iot_fs_read_file(fd, (UINT8*)&vtbl, sizeof(vtbl));

    if(ret == sizeof(vtbl))
    {
        if(vtbl.magic == 0x87654321)
        {
            iot_fs_seek_file(fd, 0, SF_SEEK_SET);
            iRet = iot_fs_seek_file(fd, 0, SF_SEEK_END);
            iot_debug_print("[ota] ota file len =%d", iRet);
        }
        else
        {
            iot_debug_print("[ota] ota file invalid");
            iRet = 0;
        }
    }
    else
    {
        iot_debug_print("[ota] read file %s failed ret=%d", DEMO_OTA_FILE_NAME, ret);
        iRet = 0;
    }

    iot_fs_close_file(fd);
    return iRet;
}
static VOID demo_ota_upgrade(UINT32 appsize)
{
    if(iot_ota_newapp(DEMO_OTA_FILE_NAME))
    {
        iot_debug_print("[ota] set upgrade success");
        iot_debug_print("[ota] restart system for upgrade....");
        iot_os_sleep(1000);
        iot_os_restart();
    }
    else
    {
        iot_debug_print("[ota] set upgrade eror");
    }
}

static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_ota_task(PVOID pParameter)
{
    DEMO_OTA_MESSAGE*    msg;
    iot_debug_print("[ota] wait network ready....");
    BOOL sock = FALSE;
    UINT32 appsize;

    while(1)
    {
        iot_os_wait_message(g_s_ota_task, (PVOID)&msg);

        switch(msg->type)
        {
            case DEMO_OTA_MSG_NETWORK_READY:
                iot_debug_print("[ota] network connecting....");
                demo_network_connetck();
                break;
            case DEMO_OTA_MSG_NETWORK_LINKED:
                iot_debug_print("[ota] network connected");
                if(!sock)
                {
                    
         			if(!demo_ota_download(DEMO_OTA_FILE_NAME))
                    {
                        break;
                    }        
                    sock = TRUE;
                    appsize = demo_ota_verify(DEMO_OTA_FILE_NAME);
                    if(!appsize)
                    {
                        break;
                    }
                    demo_ota_upgrade(appsize);
                }
                break;
        }

        iot_os_free(msg);
    }
}


static void demo_otaworkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_OTA_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_OTA_MESSAGE));
    iot_debug_print("[ota] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = DEMO_OTA_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_ota_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = DEMO_OTA_MSG_NETWORK_READY;
        iot_os_send_message(g_s_ota_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

VOID app_main()
{
    iot_debug_print("[ota] demo_socket_init");

    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);

    //×¢²áÍøÂç×´Ì¬»Øµ÷º¯Êý
    iot_network_set_cb(demo_otaworkIndCallBack);

    g_s_ota_task = iot_os_create_task(demo_ota_task,
                        NULL,
                        2048,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_ota");
}

