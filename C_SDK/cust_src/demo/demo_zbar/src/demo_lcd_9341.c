
#include "string.h"
#include "iot_debug.h"
#include "iot_lcd.h"
#include "iot_camera.h"
#include "iot_gpio.h"
#include "iot_pmd.h"
#include "demo_lcd_9341.h"
#include "demo_zbar.h"


#ifdef LCD_9341
#define LCD_RST_PIN OPENAT_GPIO_14
#define LCD_RS_PIN OPENAT_GPIO_18
#define LCD_BL_PIN OPENAT_GPIO_2
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
u16 lcd_y_offset = 0;
u16 lcd_x_offset = 0;
unsigned char g_scanner_buff[CAMERA_PREVIEW_WITHE*CAMERA_PREVIEW_HEIGHT];
#define ZBAR_IMAGE_START_X 0
#define ZBAR_IMAGE_START_Y 0
#define ZBAR_IMAGE_END_X CAMERA_PREVIEW_WITHE
#define ZBAR_IMAGE_END_Y CAMERA_PREVIEW_HEIGHT

const int initcmd[] =
{
	0x11,
	0x00010078,
	0xCF,
	0x00030000,
	0x00030099,
	0x00030030,
	0xED,
	0x00030064,
	0x00030003,
	0x00030012,
	0x00030081,
	0xCB,
	0x00030039,
	0x0003002C,
	0x00030000,
	0x00030034,
	0x00030002,
	0xEA,
	0x00030000,
	0x00030000,
	0xE8,
	0x00030085,
	0x00030000,
	0x00030078,
	0xC0,
	0x00030023,
	0xC1,
	0x00030012,
	0xC2,
	0x00030011,
	0xC5,
	0x00030040,
	0x00030030,
	0xC7,
	0x000300A9,
	0x3A,
	0x00030055,
	0x36,
	0x00030008,
	0xB1,
	0x00030000,
	0x00030018,
	0xB6,
	0x0003000A,
	0x000300A2,
	0xF2,
	0x00030000,
	0xF7,
	0x00030020,
	0x26,
	0x00030001,
	0xE0,
	0x0003001F,
	0x00030024,
	0x00030023,
	0x0003000B,
	0x0003000F,
	0x00030008,
	0x00030050,
	0x000300D8,
	0x0003003B,
	0x00030008,
	0x0003000A,
	0x00030000,
	0x00030000,
	0x00030000,
	0x00030000,
	0xE1,
	0x00030000,
	0x0003001B,
	0x0003001C,
	0x00030004,
	0x00030010,
	0x00030007,
	0x0003002F,
	0x00030027,
	0x00030044,
	0x00030007,
	0x00030015,
	0x0003000F,
	0x0003003F,
	0x0003003F,
	0x0003001F,			
	0x29,
};

static void write_command_table(const int *table, int size)
{
    UINT16 flag;
    UINT16 value;
    UINT16 index;

    for(index = 0; index < size && table[index] != (UINT32)-1; index++)
    {
        flag = table[index]>>16;
        value = table[index]&0xffff;

        switch(flag)
        {
            case 1:
                iot_os_sleep(value);
                break;
            case 0:
            case 2:
                iot_lcd_write_cmd(value&0x00ff);
                break;

            case 3:
                iot_lcd_write_data(value&0x00ff);
                break;

            default:
                break;
        }
    }
}

static void lcd_reg_init(void)
{
    write_command_table(initcmd, sizeof(initcmd)/sizeof(int));
}

void lcdSetWindowAddress(T_AMOPENAT_LCD_RECT_T *pRect)
{
    u16 ltx = pRect->ltX + lcd_x_offset;
    u16 lty = pRect->ltY + lcd_y_offset;
    u16 rbx = pRect->rbX + lcd_x_offset;
    u16 rby = pRect->rbY + lcd_y_offset;
	
    iot_lcd_write_cmd(0x2a);    //Set Column Address
    iot_lcd_write_data(ltx>>8);
    iot_lcd_write_data(ltx&0x00ff);        
    iot_lcd_write_data(rbx>>8);
    iot_lcd_write_data(rbx&0x00ff);
    
    iot_lcd_write_cmd(0x2b);    //Set Page Address
    iot_lcd_write_data(lty>>8);
    iot_lcd_write_data(lty&0x00ff);                
    iot_lcd_write_data(rby>>8);
    iot_lcd_write_data(rby&0x00ff);
    
    // Write the display data into GRAM here 
    iot_lcd_write_cmd(0x2C); //GRAM start writing 
    
}

void lcdZbarMsgSend(unsigned char *data)
{
	int i,j;
	unsigned char *src = data;
	unsigned char *dst = g_scanner_buff;
	ZBAR_MESSAGE *msg;

	if (zbar_scannerStatusGet() != ZBAR_SCANNER_STATUS_IDLE)
		return;

	msg = (ZBAR_MESSAGE *)iot_os_malloc(sizeof(ZBAR_MESSAGE));

	if (!msg)
		return;
	
	msg->type = ZBAR_TYPE_CAMERA_DATA;
	msg->height = ZBAR_IMAGE_END_Y - ZBAR_IMAGE_START_Y;
	msg->width = ZBAR_IMAGE_END_X - ZBAR_IMAGE_START_X;
	msg->dataLen = (ZBAR_IMAGE_END_Y - ZBAR_IMAGE_START_Y) * (ZBAR_IMAGE_END_X - ZBAR_IMAGE_START_X);
	msg->data = g_scanner_buff;

	// 解析黑白图片， 减少解析的一半图片
    for (j=ZBAR_IMAGE_START_Y; j<ZBAR_IMAGE_END_Y; j++)
    {
        for(i=ZBAR_IMAGE_START_X; i<ZBAR_IMAGE_END_X; i++)
		{
			dst[((j-ZBAR_IMAGE_START_Y)*CAMERA_PREVIEW_WITHE)+(i-ZBAR_IMAGE_START_X)]
			 = src[((j*CAMERA_PREVIEW_WITHE)+i)*2];
		}
	}

	zbar_send_message(msg);
}

void lcdMsgCallbacktime(void)
{
    static int tick;
    int time;

    if (tick == 0)
        tick = iot_os_get_system_tick();

    time = (iot_os_get_system_tick() - tick) * 1000 / 16384;
    tick = iot_os_get_system_tick();
    iot_debug_print("[zbar] lcdMsgCallbacktime time %d", time);
}

// update 采集数据, 并发送数据给ZBARtask解析二维码
void lcdMsgCallback(T_AMOPENAT_LCD_MESSAGE *pMsg)
{    
    switch(pMsg->evtId)
    {
        case OPENAT_DRV_EVT_LCD_REFRESH_REQ:
            {
                T_AMOPENAT_LCD_REFRESH_REQ *pRefreshReq = &pMsg->param.refreshReq;

                // 打印updata的时间间隔
                //lcdMsgCallbacktime();
                        
                // 获取camera得到的数据， 发送到zbartask 去解析
                  lcdZbarMsgSend((unsigned char *)pRefreshReq->pFrameBuffer);

                // update lcd显示camera数据
                lcdSetWindowAddress(&pRefreshReq->rect);
                iot_lcd_update_color_screen(&pRefreshReq->rect, pRefreshReq->pFrameBuffer);
            }
            break;

        default:
            break;
    }


}

void lcd_led(void)
{
    T_AMOPENAT_GPIO_CFG cfg;

    memset(&cfg, 0, sizeof(T_AMOPENAT_GPIO_CFG));
    cfg.mode = OPENAT_GPIO_OUTPUT;
    cfg.param.defaultState = 1;
    iot_gpio_config(LCD_BL_PIN, &cfg);
    lcd_reg_init();
}

void lcd_open(void)
{  
    BOOL ret;
    T_AMOPENAT_COLOR_LCD_PARAM param;
    
    iot_pmd_poweron_ldo(OPENAT_LDO_POWER_LCD,6);
    iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VLCD,6);

    //初始化LCD
    param.width = LCD_WIDTH;
    param.height = LCD_HEIGHT;
    param.msgCallback = lcdMsgCallback;
    param.bus = OPENAT_LCD_SPI4LINE; // 设置lcd接口为spi接口
    param.lcdItf.spi.frequence = 26000000;
    param.lcdItf.spi.csPort = OPENAT_GPIO_UNKNOWN; //无自定义cs脚则设为unknown
    param.lcdItf.spi.rstPort = LCD_RST_PIN;
    ret = iot_lcd_color_init(&param);
    iot_debug_print("[zbar] lcd init result %d", ret);

    // 打开背光
    lcd_led();

}
#endif

