#include "iot_lcd.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_keypad.h"

#define LCD_WIDTH 128
#define LCD_HEIGHT 128

UINT16 LcdFrameBuffer[LCD_WIDTH*LCD_HEIGHT];

u16 lcd_y_offset = 32;
u16 lcd_x_offset = 0;

int displayflag = 0;
int timer_flag;

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

static void lcd_test_display( T_AMOPENAT_KEYPAD_MESSAGE *keypad_ispressed)
{
    
    UINT16 i,j;
    T_AMOPENAT_LCD_RECT_T rect;
    if( displayflag == 1){
    for ( i=0; i < 64; i++) {
        for ( j=0; j < LCD_WIDTH; j++) {					//蓝色0x001f   绿色0x07e0  红色0xE8C4
            LcdFrameBuffer[i*128+j] = 0xE8C4;
	     LcdFrameBuffer[(i+64)*128+j] = 0x001f;}     	       // write display color
    	}
	if (keypad_ispressed->bPressed != 0){
	displayflag = 0;}
    	}
    else
    	{
    	    for ( i=0; i < 64; i++) {
      	    for ( j=0; j < LCD_WIDTH; j++) {					//蓝色0x001f   绿色0x07e0  红色0xE8C4
            LcdFrameBuffer[i*128+j] = 0x001f;
	    LcdFrameBuffer[(i+64)*128+j] = 0xE8C4;}   
    	    	}
		if (keypad_ispressed->bPressed != 0){
		displayflag = 1;}
    	}
	
    rect.ltX = 0;
    rect.ltY = 0;
    rect.rbX = LCD_WIDTH-1;
    rect.rbY = LCD_HEIGHT-1;

    lcdSetWindowAddress(&rect);
    iot_lcd_update_color_screen(&rect, LcdFrameBuffer);
//    IVTBL(start_timer)( timer_flag,5000); 			//打开定时器
}

static VOID lcd_init_cmd()
{	
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xef);
	iot_lcd_write_cmd(0xb3);
	iot_lcd_write_data(0x03);
	iot_lcd_write_cmd(0xb6);
	iot_lcd_write_data(0x01);
	iot_lcd_write_cmd(0xa3);
	iot_lcd_write_data(0x11);
	iot_lcd_write_cmd(0x35);

	iot_lcd_write_data(0x00);
	iot_lcd_write_cmd(0x44);
	iot_lcd_write_data(0x00);
	iot_lcd_write_cmd(0x21);
	iot_lcd_write_cmd(0x36);
	iot_lcd_write_data(0xc8);
	iot_lcd_write_cmd(0x3a);
	iot_lcd_write_data(0x05);
	iot_lcd_write_cmd(0xb4);
	iot_lcd_write_data(0x21);
	iot_lcd_write_cmd(0xf0);
	iot_lcd_write_data(0x2d);
	iot_lcd_write_data(0x54);

	iot_lcd_write_data(0x24);
	iot_lcd_write_data(0x61);
	iot_lcd_write_data(0xab);
	iot_lcd_write_data(0x2e);
	iot_lcd_write_data(0x2f);
	iot_lcd_write_data(0x00);
	iot_lcd_write_data(0x20);
	iot_lcd_write_data(0x10);
	iot_lcd_write_data(0x10);

	iot_lcd_write_data(0x17);
	iot_lcd_write_data(0x13);
	iot_lcd_write_data(0x0f);
	iot_lcd_write_cmd(0xf1);
	iot_lcd_write_data(0x02);
	iot_lcd_write_data(0x22);
	iot_lcd_write_data(0x25);
	iot_lcd_write_data(0x35);
	iot_lcd_write_data(0xa8);
	iot_lcd_write_data(0x08);

	iot_lcd_write_data(0x08);
	iot_lcd_write_data(0x00);
	iot_lcd_write_data(0x00);
	iot_lcd_write_data(0x09);
	iot_lcd_write_data(0x09);
	iot_lcd_write_data(0x17);
	iot_lcd_write_data(0x18);
	iot_lcd_write_data(0x0f);
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xff);
	iot_lcd_write_cmd(0x11);
	iot_os_sleep(0x78);
	iot_lcd_write_cmd(0x29);
}

static VOID lcd_wake_wr_cmd()
{	
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xef);
	iot_lcd_write_cmd(0x11);
	iot_os_sleep(0x78);
	iot_lcd_write_cmd(0x29);
	iot_os_sleep(0x64);
}

static VOID lcd_sleep_wr_cmd()
{
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xef);
	iot_lcd_write_cmd(0x28);
	iot_os_sleep(0x64);
	iot_lcd_write_cmd(0x10);
	iot_os_sleep(0x64);
}

VOID cust_color_lcd_init(VOID)
{
    T_AMOPENAT_COLOR_LCD_PARAM param;
    BOOL ret;
    param.width = LCD_WIDTH;
    param.height = LCD_HEIGHT;
    param.msgCallback = NULL;
    param.bus = OPENAT_LCD_SPI4LINE; // 设置lcd接口为spi接口
    param.lcdItf.spi.frequence = 13000000;
    param.lcdItf.spi.csPort = OPENAT_GPIO_UNKNOWN; //无自定义cs脚则设为unknown
    param.lcdItf.spi.rstPort = OPENAT_GPIO_14;
    ret = iot_lcd_color_init(&param);

}

VOID app_main(VOID)
{ 	
//	PTIMER_EXPFUNC sys_timer ; 	//定时器控制
//	sys_timer = lcd_test_display;  	//定时器控制
	T_AMOPENAT_KEYPAD_MESSAGE  keypad_ispress;
	keypad_ispress.bPressed = 0 ;
	T_AMOPENAT_KEYPAD_CONFIG KEYPAD;
	KEYPAD.type = OPENAT_KEYPAD_TYPE_MATRIX;
	KEYPAD.pKeypadMessageCallback = lcd_test_display;
	
	iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
	
	iot_pmd_exit_deepsleep();
	
	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_KP_LEDR,4);
	
	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_LCD,6);
	
//	timer_flag = IVTBL(create_timer)(sys_timer,NULL);  //定时器控制
	
	cust_color_lcd_init();

	lcd_init_cmd();

	lcd_wake_wr_cmd();

   	lcd_test_display(&keypad_ispress);

	iot_keypad_init(&KEYPAD);
}

