/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_disp.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Define 'Disp' class.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __PLATFORM_DISP_H__
#define __PLATFORM_DISP_H__

#include "sim_key.h"
#include "sim_tp.h"
#include "stdint.h"
#include "gif_lib.h"
#include "platform_rtos.h"

#define PATH_LUA_DEBUG_INI "./AMLuaDebug.ini"

#define SEC_FEATURES "FeatureOptions"
#define KEY_DEVICE_TYPE "DEVICE_TYPE"
#define KEY_KEYPAD_TYPE "KEYPAD_TYPE"

#define FILE_ERROR (-1)

#ifdef WIN32
#define __attribute__(x)
#endif

#ifdef WIN32 
#pragma pack(push,pack1,1)
#endif
typedef struct _bitmap_file_header
{
	UINT16 file_type;
	UINT32 file_size;
	UINT16 reserved1;
	UINT16 reserved2;
	UINT32 bitmap_offset;
} __attribute__((packed)) bitmap_file_header ;

typedef struct _bitmap_info_header
{
	UINT32 header_size;
	UINT32 width;
	UINT32 height;
	UINT16 number_of_planes;
	UINT16 bits_per_pixel;
	UINT32 compression;
	UINT32 bitmap_size;
	UINT32 device_width;
	UINT32 device_height;
	UINT32 number_of_colors;
	UINT32 number_of_important_colors;
} __attribute__((packed)) bitmap_info_header ;
#ifdef WIN32 
#pragma pack(pop,pack1)
#endif

typedef struct DispBitmapTag
{
	UINT16 width;
	UINT16 height;
	UINT16 orgWidth; //font zoom
	UINT16 orgHeight;
	float zoomRatio;
	UINT8 bpp;
	const UINT8 *data;
}DispBitmap;

#define ASC_FONT_FILE_NAME "font_40.dat"
#define PINYIN_FONT_FILE_NAME "font_pinyin_40.dat"
#define HZ_FONT_FILE_NAME "fonthz_40.dat"
#define HZ_EXT_FONT_FILE_NAME "fonthzext_40.dat"
#define HZ_EXT_FONT_OFFSET_FILE_NAME "fonthzext_offset_40.dat"

#define HZ_FONT_WIDTH  (40)
#define HZ_FONT_HEIGHT (40)
#define ASC_FONT_WIDTH  (20)
#define ASC_FONT_HEIGHT (40)

#define FONT_SIZE(WIDTH, HEIGHT)   (((WIDTH)+7)/8*(HEIGHT))
#define HZ_FONT_SIZE FONT_SIZE(HZ_FONT_WIDTH, HZ_FONT_HEIGHT)
#define ASC_FONT_SIZE FONT_SIZE(ASC_FONT_WIDTH, ASC_FONT_HEIGHT)

#define MAX_FONTS	10
typedef struct FontInfoTag
{
	UINT8	width;
	UINT8	height;
	UINT8	size;
	UINT16	start;
	UINT16	end;
	const UINT8	*data;
}FontInfo;

typedef struct FontFileInfoTag
{
	UINT8	width;
	UINT8	height;
	UINT8	type;
	UINT16	start;
	UINT16	end;
}FontFileInfo;

typedef struct gifImage{  
	WORD logX;  
	WORD logY;  
	WORD width;  
	WORD height;  
	struct flag{  
		BYTE d:3;  
		BYTE c:1;  
		BYTE b:3;  
		BYTE a:1;  
	}Flag;  
}GifImage,*PGifImage;

typedef struct  
{
	CHAR szImagePath[FS_PATH_LEN];
	int x;
	int y;
	int times;
}PlayGifImage;

#define COLOR_WHITE_16 (0xffff)
#define COLOR_BLACK_16 (0x0000)
#define COLOR_WHITE_24 (0xffffff)
#define COLOR_BLACK_24 (0x000000)

#define COLOR_WHITE_1 (0xff)
#define COLOR_BLACK_1 (0x00)

#define BASIC_LAYER_ID  0
#define USER_LAYER_1_ID  1
#define USER_LAYER_2_ID  2
#define INVALID_LAYER_ID  -1

typedef enum PlatformLcdBusTag
{
	PLATFORM_LCD_BUS_SPI4LINE,
	PLATFORM_LCD_BUS_PARALLEL,
	PLATFORM_LCD_BUS_I2C,
	PLATFORM_LCD_BUS_SPI,
	PLATFORM_LCD_BUS_QTY,
	PLATFORM_LCD_BUS_MAX
}PlatformLcdBus;

typedef union {
	struct {
		int bus_id;
		int pin_rs;
		int pin_cs;
		int freq;
	} bus_spi;

	struct {
		int bus_id;
		int freq;
		int slave_addr;
		int cmd_addr;
		int data_addr;
	} bus_i2c;
} lcd_itf_t;

typedef struct PlatformDispInitParamTag
{
	UINT16 width;
	UINT16 height;
	UINT8  bpp; // Bits Per Pixel(1,16,24)
	UINT16 x_offset;
	UINT16 y_offset;
	UINT32 *pLcdCmdTable;
	UINT16 tableSize;
	UINT32 *pLcdSleepCmd;
	UINT16 sleepCmdSize;
	UINT32 *pLcdWakeCmd;
	UINT16 wakeCmdSize;
	PlatformLcdBus bus;
	lcd_itf_t lcd_itf;
	INT pin_rst; //reset pin
	INT hwfillcolor; //lcd filled color
	INT pin_cs; // cs pin
	UINT8 *framebuffer;
}PlatformDispInitParam;

typedef struct {
	HWND  hwnd;
	RECT  lcd_location;
	RECT  key_location[MAX_KEY_SUPPORT];
	HDC	  main_bitmap, main_bitmap_down;

	UINT	main_bitmap_width;
	UINT	main_bitmap_height;

	/* the data below are read from the INI file */
	UINT	main_lcd_height;
	UINT	main_lcd_width;
	UINT	skin_main_lcd_x;
	UINT	skin_main_lcd_y;
	UINT	window_height;
	UINT	window_width;
	/* the data above are read from the INI file */

} T_W32_SCREEN_DEVICE;

class Disp{
	INT dispRun; //only debug
	BOOL   bIsExitThread;

	BOOLEAN g_bUpdateScreen;
	UINT16 g_s_fontZoomSize;
	INT g_mainlcd_backlight_level_min;
	INT g_mainlcd_backlight_level_max;
	INT mainlcd_backlight_level;

	HINSTANCE lcd_Instance;
	HDC	lcd_hdcMem;	
	HBITMAP	lcd_hbmp;

	BYTE* gdi_w32_lcd_buffer;

	BITMAPINFO bitmap_info;

	INT	lcd_offset_x,lcd_offset_y;
	PlatformDispInitParam dispInitParam;
	T_W32_SCREEN_DEVICE	device;
	INT skin_lcd_width,skin_lcd_height;

	UINT8 curr_font_id;
	FontInfo sansHzFont16;
	FontInfo sansFont16;
	FontInfo dispFonts[MAX_FONTS];
	FontInfo *dispHzFont;
	FontInfo sansHzFontPinyin;

	COLORREF disp_bkcolor; //background color
	COLORREF disp_color;  //foreground color
	COLORREF lcd_hwfillcolor;

	UINT16 lua_lcd_height;
	UINT16 lua_lcd_width;
	UINT8 lua_lcd_bpp;

	TouchPanelEventStruct tp_event, last_tp_event;

public:
	Disp();
	Disp(HWND hwnd, HINSTANCE hInstance);
	~Disp();
	void WinAbnormalExit(HWND hwnd, PCSTR msg);
	void poweroff(void);
	void DispInit(HWND hwnd, HINSTANCE hInstance);
	void w32_screen_init(HWND hWnd, HINSTANCE hInstance);
	void w32_draw_main_bitmap(HDC hdc);
	void w32_update_screen(void);
	void w32_imitated_input(UINT16 type, POINT p);

	void init(PlatformDispInitParam *pParam);
	COLORREF setcolor(DWORD color);
	COLORREF setbkcolor(DWORD color);
	void hdcClear(void);
	void clear(void);
	void update(void);
	UINT16* puttext(CHAR *text, INT startx, INT starty);
	INT drawrect(
		INT startx, INT starty,
		INT endx, INT endy,
		DWORD pColor);
	INT putimage(CHAR *filename,
		INT startx, INT starty,	DWORD transcolor,
		INT left, INT top, INT right, INT bottom, INT transtype);
	INT platform_get_png_file_resolution(const char *filename, UINT32* width, UINT32* height);
	void playgif(const char* gif_file_name, int x, int y,  int times);
	void stopgif(void);
	void platform_lcd_powersave(INT sleep_wake);
	void put_qr_code_buff(unsigned char* buff, int width);
	void platform_layer_hang_stop(void);

	INT loadfont(const CHAR *name);
	INT setfont(INT id);
	INT setfontHeight(INT height);
	INT getfontHeight(void);
		
	static Disp* GetDisp(void);
	static INT GetDispRun(void);
	static HWND GetDispWnd(void);
	void SetDisp(Disp* disp);
	void SetDispRun(INT dispRun);

private:
	INT load_file_data(const CHAR *name, UINT8 **buf_pp);
	void lcd_init(const PlatformDispInitParam *pParam);
	void fontInit(void);
	void Disp_Initialize(void);
	CHAR* w32_GerProfilePath(void);
	CHAR* w32_GetProfileName(void);
	void w32_draw_main_bitmap_roi(HDC hdc, RECT rcROI, BOOL bDownEffect);
	INT put_bmp_file_buff(const UINT8 *bitmap_buffer, INT x, INT y, INT transcolor, INT left, INT top, INT right, INT bottom);
	INT put_png_file_buff(const char *filename, int x, int y, int transcolor, int left, int top, int right, int bottom, int transtype);
	void DumpScreen2RGB(ColorMapObject *ColorMap, GifRowType *ScreenBuffer,
		int ScreenWidth, int ScreenHeight);
	LONG ReadPngData(const CHAR *szPath, BITMAP *bitmap);
	HBITMAP GetSrcBit(HDC hDC, int BitWidth, int BitHeight);
	void RotateBmpBuf(INT nwidth, INT nheight, UINT8 * bmp, UINT8 *destbmp,INT nAngle);
	void getHzBitmap(DispBitmap *pBitmap, UINT16 charcode);
	void getFontBitmap(DispBitmap *pBitmap, UINT16 charcode);
	void getCharBitmap(DispBitmap *pBitmap, UINT16 charcode);
	void charBitmap_bpp1(const DispBitmap *pBitmap, UINT16 startx, UINT16 starty);
	void charBitbmp_bpp16(const DispBitmap *pBitmap, UINT16 startx, UINT16 starty);
	void charBitbmp_bpp24(const DispBitmap *pBitmap, UINT16 startx, UINT16 starty);
	void MTKLoadImage(HDC* pbmpImage, LPCSTR buffer, SIZE* sz);
	void ReadOneImageInformation(
		LPCSTR pcszImageName, 
		UINT* plWidth, UINT* plHeight,
		HDC* pbmpImage, HDC hdcTplt,
		LPCSTR pcszIniSection, LPCSTR pcszIniPath);
	void ReadOneKeyInformation(LPCSTR pcszKeyname, RECT* prectKey, 
		HDC hdcTplt, LPCSTR pcszIniSection, LPCSTR pcszIniPath);
	void w32_ReadKeyMapping(char in_path[], HWND hwnd);
	void w32_ReadKeyProfile(CHAR in_path[], HWND hwnd);
	void w32_dspl_init(void);
	void w32_imitated_tp_reset(void);
	void SetClientSize(HWND hWnd, INT width, INT height);
	void w32_light_black_draw(int x, int y, int w, int h, HDC hdc, HDC lcd);
	void w32_put_screen(INT x, INT y, INT w, INT h, BYTE *bmp);
private:
	static Disp* onlyWatchDisp;
};

#endif //__DISP_H__