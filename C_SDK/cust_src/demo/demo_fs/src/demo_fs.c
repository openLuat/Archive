#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_fs.h"
#include "iot_flash.h"

#define fs_print iot_debug_print
#define DEMO_FS_FILE_PATH "demo_file"

VOID demo_fs_delete(char* file)
{
    INT32 err;

    err = iot_fs_delete_file(file);

    if (err < 0)
        return;

    fs_print("[fs] delete demo_file");
}

BOOL demo_fs_create(char* file)
{
    INT32 fd;

    fd = iot_fs_open_file(file, SF_RDONLY);

    if (fd >= 0) //DEMO_FS_FILE_PATH文件存在
    {
        iot_fs_close_file(fd);
        return FALSE;
    }
    
    // 创建文件DEMO_FS_FILE_PATH
    iot_fs_create_file(file);

    fs_print("[fs] create demo_file");
    iot_fs_close_file(fd);

    return TRUE;
}

VOID demo_fs_read(char* file)
{
    INT32 fd;
    UINT8 read_buff[64] = {0};
    INT32 read_len;
    
    fd = iot_fs_open_file(file, SF_RDONLY);

    if (fd < 0)
        return;
   
    read_len = iot_fs_read_file(fd, read_buff, sizeof(read_buff));

    if (read_len < 0)
        return;
    
    fs_print("[fs] readlen %d, read_buff %s", read_len, read_buff);

    iot_fs_close_file(fd);
}

VOID demo_fs_write(char* file)
{
    INT32 fd;
    UINT8 *write_buff = "hello world";
    INT32 write_len;
    
    fd = iot_fs_open_file(file, SF_WRONLY);

    if (fd < 0)
        return;
    
    write_len = iot_fs_write_file(fd, write_buff, strlen(write_buff));

    if (write_len < 0)
        return;
    
    fs_print("[fs] write_len %d, write_buff %s", write_len, write_buff);

    iot_fs_close_file(fd);
}

VOID demo_fs_init(VOID)
{
    //文件不存, 创建成功, 写数据读数据
    if (demo_fs_create(DEMO_FS_FILE_PATH))  
    {
        demo_fs_write(DEMO_FS_FILE_PATH); // 写文件
        demo_fs_read(DEMO_FS_FILE_PATH); // 读文件
    }
    //文件存在直接读,
    else
    {
        demo_fs_read(DEMO_FS_FILE_PATH); // 读文件
    }
}

VOID app_main(VOID)
{
	//开机立刻使用文件系统，会看不到打印信息
	iot_os_sleep(400);
    fs_print("[fs] app_main");

    demo_fs_init();
}
