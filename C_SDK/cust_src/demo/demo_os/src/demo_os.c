#include "iot_os.h"
#include "iot_debug.h"

HANDLE demo_os_task1;
HANDLE demo_os_task2;
HANDLE demo_os_sem;

typedef enum
{
    DEMO_TASK1_MSG,
    DEMO_TASK2_MSG,
    DEMO_TIMER,
}E_DEMO_TASK_MSG_ID;

typedef struct
{
    E_DEMO_TASK_MSG_ID id;
    UINT32 len;
    VOID *param;
} T_DEMO_TASK_MSG;


VOID demo_os_msg_send(HANDLE hTask, E_DEMO_TASK_MSG_ID id, VOID *param, UINT32 len)
{
    T_DEMO_TASK_MSG *msg = NULL;

    msg = (T_DEMO_TASK_MSG *)iot_os_malloc(sizeof(T_DEMO_TASK_MSG));
    msg->id = id;
    msg->param = param;
    msg->len = len;

    iot_os_send_message(hTask, msg);
}

static VOID demo_os_task2_main(PVOID pParameter)
{
    T_DEMO_TASK_MSG *msg = NULL;
    
    iot_debug_print("[os] task2");

    iot_os_wait_message(demo_os_task2, (PVOID*)&msg);

    switch(msg->id)
    {
        case DEMO_TASK2_MSG:                   
            iot_debug_print("[os] task2 recv msg DEMO_TASK2_MSG");
            iot_debug_print("[os] task2 release sem");
             //6. 收到消息后释放信号量
            iot_os_release_semaphore(demo_os_sem);
            
            break;
        default:
            break;
    }

    if (msg)
    {
        if (msg->param)
        {
            iot_os_free(msg->param);
            msg->param = NULL;
        }

        iot_os_free(msg);
        msg = NULL;
    }
}

static VOID demo_os_task1_main(PVOID pParameter)
{
    T_DEMO_TASK_MSG *msg = NULL;
    
    iot_debug_print("[os] task1");

    // 4.发送消息给task2
    demo_os_msg_send(demo_os_task2, DEMO_TASK2_MSG, NULL, 0);
    iot_debug_print("[os] task1 send msg DEMO_TASK2_MSG");

    // 5. 等待task2收到消息释放信号量
    iot_debug_print("[os] task1 sem waiting");
    iot_os_wait_semaphore(demo_os_sem, 0);
    iot_debug_print("[os] task1 sem wait end");
    
    iot_os_wait_message(demo_os_task1, (PVOID*)&msg);

    switch(msg->id)
    {
        case DEMO_TASK1_MSG:
            break;
        default:
            break;
    }


    if (msg)
    {
        if (msg->param)
        {
            iot_os_free(msg->param);
            msg->param = NULL;
        }

        iot_os_free(msg);
        msg = NULL;
    }
}

VOID demo_os(VOID)
{
    //1. 创建task1 , 
    demo_os_task1 =  iot_os_create_task(demo_os_task1_main, NULL, 
        2048, 1, OPENAT_OS_CREATE_DEFAULT, "task1");

    //2. 创建task2 , 
    demo_os_task2 =  iot_os_create_task(demo_os_task2_main, NULL, 
        2048, 2, OPENAT_OS_CREATE_DEFAULT, "task2");

    //3. 创建信号量
    demo_os_sem = iot_os_create_semaphore(0);
}

VOID app_main(VOID)
{
    iot_debug_print("[os] app_main");

    demo_os();
}