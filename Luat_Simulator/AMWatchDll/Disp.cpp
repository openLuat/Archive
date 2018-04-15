/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    disp.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Register all functions of 'Disp' into LUA's stack.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "lua.hpp"
#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "disp.h"


extern lua_State* gL;
static BOOL putimage_assert_fail = FALSE;

int checkFiledInt(lua_State *L, int index, const char *key)
{
	int d;
	lua_getfield(L, index, key);
	d = luaL_checkinteger(L, -1);
	lua_remove(L, -1);
	return d;
}

int getFiledInt(lua_State *L, int index, const char *key)
{
	int d;
	lua_getfield(L, index, key);
	d = lua_tointeger(L, -1);
	lua_remove(L, -1);
	return d;
}

int optFiledInt(lua_State *L, int index, const char *key, int defval)
{
	int d;
	lua_getfield(L, index, key);
	d = luaL_optint(L, -1, defval);
	lua_remove(L, -1);
	return d;
}

int disp_getver(lua_State* L)
{
	char ver[] = "AMWatchDll V1.0.1";

	lua_pushstring(L, ver);

	return 1;
}

int disp_init(lua_State* L)
{
	PlatformDispInitParam param;
	int cmdTableIndex;

	luaL_checktype(L, 1, LUA_TTABLE);

	memset(&param, 0, sizeof(param));

	param.width = getFiledInt(L, 1, "width");
	param.height = getFiledInt(L, 1, "height");

	if(param.width == 0 || param.height == 0)
	{
		return luaL_error(L, "disp_init: error param width(%d) height(%d)", 
			param.width, param.height);
	}

	#if defined(_WIN32)
		param.bpp = 24;
	#else
		param.bpp = getFiledInt(L, 1, "bpp");
	#endif

	LogWriter::LOGX("[disp_init] width=%d,height=%d,bpp=%d.",
		param.width,param.height,param.bpp);

	if(!(param.bpp == 16 || param.bpp == 1 || param.bpp == 24))
	{
		return luaL_error(L, "disp_init: pixel depth must be 16 or 1!%d", param.bpp); 
	}

	// lcd传输接口
	param.bus = (PlatformLcdBus)getFiledInt(L, 1, "bus");

	// 不同传输接口定义
	if(param.bus == PLATFORM_LCD_BUS_I2C || param.bus == PLATFORM_LCD_BUS_SPI){
		lua_getfield(L, 1, "interface");
		luaL_checktype(L, -1, LUA_TTABLE);

		if(param.bus == PLATFORM_LCD_BUS_I2C){
			param.lcd_itf.bus_i2c.bus_id = checkFiledInt(L, -1, "bus_id");
			param.lcd_itf.bus_i2c.freq = checkFiledInt(L, -1, "freq");
			param.lcd_itf.bus_i2c.slave_addr = checkFiledInt(L, -1, "slave_addr");
			param.lcd_itf.bus_i2c.cmd_addr = checkFiledInt(L, -1, "cmd_addr");
			param.lcd_itf.bus_i2c.data_addr = checkFiledInt(L, -1, "data_addr");
		} else if(param.bus == PLATFORM_LCD_BUS_SPI){
			param.lcd_itf.bus_spi.bus_id = checkFiledInt(L, -1, "bus_id");
			param.lcd_itf.bus_spi.pin_rs = checkFiledInt(L, -1, "pin_rs");
			param.lcd_itf.bus_spi.pin_cs = optFiledInt(L, -1, "pin_cs", PLATFORM_IO_UNKNOWN_PIN);
			param.lcd_itf.bus_spi.freq = checkFiledInt(L, -1, "freq");
		}
	}

	// lcd rst脚必须定义
	param.pin_rst = checkFiledInt(L, 1, "pinrst");

	lua_getfield(L, 1, "pincs");

	if(lua_type(L,-1) != LUA_TNUMBER)
		param.pin_cs = PLATFORM_IO_UNKNOWN_PIN;
	else
		param.pin_cs = (INT)lua_tonumber(L,-1);

	// 不设偏移则默认0
	param.x_offset = getFiledInt(L, 1, "xoffset");
	param.y_offset = getFiledInt(L, 1, "yoffset");

	param.hwfillcolor = optFiledInt(L, 1, "hwfillcolor", -1);

	// .initcmd 初始化指令表
	lua_getfield(L, 1, "initcmd");
	luaL_checktype(L, -1, LUA_TTABLE);
	param.tableSize = luaL_getn(L, -1);
	param.pLcdCmdTable = (UINT32*)WinUtil::L_MALLOC(sizeof(int)*param.tableSize);

	for(cmdTableIndex = 0; cmdTableIndex < param.tableSize; cmdTableIndex++)
	{
		lua_rawgeti(L, -1, cmdTableIndex+1);
		param.pLcdCmdTable[cmdTableIndex] = lua_tointeger(L, -1);
		lua_remove(L,-1);
	}

	lua_getfield(L, 1, "sleepcmd");
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		param.sleepCmdSize = luaL_getn(L, -1);
		param.pLcdSleepCmd = (UINT32*)WinUtil::L_MALLOC(sizeof(int)*param.sleepCmdSize);

		for(cmdTableIndex = 0; cmdTableIndex < param.sleepCmdSize; cmdTableIndex++)
		{
			lua_rawgeti(L, -1, cmdTableIndex+1);
			param.pLcdSleepCmd[cmdTableIndex] = lua_tointeger(L, -1);
			lua_remove(L,-1);
		}
	}

	lua_getfield(L, 1, "wakecmd");
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		param.wakeCmdSize = luaL_getn(L, -1);
		param.pLcdWakeCmd = (UINT32*)WinUtil::L_MALLOC(sizeof(int)*param.wakeCmdSize);

		for(cmdTableIndex = 0; cmdTableIndex < param.wakeCmdSize; cmdTableIndex++)
		{
			lua_rawgeti(L, -1, cmdTableIndex+1);
			param.pLcdWakeCmd[cmdTableIndex] = lua_tointeger(L, -1);
			lua_remove(L,-1);
		}
	}

	Disp::GetDisp()->init(&param);

	WinUtil::L_FREE(param.pLcdCmdTable);

	if(param.pLcdSleepCmd)
		WinUtil::L_FREE(param.pLcdSleepCmd);

	if(param.pLcdWakeCmd)
		WinUtil::L_FREE(param.pLcdWakeCmd);

	return 0;
}

int disp_clear(lua_State* L)
{
	Disp::GetDisp()->clear();

	return 0;
}

int disp_setcolor(lua_State* L)
{
	int color = luaL_checkinteger(L,1);
	int ret = Disp::GetDisp()->setcolor(color);
	lua_pushinteger(L, ret);

	return 1;
}

int disp_setbkcolor(lua_State* L)
{
	int color = luaL_checkinteger(L, 1);
	int ret = Disp::GetDisp()->setbkcolor(color);
	lua_pushinteger(L, ret);

	return 1;
}

int disp_update(lua_State* L)
{
	Disp::GetDisp()->update();

	return 0;
}

int disp_putimage(lua_State *L)
{
	CHAR *filename,*root;
	CHAR fpath[512] = {0}, regPath[512] = {0};
	UINT16 x, y, left, top, right, bottom;
	INT transcolor,transtype;
	INT result;

	assert(putimage_assert_fail==FALSE);
	putimage_assert_fail = TRUE;

	filename   = (CHAR*)luaL_checkstring(L, 1);
	x     = luaL_optint(L, 2, 0);
	y     = luaL_optint(L, 3, 0);
	transcolor = luaL_optint(L, 4, -1); //Default: not transparent
	left = luaL_optint(L, 5, 0);
	top = luaL_optint(L, 6, 0);
	right = luaL_optint(L, 7, 0);
	bottom = luaL_optint(L, 8, 0);
	transtype = luaL_optint(L, 9, 1);

	lua_getglobal(L,"_LROOT");
	root = (CHAR*)lua_tostring(L,-1);
	if(filename[0] == '-')
		WinUtil::separator_strrpl(regPath, filename+2, "/", "\\");
	else
		WinUtil::separator_strrpl(regPath, filename, "/", "\\");
	strcat(fpath,root);
	strcat(fpath,"\\src");
	strcat(fpath,regPath);

	result = Disp::GetDisp()->putimage(fpath, x, y, transcolor,left,top,right,bottom,transtype);

	putimage_assert_fail = FALSE;

	return result; 
}

int disp_drawrect(lua_State* L)
{
	int startx, starty, endx, endy;
	int color;

	startx = luaL_checkinteger(L, 1);
	starty = luaL_checkinteger(L, 2);
	endx = luaL_checkinteger(L, 3);
	endy = luaL_checkinteger(L, 4);
	color = luaL_checkinteger(L, 5);


	Disp::GetDisp()->drawrect(startx,starty,endx,endy,color);

	return 0;
}

static int disp_playgif(lua_State *L) {
	MSG msg;
	const char *filename, *root;
	CHAR fpath[FS_PATH_LEN] = {0}, regPath[FS_PATH_LEN] = {0};
	UINT16 x, y, times;

	assert(putimage_assert_fail==FALSE);
	putimage_assert_fail = TRUE;

	filename   = luaL_checkstring(L, 1);
	x     = luaL_optint(L, 2, 0);
	y     = luaL_optint(L, 3, 0);
	times = luaL_optint(L, 4, 1);

	lua_getglobal(L,"_LROOT");
	root = (CHAR*)lua_tostring(L,-1);
	if(filename[0] == '-')
		WinUtil::separator_strrpl(regPath, (CHAR*)(filename+2), "/", "\\");
	else
		WinUtil::separator_strrpl(regPath, (CHAR*)filename, "/", "\\");
	strcat(fpath,root);
	strcat(fpath,"\\src");
	strcat(fpath,regPath);


	PlayGifImage* playInfo = (PlayGifImage*)WinUtil::L_MALLOC(sizeof(PlayGifImage));
	memset(playInfo, 0, sizeof(playInfo));
	memcpy(playInfo->szImagePath,fpath,strlen(fpath)+1);
	playInfo->x = x;
	playInfo->y = y;
	playInfo->times = times;
	msg.message = WM_LUA_PLAY_GIF;
	msg.wParam = (WPARAM)playInfo;
	SendToLuaShellMessage(&msg);
	putimage_assert_fail = FALSE;

	return 0; 
}



int  disp_stopgif(lua_State *L) {
	MSG msg;

	msg.message = WM_LUA_STOP_GIF;
	msg.wParam = NULL;
	msg.lParam = NULL;
	SendToLuaShellMessage(&msg);

	return 0; 
}

int disp_puttext(lua_State* L)
{
	CHAR *str;
	UINT16 x, y, *offset;

	str   = (CHAR*)luaL_checkstring(L, 1);
	x     = (UINT16)luaL_checkinteger(L, 2);
	y     = (UINT16)luaL_checkinteger(L, 3);

	offset = Disp::GetDisp()->puttext(str, x, y);
	lua_pushinteger(L, offset[0]);
	lua_pushinteger(L, offset[1]);

	return 2; 
}

int disp_setfontHeight(lua_State *L)
{
	int height = luaL_checkinteger(L, 1);
	lua_pushinteger(L, Disp::GetDisp()->setfontHeight(height));
	return 1;
}

int disp_getfontHeight(lua_State *L)
{
	lua_pushinteger(L, Disp::GetDisp()->getfontHeight());
	return 1;
}

int disp_sleep(lua_State *L) {    
	int sleep = luaL_checkinteger(L,1);

	Disp::GetDisp()->platform_lcd_powersave(sleep);

	return 0; 
}

int disp_createuserlayer(lua_State *L) {
	int layer_id;
	int layer_width;
	int layer_height;
	int start_x;
	int start_y;

	layer_id     = luaL_optint(L, 1, 0);
	start_x      = luaL_optint(L, 2, 0);
	start_y      = luaL_optint(L, 3, 0);
	layer_width  = luaL_optint(L, 4, 0);
	layer_height = luaL_optint(L, 5, 0);

	//Disp::GetDisp()->platform_create_user_layer(layer_id, start_x, start_y, layer_width, layer_height); 

	return 0;
}

int disp_destroyuserlayer(lua_State *L) {  
	int layer_id = luaL_optint(L, 1, 0);

	//Disp::GetDisp()->platform_destroy_user_layer(layer_id); 

	return 0;
}

int disp_setactivelayer(lua_State *L) {  
	int layer_id = luaL_optint(L, 1, 0);

	//Disp::GetDisp()->platform_set_active_layer(layer_id); 

	return 0;
}

int disp_copy_layer(lua_State *L) {  
	int layer_id1, layer_id2, x1,y1;
	RECT rect;
	layer_id1 = luaL_optint(L, 1, -1);
	x1     = luaL_optint(L, 2, 0);
	y1     = luaL_optint(L, 3, 0);
	layer_id2 = luaL_optint(L, 4, -1);
	rect.left  = luaL_optint(L, 5, 0);
	rect.top  = luaL_optint(L, 6, 0);
	rect.right  = luaL_optint(L, 7, 0); 
	rect.bottom  = luaL_optint(L, 8, 0); 

	//Disp::GetDisp()->platform_copy_layer(layer_id1, x1, y1, layer_id2, &rect); 

	return 0; 
}

int disp_layer_display(lua_State *L) {
	int layer_id1, layer_id2, layer_id3, x1,y1,x2,y2,x3,y3;

	layer_id1 = luaL_optint(L, 1, -1);
	x1     = luaL_optint(L, 2, 0);
	y1     = luaL_optint(L, 3, 0);
	layer_id2 = luaL_optint(L, 4, -1);
	x2     = luaL_optint(L, 5, 0);
	y2     = luaL_optint(L, 6, 0);
	layer_id3 = luaL_optint(L, 7, -1);
	x3     = luaL_optint(L, 8, 0);
	y3     = luaL_optint(L, 9, 0);

	return PLATFORM_OK;
}

int disp_get_image_resolution(lua_State *L)
{
	UINT32 width, height;
	CHAR fpath[FS_PATH_LEN] = {0}, regPath[FS_PATH_LEN] = {0};
	CHAR* root;
	int result;

	const char* filename   = luaL_checkstring(L, 1);

	lua_getglobal(L,"_LROOT");
	root = (CHAR*)lua_tostring(L,-1);
	if(filename[0] == '-')
		WinUtil::separator_strrpl(regPath, (CHAR*)(filename+2), "/", "\\");
	else
		WinUtil::separator_strrpl(regPath, (CHAR*)filename, "/", "\\");
	strcat(fpath,root);
	strcat(fpath,"\\src");
	strcat(fpath,regPath);

	result = Disp::GetDisp()->platform_get_png_file_resolution(fpath,  &width, &height);

	lua_pushinteger(L, result);
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	return 3;
}

int disp_layer_start_move(lua_State *L) {

	int layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc;

	layer_id1 = luaL_optint(L, 1, 0);
	layer_id2 = luaL_optint(L, 2, 0);
	layer_id3 = luaL_optint(L, 3, 0);
	delay_ms  = luaL_optint(L, 4, 0);
	x_inc  = luaL_optint(L, 5, 0);
	y_inc = luaL_optint(L, 6, 0); 

	LogWriter::LOGX("disp_layer_start_move %d, %d, %d, %d, %d, %d", layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc);

	//Disp::GetDisp()->platform_layer_start_move(layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc);

	return 0;
}

int disp_layer_hang_start(lua_State *L) {
	int layer_id1, layer_id2, layer_id3, y_inc, delay_ms;
	int move_config, lost_dirction;

	layer_id1 = luaL_optint(L, 1, 0);
	layer_id2 = luaL_optint(L, 2, 0);
	layer_id3 = luaL_optint(L, 3, 0);
	y_inc  = luaL_optint(L, 4, 0);
	delay_ms  = luaL_optint(L, 5, 0);
	move_config = luaL_optint(L, 6, 0);
	lost_dirction = luaL_optint(L, 7, 0);

	//Disp::GetDisp()->platform_layer_hang_start(layer_id1, layer_id2, layer_id3, y_inc, delay_ms, move_config, lost_dirction);

	return 0;
}

int disp_layer_set_picture(lua_State *L) {
	int file_id, x, y, transcolor, left, right, top, bottom, transtype;
	const char *fileName;

	file_id = luaL_optint(L, 1, 0);
	fileName = luaL_checkstring(L, 2);
	x = luaL_optint(L, 3, 0);
	y  = luaL_optint(L, 4, 0);
	transcolor  = luaL_optint(L, 5, -1);
	left = luaL_optint(L, 6, 0);
	top = luaL_optint(L, 7, 0);
	right = luaL_optint(L, 8, 0);
	bottom = luaL_optint(L, 9, 0);
	transtype = luaL_optint(L, 10, 1);

	return 0;
}

int disp_layer_set_text(lua_State *L)
{
	int file_id, x, y;
	const char *textString;

	file_id = luaL_optint(L, 1, 0);
	textString = luaL_checkstring(L, 2);
	x = luaL_optint(L, 3, 0);
	y  = luaL_optint(L, 4, 0);

	//Disp::GetDisp()->openat_layer_hang_text_inset(file_id, textString, x, y);

	return 0;
}

int disp_layer_set_RQcode(lua_State *L)
{
	int file_id;
	const char *urlString;

	file_id = luaL_optint(L, 1, 0);
	urlString = luaL_checkstring(L, 2);
	//Disp::GetDisp()->openat_layer_hang_RQcode_inset(file_id, urlString);

	return 0;
}

int disp_layer_set_drawRect(lua_State *L)
{
	int file_id, x1, y1, x2, y2, color;

	file_id = luaL_optint(L, 1, 0);
	x1 = luaL_optint(L, 2, 0);
	y1 = luaL_optint(L, 3, 0);
	x2 = luaL_optint(L, 4, 0);
	y2 = luaL_optint(L, 5, 0);
	color = luaL_optint(L, 6, 0);

	//Disp::GetDisp()->openat_layer_hang_drawRect_inset(file_id, x1, y1, x2, y2, color);

	return 0;
}

int disp_layer_hang_stop(lua_State *L) {

	Disp::GetDisp()->platform_layer_hang_stop();

	return 0;
}

int disp_run(lua_State* L)
{
	LogWriter::LOGX("Disp_run1 dispRun=%d",Disp::GetDisp()->GetDispRun());

	do {
     //TO DO ...
	}while(Disp::GetDisp()->GetDispRun());

	LogWriter::LOGX("Disp_run2 dispRun=%d",Disp::GetDisp()->GetDispRun());


	return 0;
}


struct luaL_reg disp_map[] = {
	{"getver",disp_getver},
	{"init",disp_init},
	{"clear",disp_clear},
	{"setcolor",disp_setcolor},
	{"setbkcolor",disp_setbkcolor},
	{"update",disp_update},
	{"putimage",disp_putimage},
	{"playgif", disp_playgif},
	{"stopgif", disp_stopgif},
	{"drawrect",disp_drawrect},
	{"puttext",disp_puttext},
	{"setfontheight", disp_setfontHeight},
	{"getfontheight", disp_getfontHeight},
	{"sleep",disp_sleep},

	{"createuserlayer", disp_createuserlayer},
	{"destroyuserlayer", disp_destroyuserlayer},
	{"setactivelayer", disp_setactivelayer},
	{"copylayer", disp_copy_layer},
	{"layerdisplay", disp_layer_display},

	{"getimageresolution", disp_get_image_resolution},

	{"layermovestart", disp_layer_start_move},
	{"layermovehangstart", disp_layer_hang_start},
	{"layersetpicture", disp_layer_set_picture},
	{"layersetext", disp_layer_set_text},
	{"layersetqrcode", disp_layer_set_RQcode},
	{"layersetdrawrect", disp_layer_set_drawRect},
	{"layermovehangstop", disp_layer_hang_stop},
	{"run",disp_run},
	{NULL,NULL}
};

int luaopen_disp(lua_State* L)
{
	luaL_register(L,"disp",disp_map);

	MOD_REG_NUMBER(L, "BUS_SPI4LINE", PLATFORM_LCD_BUS_SPI4LINE);
	MOD_REG_NUMBER(L, "BUS_PARALLEL", PLATFORM_LCD_BUS_PARALLEL);
	MOD_REG_NUMBER(L, "BUS_I2C", PLATFORM_LCD_BUS_I2C);
	MOD_REG_NUMBER(L, "BUS_SPI", PLATFORM_LCD_BUS_SPI);

	MOD_REG_NUMBER(L, "BASE_LAYER", BASIC_LAYER_ID);
	MOD_REG_NUMBER(L, "USER_LAYER1", USER_LAYER_1_ID);
	MOD_REG_NUMBER(L, "USER_LAYER2", USER_LAYER_2_ID);
	MOD_REG_NUMBER(L, "INVALID_LAYER", INVALID_LAYER_ID);

	return 1;
}
