#include "iot_lcd.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_touchscreen.h"
#include "iot_camera.h"
#include "iot_fs.h"
#include "iot_keypad.h"

#define LCD_WIDTH 128
#define LCD_HEIGHT 128
#define CAM_BY3A01_ID   0x01

static VOID camera_lcdMsgCallback(T_AMOPENAT_LCD_MESSAGE *pMsg);
UINT16 LcdFrameBuffer[LCD_WIDTH*LCD_HEIGHT];

u16 lcd_y_offset = 32;
u16 lcd_x_offset = 0;

UINT32 COLOR[5]={0xff80,0xfbe4,0x07e0,0xe8c4,0x001f};//蓝色0x001f   绿色0x07e0  红色0xE8C4 
								//黄色0xff80	橙色：0xfbe4
char	*savephoto_filepath = "/PHOTO.jpg";

const AMOPENAT_CAMERA_REG cameraInitReg [] =
{
	  //bf3a01 initial
	  //initial black
	  {0xfe, 0x00},
	  {0x3d, 0x00},
	  {0x30, 0x3b},
	  {0x31, 0x3b},
	  {0x34, 0x01},
	  {0x35, 0x0a},
	             
	  {0xfe, 0x01},
	  {0xe0, 0xba},//VCLK=48M//9a
	  {0xe1, 0x03},
	  {0xe2, 0x05},
	  {0xe3, 0x42},//bit[1]=1,bypass PLL
	  {0xe4, 0x22},
	  {0xe5, 0x03},//add lzq
	             
	  {0xe7, 0x05},
	             
	  //{0x51, 0x00, 0x01},//VSYNC:active low  HSYNC:active high
	  {0x50, 0x00},//SP mode
	  {0x66, 0x01},
	  {0x52, 0xf1},
	  {0x53, 0x20},//10
	  {0x5d, 0x00},
	             
	  {0xfe, 0x00},
	  {0x00, 0x4b},//bit[5]:1,mirror;bit[4]:1,flip;
	  {0x02, 0x10},//dummy pixel
	  {0x15, 0x0a},
	  {0x3c, 0x9d},//0x9a bit[3:0]:black lock 20140918
	  {0x41, 0x02},// anti blc vibration
	  {0x3e, 0x68},
	  {0x0f, 0x13},
	             
	  //initial AWB
	  {0xfe, 0x00},
	  {0xa0, 0x54},
	  {0xb0, 0x19},//blue gain
	  {0xb1, 0x2d},//red gain
	             
	  //initial AE 
	  {0xfe, 0x01},
	  {0x00, 0x08},
	  {0x0e, 0x03},
	  {0x0f, 0x30},//{0x0e,0x0f}:exposure time
	  {0x10, 0x18},//global gain
	             
	  {0xfe, 0x00},
	  {0x84, 0xa2},//denoise; bit[7:4],the larger the smaller noise
	  {0x82, 0x08},//denoise; The larger the smaller noise;
	  {0x86, 0x23},//bit[6:4]:bright edge enhancement;bit[2:0]:dark edge enhancement;
	             
	  //gamma tongtouxing hao
	  {0xfe, 0x00},
	  {0x60, 0x25},
	  {0x61, 0x2a},
	  {0x62, 0x28},
	  {0x63, 0x28},
	  {0x64, 0x20},
	  {0x65, 0x1d},
	  {0x66, 0x17},
	  {0x67, 0x15},
	  {0x68, 0x0f},
	  {0x69, 0x0e},
	  {0x6a, 0x0a},
	  {0x6b, 0x06},
	  {0x6c, 0x05},
	  {0x6d, 0x04},
	  {0x6e, 0x02},
	             
	  {0x72, 0x0c},//gamma offset
	  {0x73, 0x0c},//gamma offset
	  {0x74, 0x44},//bit[7:4] and bit[3:0]值越大，整体越绿
	             
	  /*//gamma default
	  {0xfe, 0x00},
	  {0x60, 0x38},
	  {0x61, 0x30},
	  {0x62, 0x24},
	  {0x63, 0x1f},
	  {0x64, 0x1c},
	  {0x65, 0x16},
	  {0x66, 0x12},
	  {0x67, 0x0f},
	  {0x68, 0x0d},
	  {0x69, 0x0c},
	  {0x6a, 0x0b},
	  {0x6b, 0x09},
	  {0x6c, 0x09},
	  {0x6d, 0x08},
	  {0x6e, 0x07},
	             
	  //gamma low noise
	  {0xfe, 0x00},
	  {0x60, 0x24},
	  {0x61, 0x30},
	  {0x62, 0x24},
	  {0x63, 0x1d},
	  {0x64, 0x1a},
	  {0x65, 0x14},
	  {0x66, 0x11},
	  {0x67, 0x0f},
	  {0x68, 0x0e},
	  {0x69, 0x0d},
	  {0x6a, 0x0c},
	  {0x6b, 0x0b},
	  {0x6c, 0x0a},
	  {0x6d, 0x09},
	  {0x6e, 0x09},*/
	             
	  //outdoor color
	  {0xfe, 0x00},
	  {0xc7, 0x21},
	  {0xc8, 0x19},
	  {0xc9, 0x84},
	  {0xca, 0x64},
	  {0xcb, 0x89},
	  {0xcc, 0x3f},
	  {0xcd, 0x16},
	             
	  //indoor color default
	  {0xfe, 0x00},
	  {0xc0, 0x05},
	  {0xc1, 0x07},
	  {0xc2, 0x30},
	  {0xc3, 0x28},
	  {0xc4, 0x3c},
	  {0xc5, 0x10},
	  {0xc6, 0x96},
	             
	  /*//indoor color vivid green OK
	  {0xfe, 0x00},
	  {0xc0, 0x26},
	  {0xc1, 0x1b},
	  {0xc2, 0x5e},
	  {0xc3, 0x58},
	  {0xc4, 0x7d},
	  {0xc5, 0x2b},
	  {0xc6, 0x96},  */
	             
	  /*//indoor color vivid blue OK
	  {0xfe, 0x00},
	  {0xc0, 0x39},
	  {0xc1, 0x31},
	  {0xc2, 0x76},
	  {0xc3, 0x64},
	  {0xc4, 0x57},
	  {0xc5, 0x07},
	  {0xc6, 0x96},
	  */           
	             
	  //AWB        
	  {0xfe, 0x00},
	  {0xb2, 0x01},
	  {0xb3, 0x11},//green gain
	  {0xa2, 0x11},//low limit of blue gain in indoor scene //0x18 20140918
	  {0xa3, 0x36},//high limit of blue gain in indoor scene
	  {0xa4, 0x11},//low limit of red gain in indoor scene
	  {0xa5, 0x36},//high limit of red gain in indoor scene
	  {0xa7, 0x80},//blue target
	  {0xa8, 0x7f},//red target
	  {0xa9, 0x15},
	  {0xaa, 0x10},
	  {0xab, 0x10},
	  {0xac, 0x2c},
	  {0xad, 0xf0},
	  {0xae, 0x20},//0x80->0x20 Ycbcr LIMIT
	  {0xb4, 0x18},//low limit of blue gain in outdoor scene
	  {0xb5, 0x1a},//high limit of blue gain in outdoor scene
	  {0xb6, 0x1c},//low limit of red gain in outdoor scene
	  {0xb7, 0x30},//high limit of red gain in outdoor scene
	  {0xd0, 0x4c},
	             
	  //AE         
	  {0xfe, 0x01},
	  {0x04, 0x4f},//AE target
	  {0x09, 0x0d},//bit[5:0]:max integration time step
	  {0x0a, 0x45},
	  {0x0b, 0x82},//minimum integration time
	  {0x0c, 0x2e},//50hz banding
	  {0x0d, 0x26},//60hz banding
	  {0x15, 0x42},
	  {0x17, 0xb5},
	  {0x18, 0x28},//The value should be smaller than the 50hz banding. add 20140702
	  {0x1b, 0x28},//minimum global gain
	  {0x1c, 0x50},
	  {0x1d, 0x39},
	  {0x1e, 0x5d},
	  {0x1f, 0x77},//max global gain
	             
	  {0xfe, 0x00},
	  {0xce, 0x48},//Contrast
	             
	  //saturation
	  {0xfe, 0x01},
	  {0x64, 0xc0},//blue saturation
	  {0x65, 0xb0},//red saturation
	             
	  {0xfe, 0x01}, 
	  {0x59, 0x00},//bit[3:0]: skip frame counter 0, don't skip frame,else skip as many as FRAME_CNT_REG frames
	  //delay 400ms 
	  {0xfe, 0x00},
	  {0xfe, 0x00},
	  {0x3d, 0xff},
	  {0xa0, 0x55},
	  {0xfe, 0x01},
	  {0x00, 0x05},	
};
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

VOID lcd_color_init(VOID)
{
	T_AMOPENAT_COLOR_LCD_PARAM param;
	BOOL ret;
	param.width = LCD_WIDTH;
	param.height = LCD_HEIGHT;
	param.msgCallback = camera_lcdMsgCallback;
	param.bus = OPENAT_LCD_SPI4LINE; // 设置lcd接口为spi接口
	param.lcdItf.spi.frequence = 13000000;
	param.lcdItf.spi.csPort = OPENAT_GPIO_UNKNOWN; //无自定义cs脚则设为unknown
	param.lcdItf.spi.rstPort = OPENAT_GPIO_14;
	ret = iot_lcd_color_init(&param);
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

static VOID lcd_wake_cmd()
{	
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xfe);
	iot_lcd_write_cmd(0xef);
	iot_lcd_write_cmd(0x11);
	iot_os_sleep(0x78);
	iot_lcd_write_cmd(0x29);
	iot_os_sleep(0x64);
}

static VOID lcd_update_screen()
{
	T_AMOPENAT_LCD_RECT_T rect;
	rect.ltX = 0;
	rect.ltY = 0;
	rect.rbX = LCD_WIDTH-1;
	rect.rbY = LCD_HEIGHT-1;

	lcdSetWindowAddress(&rect);
	iot_lcd_update_color_screen(&rect, LcdFrameBuffer);
}

static void touchClick_display(UINT16 x,UINT16 y)
{
	UINT8 i,j;
	for ( i=0; i < LCD_WIDTH; i++)
	{
		for ( j=0; j < LCD_WIDTH; j++) 
			{					//蓝色0x001f   绿色0x07e0  红色0xE8C4
			LcdFrameBuffer[i*128+j] = 0x0000;    	       // write display color
			}
        }	
	for(i=0;i<10;i++)
	{
		for(j=0;j<10;j++)
			{
			LcdFrameBuffer[(y+i)*128+x+j] = 0xFFFF; 
			}
	}
	lcd_update_screen();
}

static void touchSliping_display()
{
	UINT8 i,j;
	static UINT8 color_choice;
	
	for ( i=0; i < LCD_WIDTH; i++) 
	{
		for ( j=0; j < LCD_WIDTH; j++) 
		{					//蓝色0x001f   绿色0x07e0  红色0xE8C4
			LcdFrameBuffer[i*128+j] = COLOR[color_choice];    	       // write display color
		}
        }											//黄色：0xff80		橙色：0xfbe4
	color_choice = color_choice+1;
	if (color_choice==3)
	{
		color_choice = 0;
    	}
	lcd_update_screen();
}

static VOID touchScreen_MsgCallback(T_AMOPENAT_TOUCHSCREEN_MESSAGE *pTouchScreenMessage)
{
	iot_debug_print("pmsg->penState%d",pTouchScreenMessage->penState);
	iot_debug_print("pmsg->x%d",pTouchScreenMessage->x );
	iot_debug_print("pmsg->y%d",pTouchScreenMessage->y);
	UINT16 X = pTouchScreenMessage->x ;
	UINT16 Y = pTouchScreenMessage->y ;
    switch(pTouchScreenMessage->penState == 1)
        {     
		case 0 : 	
			touchClick_display(X,Y);		//click
			break;
		case 1:	
			touchSliping_display(); //sliping
			break;
		case 2:			//lift
			break;
		default:
			break;
        }
}
static VOID camera_open()
{	
	T_AMOPENAT_CAM_PREVIEW_PARAM previewParam;
	BOOL open_flag,on_flag;
	previewParam.startX = 0;
	previewParam.startY = 0;
	previewParam.endX = 127;
	previewParam.endY = 127;
	previewParam.offsetX = 0;
	previewParam.offsetY = 0;
	previewParam.recordAudio = FALSE;		
	
	on_flag = iot_camera_poweron(FALSE);
	iot_debug_print("camera init%d,%d",on_flag);
	open_flag = iot_camera_preview_open(&previewParam);
	iot_debug_print("camera open%d",open_flag);	
}
static VOID camera_init()
{
	T_AMOPENAT_CAMERA_PARAM initParam =
   	 {
        NULL,
        OPENAT_I2C_2, 
        0x6e,
        AMOPENAT_CAMERA_REG_ADDR_8BITS|AMOPENAT_CAMERA_REG_DATA_8BITS,
        
        TRUE,
        FALSE,
        FALSE, 
        
        240,
        320,
        
        CAMERA_IMAGE_FORMAT_YUV422,
        cameraInitReg,
        sizeof(cameraInitReg)/sizeof(AMOPENAT_CAMERA_REG),
        {0xfd, CAM_BY3A01_ID}
  	  };
	iot_camera_init(&initParam);
}

static VOID camera_off()
{
	BOOL offflag,viewoffflag;
	offflag = iot_camera_poweroff();	
	viewoffflag = iot_camera_preview_close();
	iot_debug_print("camera off%d,%d",offflag,viewoffflag);
}

static VOID camera_lcdMsgCallback(T_AMOPENAT_LCD_MESSAGE *pMsg)
{
	if(OPENAT_DRV_EVT_LCD_REFRESH_REQ == pMsg->evtId)
	 {
		T_AMOPENAT_LCD_REFRESH_REQ *pRefreshReq = &pMsg->param.refreshReq;
		lcdSetWindowAddress(&pRefreshReq->rect);
		iot_lcd_update_color_screen(&pRefreshReq->rect, pRefreshReq->pFrameBuffer);
	}
}

static VOID camera_capture()
{
	BOOL ret;
	T_AMOPENAT_CAM_CAPTURE_PARAM cameraparam;
	cameraparam.imageWidth = 128;
	cameraparam.imageHeight = 128;
	ret = iot_camera_capture(&cameraparam);
	iot_debug_print("camera_capture %d",ret);
}

static VOID camera_savephoto(char *filename)
{
	INT32 fb;
	BOOL ret;
	iot_fs_delete_file(filename);
	fb = iot_fs_create_file(filename);
	ret = iot_camera_save_photo(fb);
	iot_debug_print("camera savephoto %d,%d",ret,fb);
	iot_fs_close_file(fb);
}

static void lcd_display(UINT32 color)
{   
	UINT16 i,j;
	for ( i=0; i < 64; i++) 
	{
        for ( j=0; j < LCD_WIDTH; j++) 
		{					//蓝色0x001f   绿色0x07e0  红色0xE8C4
		LcdFrameBuffer[i*128+j] = 0xE8C4;
		LcdFrameBuffer[(i+64)*128+j] = color;
		}     	       // write display color
    	}
	lcd_update_screen();
}

static void lcd_display_image()
{
    INT32 ret;
    T_AMOPENAT_IMAGE_INFO imageinfo;
    ret = iot_decode_jpeg(savephoto_filepath,&imageinfo);
    iot_debug_print("decode_jpeg %d, %d, %d ,%d",
                    imageinfo.width,imageinfo.height,imageinfo.format,ret);
    T_AMOPENAT_LCD_RECT_T rect;
	rect.ltX = 0;
	rect.ltY = 0;
	rect.rbX = LCD_WIDTH-1;
	rect.rbY = LCD_HEIGHT-1;
	lcdSetWindowAddress(&rect);
	iot_lcd_update_color_screen(&rect, imageinfo.buffer);
    
    ret = iot_free_jpeg_decodedata(imageinfo.buffer);
    iot_debug_print("free_decode_jpeg %d",ret);
}

VOID keypad_MsgCalllback(T_AMOPENAT_KEYPAD_MESSAGE *keypad_msg)
{
	static UINT mode = 0;
	UINT16 keycode = (keypad_msg->data.matrix.r) & (keypad_msg->data.matrix.c);
	if(keypad_msg->bPressed)
	{
	mode++;
	if (mode == 9) {mode = 1;}
	iot_debug_print("ANJIAN %x,%d",keycode,mode);
	switch(mode)
		{
		case 1:	
			touchSliping_display();iot_touchScreen_sleep(FALSE);
			break;
		case 2:	
			iot_touchScreen_sleep(TRUE);
			break;
		case 3:	
			lcd_display(COLOR[2]);
			break;
		case 4:	
			camera_open();
			break;
		case 5:	
			camera_capture();
			break;
		case 6:	
			camera_savephoto(savephoto_filepath);
			break;
		case 7:	
			camera_off();lcd_display(COLOR[1]);
			break;
		case 8:
            lcd_display_image();
			break;
		default:
			break;
		}
     }
}

static void lcd_init()
{	
	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_KP_LEDR,4);
	
	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_LCD,6);
	
	lcd_color_init();

	lcd_init_cmd();

	lcd_wake_cmd();	
}

static void keypad_init()
{	
	T_AMOPENAT_KEYPAD_CONFIG KEYPAD;
	KEYPAD.type = OPENAT_KEYPAD_TYPE_MATRIX;
	KEYPAD.pKeypadMessageCallback = keypad_MsgCalllback;
	iot_keypad_init(&KEYPAD);
}

VOID app_main(VOID)
{ 	

	lcd_init();

	keypad_init();

	iot_init_touchScreen(touchScreen_MsgCallback);

	camera_init();

	lcd_display(COLOR[4]);	//开机显示
	
}

