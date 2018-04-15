#include "string.h"
#include "iot_debug.h"
#include "iot_flash.h"

#define flash_print iot_debug_print

static UINT32 s_demo_flash_begain_addr, s_demo_flash_end_addr;

#define DEMO_FLASH_BEGIN_ADDR (s_demo_flash_begain_addr)
#define DEMO_FLASH_END_ADDR (s_demo_flash_end_addr)

VOID demo_flash_read(VOID)
{
    UINT8 read_buff[64];
    INT32 read_len;
    E_AMOPENAT_MEMD_ERR errCode;
    
    errCode = iot_flash_read(DEMO_FLASH_BEGIN_ADDR, sizeof(read_buff), &read_len, read_buff);

    if (OPENAT_MEMD_ERR_NO != errCode)
        return;

    flash_print("[flash] read_len %x, read_buff %s", read_len, read_buff);
}

VOID demo_flash_write(VOID)
{
    UINT8 write_buff[64] = {0};
    INT32 write_len ;
    E_AMOPENAT_MEMD_ERR errCode;
    
    memcpy(write_buff, "flash hello world", strlen("flash hello world"));
    errCode = iot_flash_write(DEMO_FLASH_BEGIN_ADDR, sizeof(write_buff), &write_len, write_buff);

    if (OPENAT_MEMD_ERR_NO != errCode)
        return;
    flash_print("[flash] write_len %x, write_buff %s", write_len, write_buff);
}

VOID demo_flash_erase(VOID)
{
    E_AMOPENAT_MEMD_ERR errCode;
    flash_print("[flash] erase {%x,%x}", 
        DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR);
    errCode = iot_flash_erase(DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR);

    if (OPENAT_MEMD_ERR_NO != errCode)
    {
        flash_print("[flash] erase {%x,%x} error %d", 
        DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR, errCode);
    }
}


VOID demo_flash(VOID)
{
    demo_flash_erase(); // 擦flash
    demo_flash_write(); // 写flash
    demo_flash_read();  // 读flash
}

VOID demo_flash_init(VOID)
{
    //iot_flash_getaddr(&s_demo_flash_begain_addr, NULL);
    s_demo_flash_begain_addr = 0x882f3000;
    s_demo_flash_end_addr = s_demo_flash_begain_addr + 0x1000;
    demo_flash();

    s_demo_flash_begain_addr = 0x882f4000;
    s_demo_flash_end_addr = s_demo_flash_begain_addr + 0x1000;
    demo_flash();
}

VOID app_main(VOID)
{
	//开机立刻使用flash，会看不到打印信息
	iot_os_sleep(400);
    flash_print("[flash] app_main");

    demo_flash_init();
}
