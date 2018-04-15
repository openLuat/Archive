/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_disp.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          Implement 'Disp' class.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "assert.h"
#include "WinGDI.h"
#include "png.h"
#include "platform.h"
#include "platform_conf.h"
#include "platform_disp.h"
#include <ocidl.h>  
#include <olectl.h>
#include <math.h>

Disp* Disp::onlyWatchDisp = NULL;

const UINT8 blankChar[HZ_FONT_SIZE] = {0};

static const UINT8 sansFont16Data[]=
{
#include ASC_FONT_FILE_NAME
};

static const UINT8 sansHzFont16Data[] =
{
#include HZ_FONT_FILE_NAME
};

static const UINT8 sansHzFont16ExtData[] = 
{
#include HZ_EXT_FONT_FILE_NAME
};

static const UINT16 sansHzFont16ExtOffset[] =
{
#include HZ_EXT_FONT_OFFSET_FILE_NAME
};

static const UINT8 sansHzFontPinyiData[] =
{
#include PINYIN_FONT_FILE_NAME
};

static key_mapping_struct key_mapping_table[] =
{
	//We do this definition in LuaDebug.ini
	{0,	DEVICE_KEY_UP,	0x00},		
	{1,	DEVICE_KEY_DOWN,	0x00},		
	{2,	DEVICE_KEY_LEFT,	0x00},		
	{3,	DEVICE_KEY_RIGHT,	0x00},		
	{4,	DEVICE_KEY_OK,	0x00},		
	{0xFE,	DEVICE_KEY_NONE,	0xFE},
};

//Key Name for w32_ReadKeyMapping
static PCSTR KeyMappingName[MAX_KEY_SUPPORT] = 
{
	"DEVICE_KEY_UP",
	"DEVICE_KEY_DOWN",
	"DEVICE_KEY_LEFT",
	"DEVICE_KEY_RIGHT",
	"DEVICE_KEY_OK",
};

void Disp::RotateBmpBuf(INT nwidth, INT nheight, UINT8 * bmp, UINT8 *destbmp,INT nAngle)
{

	int nLineTailDest,nnewTail;
	int x,y;
	int nlooppixel,nNewlooppixel;

	nLineTailDest = 0;
	nnewTail  = 0;
	
	if(360 == nAngle) //360
	{
		for (y = 0;y < nheight - 1;y++)
		{
			for(x = 0;x < nwidth-1;x++)
			{
				nlooppixel = (nheight - y - 1) * (nwidth*4 + nLineTailDest) + x*4;
				nNewlooppixel = y * (nwidth*4 + nnewTail) + x*4;		
				destbmp[nNewlooppixel] = bmp[nlooppixel];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
			}

		}
	}
	else if(270 == nAngle)//270
	{
		for (y = nheight-1 ; y >=0 ; y--)
		{

			for(x = 0;x < nwidth-1;x++)
			{
				nlooppixel = y * (nwidth*4 + nLineTailDest) + x*4;
				nNewlooppixel = (x)*(nheight*4 + nnewTail) + (nheight - y /*- 1*/)*4;		
				destbmp[nNewlooppixel] = bmp[nlooppixel];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
			}

		}	
	}
	else if(90 == nAngle)
	{
		for(y = nheight-1;y >=0 ;y--)
		{		
			for (x = nwidth-1 ; x >=0 ; x--)
			{
				nlooppixel = y * (nwidth*4 + nLineTailDest) + x*4;
				nNewlooppixel = (nwidth - x)*(nheight*4 + nnewTail) + y*4;
				destbmp[nNewlooppixel] = bmp[nlooppixel];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
			}
		}	
	}
	else if(180 == nAngle)//180
	{
		for (x = nwidth-1 ; x >=0 ; x--)
		{
			for(y = nheight-1;y >=0 ;y--)
			{
				nlooppixel = y * (nwidth*4 + nLineTailDest) + x*4;
				nNewlooppixel = (nwidth - x + (nheight - y) * nwidth)*4;
				destbmp[nNewlooppixel] = bmp[nlooppixel];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
				destbmp[nNewlooppixel++] = bmp[nlooppixel++];
			}
		}	
	}
}

void Disp::getHzBitmap(DispBitmap *pBitmap, UINT16 charcode)
{
	const FontInfo *pInfo = dispHzFont;

	pBitmap->bpp = 1;
	pBitmap->width = pInfo->width;
	pBitmap->height = pInfo->height;

	if(pInfo->data)
	{
		UINT8 byte1, byte2;
		UINT32 index;

		byte1 = charcode>>8;
		byte2 = charcode&0x00ff;

		if(byte1 >= 0xB0 && byte1 <= 0xF7 &&
			byte2 >= 0xA1 && byte2 <= 0xFE)
		{
			index = (byte1 - 0xB0)*(0xFE - 0xA1 + 1) + byte2 - 0xA1;
			if(byte1 > 0xD7)
			{
				index -= 5; /*D7FA-D7FE 这5个是空的*/
			}

			pBitmap->data = pInfo->data + index*pInfo->size;
		}
		else
		{
			pBitmap->data = blankChar;

			for(index = 0; index < sizeof(sansHzFont16ExtOffset)/sizeof(UINT16); index++)
			{
				if(charcode < sansHzFont16ExtOffset[index])
				{
					break;
				}

				if(charcode == sansHzFont16ExtOffset[index])
				{
					pBitmap->data = sansHzFont16ExtData + index*pInfo->size;
					break;
				}
			}
		}
	}
	else
	{
		pBitmap->data = blankChar;
	}
}

void Disp::getFontBitmap(DispBitmap *pBitmap, UINT16 charcode)
{
	const FontInfo *pInfo = &dispFonts[curr_font_id];

	pBitmap->bpp = 1;
	pBitmap->width = pInfo->width;
	pBitmap->height = pInfo->height;

	if(pInfo->data)
	{
		if(charcode >= pInfo->start && charcode <= pInfo->end)
		{
			UINT32 index = charcode - pInfo->start;

			pBitmap->data = pInfo->data + index*pInfo->size;            
		}
		else
		{
			pBitmap->data = blankChar;
		}
	}
	else
	{
		pBitmap->data = blankChar;
	}
}

HBITMAP Disp::GetSrcBit(HDC hDC, int BitWidth, int BitHeight)
{
	HDC hBufDC;
	HBITMAP hBitmap;

	hBufDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hDC, BitWidth, BitHeight);
	SelectObject(hBufDC, hBitmap);
	BitBlt(hBufDC, 0, 0, BitWidth, BitHeight, hDC,  0, 0, SRCCOPY);
	DeleteDC(hBufDC);
	return hBitmap;

}

void Disp::getCharBitmap(DispBitmap *pBitmap, UINT16 charcode)
{
	if(charcode >= 0x80A0)
	{
		getHzBitmap(pBitmap, charcode);
	} else {
		getFontBitmap(pBitmap, charcode);
	}
	pBitmap->orgHeight = pBitmap->height;
	pBitmap->orgWidth  = pBitmap->width;
	pBitmap->zoomRatio = (FLOAT)pBitmap->height/g_s_fontZoomSize;

	pBitmap->height = g_s_fontZoomSize;
	pBitmap->width = (UINT16)((FLOAT)pBitmap->width/pBitmap->zoomRatio);
}

void Disp::charBitmap_bpp1(const DispBitmap *pBitmap,
	UINT16 startX, UINT16 startY)
{
	UINT16 bx,by,x,y,bwbytes;
	UINT16 endX, endY;
	UINT32 *buffer1 = (UINT32*)gdi_w32_lcd_buffer;
	UINT16 srcbx, srcby;

	if(pBitmap->bpp != 1)
	{
		LogWriter::LOGX("[Disp::charBitmap_bpp1]: not support bpp %d", pBitmap->bpp);
		return;
	}

	endX = MIN(lua_lcd_width,startX + pBitmap->width);
	endY = MIN(lua_lcd_height,startY + pBitmap->height);

	bwbytes = (pBitmap->orgWidth+7)/8;

	for(x = startX,bx = 0; x < endX; x++,bx++)
	{
		srcbx = pBitmap->zoomRatio*bx;
		for(y = startY,by = 0; y < endY; y++,by++)
		{
			srcby = pBitmap->zoomRatio*by;
			if((disp_color^lcd_hwfillcolor) == 0)
			{
				if(pBitmap->data[bwbytes*srcby+srcbx/8]&(0x80>>(srcbx%8)))
				{
					buffer1[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = 0x00ffffff;
				}
				else
				{
					//buffer1[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = 0x00000000;
				}
			}
			else
			{
				if(pBitmap->data[bwbytes*srcby+srcbx/8]&(0x80>>(srcbx%8)))
				{
					buffer1[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = 0x00000000;
				}
				else
				{
					//buffer1[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = 0x00ffffff;
				}
			}
		}
	}
}

void Disp::charBitbmp_bpp16(const DispBitmap *pBitmap,
	UINT16 startX, UINT16 startY)
{
	UINT16 bx,by,x,y,bwbytes;
	UINT16 endX, endY;
	UINT32 *buffer16 = (UINT32*)gdi_w32_lcd_buffer;
	UINT16 srcbx, srcby;

	assert(pBitmap->bpp == 1);

	endX = MIN(lua_lcd_width,startX + pBitmap->width);
	endY = MIN(lua_lcd_height,startY + pBitmap->height);

	bwbytes = (pBitmap->orgWidth+7)/8;

	for(x = startX,bx = 0; x < endX; x++,bx++)
	{
		srcbx = pBitmap->zoomRatio*bx;
		for(y = startY,by = 0; y < endY; y++,by++)
		{            
			srcby = pBitmap->zoomRatio*by;
			if(pBitmap->data[bwbytes*srcby+srcbx/8]&(0x80>>(srcbx%8)))
			{
				//填充颜色
				buffer16[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = disp_color;
			}
			else
			{
				//填充背景色
				//buffer16[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = disp_bkcolor;
			}
		}
	}
}

// one char is 120-bytes
void Disp::charBitbmp_bpp24(const DispBitmap *pBitmap,
	UINT16 startX, UINT16 startY)
{
	UINT16 bx,by,x,y,bwbytes;
	UINT16 endX, endY;
	UINT32 *buffer24 = (UINT32*)gdi_w32_lcd_buffer;
	UINT16 srcbx,srcby;

	assert(pBitmap->bpp == 1);

	endX = MIN(lua_lcd_width,startX + pBitmap->width);
	endY = MIN(lua_lcd_height,startY + pBitmap->height);

	bwbytes = (pBitmap->orgWidth+7)/8;


	for(x = startX,bx = 0; x < endX; x++,bx++)
	{
		srcbx = pBitmap->zoomRatio*bx;
		for(y = startY,by = 0; y < endY; y++,by++)
		{            
			srcby = pBitmap->zoomRatio*by;
			if(pBitmap->data[bwbytes*srcby+srcbx/8]&(0x80>>(srcbx%8))) // pBitmap->data[dPos]&(0x80>>(srcbx%8))
			{
				//buffer24[(y - 1) * lua_lcd_width + lua_lcd_width - x - 1] = disp_color; // ago OK !!!
				buffer24[(y - 1) * lua_lcd_width + x] = disp_color;
			}
			else
			{
				//填充背景色
				//直接叠加显示,暂不支持背景色设置
				//buffer16[y*lua_lcd_width + x] = disp_bkcolor;
			}
		}
	}
}

INT Disp::put_bmp_file_buff(const UINT8 *bitmap_buffer, INT x, INT y, INT transcolor, INT left, INT top, INT right, INT bottom)
{
	bitmap_file_header *p_fileHeader = (bitmap_file_header *)bitmap_buffer;
	bitmap_info_header *p_infoHeader = (bitmap_info_header *)(bitmap_buffer+sizeof(bitmap_file_header));
	const UINT8 *data_buf;
	UINT16 data_r,data_g,data_b;
	UINT16 width, height;
	UINT16 rowIndex,colIndex;
	UINT16 bitmapRowBytes;
	UINT16 rgb16;
	UINT16 real_width;
	INT bitmapRowIndex,bmpColIndex;       
	UINT16 data_index;

	UINT8 *buffer16 = (UINT8*)gdi_w32_lcd_buffer;
	UINT8 *buffer24 = (UINT8*)gdi_w32_lcd_buffer;
	UINT16 bmp_bpp = p_infoHeader->bits_per_pixel;

	if((left > right) || (top > bottom))
	{
		LogWriter::LOGX("put_bmp_buffer: rect error");
		return PLATFORM_ERR;
	}

	if((left == 0) && (top == 0) && (right == 0) && (bottom == 0))
	{
		width = MIN(p_infoHeader->width + x, lua_lcd_width);
		height = MIN(p_infoHeader->height + y, lua_lcd_height);  
		bottom = p_infoHeader->height - 1;
		real_width = MIN(p_infoHeader->width, lua_lcd_width); 
	}
	else
	{
		width = MIN(right - left + 1 + x, lua_lcd_width);
		height = MIN(bottom - top + 1 + y, lua_lcd_height); 
		real_width = MIN(right - left + 1, lua_lcd_width);
	}

	data_buf = bitmap_buffer + p_fileHeader->bitmap_offset;

	bitmapRowBytes = ((p_infoHeader->width*bmp_bpp + 31)&~31)>>3; //4字节对齐

	if (lua_lcd_bpp == 1 && bmp_bpp == 1)
	{
		int page;
		UINT8 lcdfill = lcd_hwfillcolor == COLOR_WHITE_1 ? 1:0;
		UINT8 bmpfill = 1;
		UINT8 finalfill;

		if (p_infoHeader->number_of_planes == 1)
		{
			const UINT8 *fill_plate_p = bitmap_buffer \
				+ sizeof(bitmap_file_header) /* 跳过文件头 */ \
				+ sizeof(bitmap_info_header) /* 跳过信息头 */ \
				+ 4 /* 跳过0值的调色板信息 */;

			if(fill_plate_p[0] == 0xff && fill_plate_p[1] == 0xff && fill_plate_p[2] == 0xff){
				// 1值调色板如果是全ff 表示白色 其他值都判定为黑色
				bmpfill = 1;
			} else {
				bmpfill = 0;
			}
		}

		// lcd的填充色与位图填充色的一致结果
		finalfill = ((lcdfill^bmpfill) == 0) ? 0x80 : 0x00;

		for(rowIndex = y, bitmapRowIndex = p_infoHeader->height - top - 1; 
			rowIndex < height && bitmapRowIndex >= p_infoHeader->height - bottom - 1;
			rowIndex++, bitmapRowIndex--)
		{
			page = rowIndex/8;

			for(colIndex = x, bmpColIndex = left; colIndex < width; colIndex++, bmpColIndex++)
			{
				/* 如果该点为需要填充 并且位图填充色与lcd填充色一致 则填充 */
				int pos = page*lua_lcd_width+colIndex;
				UINT16 rgb = 0xffffff;
				if(0 == ((data_buf[bitmapRowBytes*bitmapRowIndex+bmpColIndex/8]&(0x80>>(bmpColIndex%8)))^(finalfill>>(bmpColIndex%8))))
				{
					//gdi_w32_lcd_buffer[page*lua_lcd_width+colIndex] |= 1<<(rowIndex%8);
					rgb = 0xffffff;
				}
				else /* 否则不填充 */
				{
					//gdi_w32_lcd_buffer[page*lua_lcd_width+colIndex] &= ~(1<<(rowIndex%8));
					rgb = 0x000000;
				}

				buffer16[pos++] = R_OF_RGB888(rgb); //R
				buffer16[pos++] = R_OF_RGB888(rgb); //G
				buffer16[pos++] = R_OF_RGB888(rgb); //B
				buffer16[pos++] = 0x00; //alpha
			}
		}

		return PLATFORM_OK;    
	}


	if(bmp_bpp != 24 && bmp_bpp != 16)
	{
		LogWriter::LOGX("put_bmp_buffer: bmp not support bpp %d", bmp_bpp);
		return PLATFORM_ERR;
	}

	if(lua_lcd_bpp == 16)
	{
		for(rowIndex = y, bitmapRowIndex = p_infoHeader->height - top - 1; 
			rowIndex < height && bitmapRowIndex >= p_infoHeader->height - bottom - 1;
			rowIndex++, bitmapRowIndex--)
		{
			for(colIndex = x, bmpColIndex = left; colIndex < width; colIndex++, bmpColIndex++)
			{
				data_index = bitmapRowIndex*bitmapRowBytes + bmpColIndex*bmp_bpp/8;

				if(24 == bmp_bpp)
				{
					data_r = data_buf[data_index+2];
					data_g = data_buf[data_index+1];
					data_b = data_buf[data_index];
					rgb16 = WinUtil::rgb888_to_rgb565(RGB(data_r,data_g,data_b));
				}
				else
				{
					rgb16 = *(UINT16*)&data_buf[data_index];
				}

				if(-1 == transcolor || rgb16 != transcolor)
				{
					int pos = rowIndex*lua_lcd_width+colIndex;
					DWORD rgb888 = WinUtil::rgb565_to_rgb888(rgb16);
					//buffer16[rowIndex*lua_lcd_width+colIndex] = rgb16;
					buffer16[pos ++] = R_OF_RGB888(rgb888);
					buffer16[pos ++] = G_OF_RGB888(rgb888);
					buffer16[pos ++] = B_OF_RGB888(rgb888);
					buffer16[pos ++] = 0x00;
				}
			}
		}
	}
	else
	{
		for(rowIndex = y, bitmapRowIndex = p_infoHeader->height - top - 1; 
			rowIndex < height && bitmapRowIndex >= p_infoHeader->height - bottom - 1;
			rowIndex++, bitmapRowIndex--)
		{
			int pos = (rowIndex *lua_lcd_width + x)* 4;
			int offset = bitmapRowIndex * bitmapRowBytes  + left * 3;
			int offsetmax = offset + real_width*3;

			for(; offset<offsetmax; )
			{
				buffer24[pos++] = data_buf[offset + 0];        //B
				buffer24[pos++] = data_buf[offset + 1];        //G
				buffer24[pos++] = data_buf[offset + 2];        //R
				buffer24[pos++] = 0x00;        //alpha
				offset = offset + 3;
			}
		}

	}
	return PLATFORM_OK;
}

INT Disp::platform_get_png_file_resolution(const char *filename, UINT32* width, UINT32* height)
{

	png_FILE_p fp;
	png_structp read_ptr;
	png_infop read_info_ptr;

	fp = fopen(filename, "rb");
	if(NULL == fp)
	{
		LogWriter::LOGX("[put_png_file_buff]: %s file not exist.\n", filename);
		return PLATFORM_ERR;
	}

	read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	read_info_ptr = png_create_info_struct(read_ptr);    

#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(read_ptr)))
	{
		LogWriter::LOGX("[put_png_file_buff]: %s libpng read error\n", filename);
		png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
		fclose(fp);
		return PLATFORM_ERR;
	} 
#endif

	png_init_io(read_ptr, fp);

	png_read_info(read_ptr, read_info_ptr);
	*width = png_get_image_width(read_ptr, read_info_ptr);
	*height = png_get_image_height(read_ptr, read_info_ptr);

	png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
	fclose(fp);

	return PLATFORM_OK;
}

int Disp::put_png_file_buff(const char *filename, int x, int y, int transcolor, int left, int top, int right, int bottom, int transtype)
{
    png_FILE_p fp;
    png_structp read_ptr;
    png_infop read_info_ptr;
    png_bytep row_buf = NULL;
    png_uint_32 width, height, row_idx, tmp_idx;
    png_byte color_type,channel;
    png_size_t row_bytes;

    UINT16 *buffer16 = (UINT16*)gdi_w32_lcd_buffer;
    UINT8 data_alpha,proc;
    UINT16 rgb16;
    short fr,fg,fb,fa;
    short br,bg,bb,ba;
    UINT8 dr,dg,db,da;
    UINT8 fpercent, bpercent;

    UINT8 *buffer24 = (UINT8*)gdi_w32_lcd_buffer;
    UINT32 layer_width = lua_lcd_width;
    
	UINT32 begin = WinUtil::ust_get_current_time();

    if((left > right) || (top > bottom))
    {
		LogWriter::LOGX("[put_png_file_buff]: rect error.");
        return PLATFORM_ERR;
    }

    fp = fopen(filename, "rb");
    if(NULL == fp)
    {
		LogWriter::LOGX("[put_png_file_buff]: %s file not exist.", filename);
        return PLATFORM_ERR;
    }

    read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    read_info_ptr = png_create_info_struct(read_ptr);    

    #ifdef PNG_SETJMP_SUPPORTED
    if (setjmp(png_jmpbuf(read_ptr)))
    {
       LogWriter::LOGX("[put_png_file_buff]: %s libpng read error", filename);
       png_free(read_ptr, row_buf);
       row_buf = NULL;
       png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
       fclose(fp);
       return PLATFORM_ERR;
    } 
    #endif

    png_init_io(read_ptr, fp);
    png_set_bgr(read_ptr);
    
    png_read_info(read_ptr, read_info_ptr);
    width = png_get_image_width(read_ptr, read_info_ptr);
    height = png_get_image_height(read_ptr, read_info_ptr);
    color_type = png_get_color_type(read_ptr, read_info_ptr);
    channel = png_get_channels(read_ptr, read_info_ptr);
    row_bytes = png_get_rowbytes(read_ptr, read_info_ptr);
    /*if(strcmp(filename,"BAT.png") == 0)
    {
        printf("[put_png_file_buff]: width=%d,height=%d,color_type=%d,channel=%d,row_bytes=%d\n", width, height, color_type, channel,row_bytes);
    }*/
    if((3 != channel) && (4 != channel))
    {
		LogWriter::LOGX("[put_png_file_buff]: channel %d error", channel);
        png_free(read_ptr, row_buf);
        row_buf = NULL;
        png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
        fclose(fp);
        return PLATFORM_ERR;
    }

    row_buf = (png_bytep)png_malloc(read_ptr, row_bytes);
    if(NULL == row_buf)
    {
		LogWriter::LOGX("[put_png_file_buff]: %d row no buf error", row_bytes);
        png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
        fclose(fp);
        return PLATFORM_ERR;
    }

    if((left == 0) && (top == 0) && (right == 0) && (bottom == 0))
    {
        left = 0;
        top = 0;
        right = width - 1;
        bottom = height - 1;
        
        if(lua_lcd_bpp == 24 && channel == 3)
        {
            int write_index = ((y)*layer_width + x) * 4; //((y)*layer_width + x) * 3
            
            LogWriter::LOGX("[put_png_file_buff]: FAST READ");

            for(row_idx=0; row_idx< height; row_idx++)
            {
                png_read_row(read_ptr, row_buf, NULL);
				/*
                memcpy(&buffer24[write_index],
                        row_buf,
                        row_bytes);
				*/
				for(int i=0, pos=write_index; i<row_bytes; )
				{
					buffer24[pos++] = row_buf[i + 0];        //B
					buffer24[pos++] = row_buf[i + 1];        //G
					buffer24[pos++] = row_buf[i + 2];        //R
					buffer24[pos++] = 0x00;        //alpha
					i = i + 3;
				}
                
                //write_index += layer_width * 3;
				write_index += layer_width * 4;
            }

            #ifdef PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
              #ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
                    png_free_data(read_ptr, read_info_ptr, PNG_FREE_UNKN, -1);
              #endif
            #endif

            png_free(read_ptr, row_buf);
            row_buf = NULL;
            png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
            fclose(fp);

			LogWriter::LOGX("put png file time %d", WinUtil::ust_get_current_time() - begin);
            return PLATFORM_OK;

        }
    }

    for(row_idx=0; row_idx<height; row_idx++)
    {
         if(row_idx > bottom)
         {
             break;
         }
         
        png_read_row(read_ptr, row_buf, NULL);
        
        if(row_idx < top)
        {
            continue;
        }

         
        for(tmp_idx=left; tmp_idx<=right; tmp_idx++)
        {
            proc = 0;
            
            if(channel==4)
            {
               data_alpha = row_buf[tmp_idx*channel + 3];
            }

            if(lua_lcd_bpp == 16)
            {
                fr = row_buf[tmp_idx*channel + 2];
                fg = row_buf[tmp_idx*channel + 1];
                fb = row_buf[tmp_idx*channel + 0];

                rgb16 = WinUtil::rgb888_to_rgb565(RGB(fr,fg,fb));
                
                if(-1 == transcolor || rgb16 != transcolor || transtype == 2)
                {
                    if(channel==4)
                    {
                        //0:正常处理全透，非全透的当做不透明处理
                        //1:正常处理所有透明度
                        //2:正常处理全透，非全透的当成与transcolor相同的透明度处理
                        if((transtype==0 && data_alpha==0)
                            || (transtype==1)
                            || (transtype==2)
                            )
                        {
                            proc = 1;

                            br = (buffer16[(y+row_idx-top)*layer_width+(x+tmp_idx-left)] >> 8) & 0xf8;
                            bg = (buffer16[(y+row_idx-top)*layer_width+(x+tmp_idx-left)] >> 3) & 0xfc;
                            bb = (buffer16[(y+row_idx-top)*layer_width+(x+tmp_idx-left)] << 3) & 0xf8;
                            
                            
                           if(transtype==2 && data_alpha != 0)
                            {
                                fpercent = (float)transcolor/255;
                            }
                            else
                            {
                                fpercent = (float)data_alpha/255;
                            }
                            
                            bpercent = 1 - fpercent;
                            
                            dr = fr * fpercent + br * bpercent; 
                            dg = fg * fpercent + bg * bpercent; 
                            db = fb * fpercent + bb * bpercent; 
                            
                            buffer16[(y+row_idx-top)*layer_width+(x+tmp_idx-left)] = WinUtil::rgb888_to_rgb565(RGB(dr, dg, db));
                            
                        }                    
                    }
                    
                    if(proc==0)
                    {
                        buffer16[(y+row_idx-top)*layer_width+(x+tmp_idx-left)] = rgb16;
                    }
                }
            }
            else  if(lua_lcd_bpp == 24)
            {
            
                fr = row_buf[tmp_idx*channel + 2];
                fg = row_buf[tmp_idx*channel + 1];
                fb = row_buf[tmp_idx*channel + 0];
				fa = 0xff;

                if(channel==4)
                {
                    //0:正常处理全透，非全透的当做不透明处理
                    //1:正常处理所有透明度
                    //2:正常处理全透，非全透的当成与transcolor相同的透明度处理
                    if(data_alpha == 0xff)
                    {
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 2] = fr;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 1] = fg;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 0] = fb;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 3] = fa;

						continue;
                    }
                    else if(data_alpha == 0)
                    {
                        continue;
                    }
                    else
                    {
					    if(transtype== 2 && data_alpha != 0)
						{
						    fpercent = transcolor;
						}
						else
						{
						    fpercent = data_alpha;
						}
						br = buffer24[((y+row_idx-top)*layer_width + (x+tmp_idx-left)) * 4 + 2] ;
						bg = buffer24[((y+row_idx-top)*layer_width + (x+tmp_idx-left)) * 4 + 1] ;
						bb = buffer24[((y+row_idx-top)*layer_width + (x+tmp_idx-left)) * 4 + 0] ;
						ba = buffer24[((y+row_idx-top)*layer_width + (x+tmp_idx-left)) * 4 + 3] ;

                                                    
                        dr = ((((fr - br)* fpercent) >> 8) ) + br; 
                        dg = ((((fg - bg)* fpercent) >> 8) ) + bg; 
                        db = ((((fb - bb)* fpercent) >> 8) ) + bb;
						da = ba;
                        
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 2] = dr;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 1] = dg;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 0] = db;
						buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 3] = da;
                    }     
                }
				else
				{
					buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 2] = fr;
					buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 1] = fg;
					buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 0] = fb;
					buffer24[((y+row_idx-top)*layer_width+(x+tmp_idx-left))*4 + 3] = fa;
				}

                
            }
        }
    }

    #ifdef PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
      #ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
            png_free_data(read_ptr, read_info_ptr, PNG_FREE_UNKN, -1);
      #endif
    #endif

    png_free(read_ptr, row_buf);
    row_buf = NULL;
    png_destroy_read_struct(&read_ptr, &read_info_ptr, NULL);
    fclose(fp);

	LogWriter::LOGX("put png file time %d", WinUtil::ust_get_current_time() - begin);
    
    return PLATFORM_OK;
}


long Disp::ReadPngData(const CHAR *szPath,
	BITMAP *bitmap)
{
	FILE *fp = NULL;
	long file_size = 0, pos = 0, mPos = 0;
	int color_type = 0, x = 0, y = 0, block_size = 0;
	int bit_depth;
	BYTE* bmBits;

	png_infop info_ptr,end_info_ptr;
	png_structp read_ptr;
	png_bytep *row_point = NULL;

	fp = fopen(szPath, "rb");
	if( !fp ){
		LogWriter::LOGX("[Disp::ReadPngData] Don't open file:%s",szPath);
		return FILE_ERROR;
	}

	read_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);        //创建png读取结构
	info_ptr = png_create_info_struct(read_ptr);        //png 文件信息结构
	end_info_ptr = png_create_info_struct(read_ptr);
	png_set_read_fn(read_ptr, (png_voidp)fp, NULL);
	png_read_png(read_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);                //读取png文件


	bitmap->bmWidth  = png_get_image_width(read_ptr, info_ptr);        //获得图片宽度
	bitmap->bmHeight = png_get_image_height(read_ptr, info_ptr);        //获得图片高度
	bitmap->bmType = png_get_color_type(read_ptr, info_ptr);        //获得图片色彩深度
	bitmap->bmPlanes = 1;
	bit_depth = png_get_bit_depth(read_ptr, info_ptr);
	bitmap->bmBitsPixel = bit_depth * 4;
	file_size = (bitmap->bmWidth * bitmap->bmHeight + 1) * 4;                    //计算需要存储RGB(A)数据所需的内存大小
	bitmap->bmBits = WinUtil::L_MALLOC(file_size);            //申请所需的内容, 并将 *cbData 指向申请的这块内容
	bmBits = (BYTE*)bitmap->bmBits;


	row_point = png_get_rows( read_ptr, info_ptr );            //读取RGB(A)数据

	if(row_point == NULL)
	{
		LogWriter::LOGX("[Disp::ReadPngData] Read Data Fail!");
		WinUtil::L_FREE(bmBits);
		png_destroy_read_struct(&read_ptr, &info_ptr, &end_info_ptr);
		fclose( fp );
		return FILE_ERROR;
	}


	block_size = color_type == 6 ? 4 : 3;                    //根据是否具有a通道判断每次所要读取的数据大小, 具有Alpha通道的每次读4位, 否则读3位
	bitmap->bmWidthBytes = 4;

	//将读取到的RGB(A)数据按规定格式读到申请的内存中
	for(x = 0; x < bitmap->bmWidth; x++) {
		for(y = 0; y < bitmap->bmHeight*block_size; y+=block_size)
		{
			(bmBits)[pos++] = row_point[x][y + 2];
			(bmBits)[pos++] = row_point[x][y + 1];
			(bmBits)[pos++] = row_point[x][y + 0];
			(bmBits)[pos++] = row_point[x][y + 3];
		}
	}


	png_destroy_read_struct(&read_ptr, &info_ptr, &end_info_ptr);
	fclose( fp );


	return file_size;
}

Disp* Disp::GetDisp(void)
{
	if (onlyWatchDisp == NULL)
	{
		//onlyWatchDisp = new Disp();
		//LogWriter::LOGX("[platform_disp] GetDisp onlyWatchDisp=%x.",onlyWatchDisp);
	}
	return onlyWatchDisp;
}

void Disp::SetDisp(Disp* disp)
{
	onlyWatchDisp = disp;
}

INT Disp::GetDispRun(void)
{
	return Disp::GetDisp()->dispRun;
}

void Disp::SetDispRun(INT run)
{
	Disp::GetDisp()->dispRun = run;
}

HWND Disp::GetDispWnd(void)
{
	return Disp::GetDisp()->device.hwnd;
}

Disp::Disp()
{
	dispRun = 0;
	LogWriter::LOGX("[Disp::Disp()] onlyWatchDisp=%x.",onlyWatchDisp);
}

void Disp::w32_imitated_tp_reset(void)
{
	tp_event.model = 0;
	tp_event.time_stamp = 0;
	tp_event.points.event = TP_INVALID;
	tp_event.points.x = 0;
	tp_event.points.y = 0;

	last_tp_event.model = 0;
	last_tp_event.time_stamp = 0;
	last_tp_event.points.event = TP_INVALID;
	last_tp_event.points.x = 0;
	last_tp_event.points.y = 0;
}

void Disp::Disp_Initialize(void)
{
  //SetDispRun(1);
  dispRun = 1;
  g_bUpdateScreen = FALSE;
  g_s_fontZoomSize = HZ_FONT_HEIGHT;
  g_mainlcd_backlight_level_min = 4;
  g_mainlcd_backlight_level_max = 9;
  mainlcd_backlight_level = 9;

  gdi_w32_lcd_buffer = NULL;

  device.main_bitmap = NULL;
  device.main_bitmap_down = NULL;

  lcd_hdcMem = NULL;

  curr_font_id = 0;

  lcd_itf_t itf_t = {0,0,0,0};
  PlatformDispInitParam mDispInitParam = {240, 240, \
	24, \
	0,0, \
	NULL,0, \
	NULL,0, \
	NULL,0, \
	PLATFORM_LCD_BUS_MAX, \
	itf_t, \
	-1, \
	0xffffff, \
	-1, \
	NULL \
  };

  dispInitParam = mDispInitParam;

  FontInfo mSansFont16 = {ASC_FONT_WIDTH,ASC_FONT_HEIGHT,ASC_FONT_SIZE,0x20,0x7E, sansFont16Data};
  sansFont16 = mSansFont16;

  FontInfo mSansHzFont16 = {HZ_FONT_WIDTH,HZ_FONT_HEIGHT,HZ_FONT_SIZE,0,0,NULL};
  sansHzFont16 = mSansHzFont16;

  FontInfo mSansHzFontPinyin = {ASC_FONT_WIDTH,HZ_FONT_HEIGHT,ASC_FONT_SIZE,0xa8a1,0xa8bf,NULL};
  sansHzFontPinyin = mSansHzFontPinyin;

  /*
  disp_bkcolor = COLOR_WHITE_1;
  disp_color = COLOR_BLACK_1;
  */
  disp_bkcolor = COLOR_BLACK_1;
  disp_color = COLOR_WHITE_1;
  lcd_hwfillcolor = COLOR_BLACK_1;

  w32_imitated_tp_reset();
}

Disp::Disp(HWND hwnd, HINSTANCE hInstance)
{
	CHAR path[256];

	LogWriter::LOGX("[Disp::Disp(x,x)] onlyWatchDisp=%x.",onlyWatchDisp);

	Disp_Initialize();

	lcd_Instance = hInstance;
	strcpy(path, w32_GerProfilePath());
	w32_ReadKeyProfile(path,hwnd);

	fontInit();
}

Disp::~Disp()
{
	if(gdi_w32_lcd_buffer != NULL)
	{
		WinUtil::L_FREE(gdi_w32_lcd_buffer);
		gdi_w32_lcd_buffer = NULL;
	}

	if(lcd_hdcMem != NULL) DeleteDC(lcd_hdcMem);
}

void Disp::WinAbnormalExit(HWND hwnd, PCSTR msg)
{
	HWND hWnd = hwnd == NULL ? GetDispWnd() : hwnd;
    TCHAR szTitle[MAX_LOADSTRING];
	TCHAR szError[MAX_LOADSTRING];

	LogWriter::LOGX("[Disp] WinAbnormalExit!!!");

	LoadString(lcd_Instance,IDS_APP_TITLE,szTitle,MAX_LOADSTRING);
	LoadString(lcd_Instance,IDS_UNKNOWN_FATAL_ERROR,szError,MAX_LOADSTRING);
	if(msg != NULL)
	{
		MessageBox(hWnd,(LPTSTR)msg,szTitle,MB_OK);
	}
	else
	{
		MessageBox(hWnd,szError,szTitle,MB_OK);
	}
	PostMessage(hWnd, WM_COMMAND, IDM_FILE_EXIT, NULL);
}

void Disp::DispInit(HWND hwnd, HINSTANCE hInstance)
{
	CHAR path[256];

	Disp_Initialize();

	lcd_Instance = hInstance;
	strcpy(path, w32_GerProfilePath());
	w32_ReadKeyProfile(path,hwnd);

	fontInit();
	init(&dispInitParam);
}

CHAR* Disp::w32_GerProfilePath(void)
{
	CHAR profilePath[256];
	GetPrivateProfileString("LCD","MMI_SKIN_NAME", NULL, profilePath, 256, "./Skin/SkinStyleLog.ini");
	return profilePath;
}

CHAR* Disp::w32_GetProfileName(void)
{
	CHAR profileName[512];
	strcpy(profileName, "./Profile_Revise.ini") ;	
	return profileName;
}

INT Disp::load_file_data(const char *name, UINT8 **buf_pp)
{
	FILE *fp = NULL;
	INT size;
	CHAR *buf = NULL;

	fp = fopen(name, "rb");

	if(fp == NULL)
	{
		LogWriter::LOGX("[Disp::load_file_data]: file not exist!");
		goto load_error;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	buf = (CHAR*)malloc(size);
	if(NULL ==  buf)
	{
		LogWriter::LOGX("[Disp::load_file_data]: no memory");
		goto load_error;
	}
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);
	fclose(fp);

	*buf_pp = (UINT8*)buf;
	return size;

load_error:
	if(fp)
		fclose(fp);

	if(buf)
		free(buf);
	return 0;
}

INT Disp::loadfont(const CHAR *name)
{
	UINT8 *buf = NULL;
	INT size = 0;
	FontFileInfo *fileInfo_p;
	UINT32 charsize;
	BOOL found = FALSE;
	INT i;
	INT retcode = 0;

	size = load_file_data(name, &buf);

	if(size <= sizeof(FontFileInfo))
	{
		retcode = -1;
		goto load_font_error;
	}

	fileInfo_p = (FontFileInfo *)buf;

	if(fileInfo_p->type != 0)
	{
		// 只支持连续字符方式的字库
		retcode = -2;
		goto load_font_error;
	}

	if(fileInfo_p->end < fileInfo_p->start)
	{
		retcode = -3;
		goto load_font_error;
	}

	charsize = (fileInfo_p->width+7)/8;
	charsize *= fileInfo_p->height;

	if(charsize*(fileInfo_p->end - fileInfo_p->start + 1) != size - sizeof(FontFileInfo))
	{
		retcode = -4;
		goto load_font_error;
	}

	for(i = 0; i < MAX_FONTS && !found; i++)
	{
		if(dispFonts[i].data == NULL)
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		retcode = -5;
		goto load_font_error;
	}

	dispFonts[i].width = fileInfo_p->width;
	dispFonts[i].height = fileInfo_p->height;
	dispFonts[i].size = charsize;
	dispFonts[i].start = fileInfo_p->start;
	dispFonts[i].end = fileInfo_p->end;
	dispFonts[i].data = buf + sizeof(FontFileInfo);

	return i;

load_font_error:
	if(buf)
		free(buf);

	LogWriter::LOGX("[Disp::loadfont]:error code %d", retcode);

	return -1;
}

void Disp::fontInit(void)
{
	sansHzFont16.data = sansHzFont16Data;
	sansHzFontPinyin.data = sansHzFontPinyiData;

	memset(dispFonts, 0, sizeof(dispFonts));

	dispFonts[0] = sansFont16;
	dispHzFont = &sansHzFont16;
}

INT Disp::setfont(INT id)
{
	int old_font_id;

	if(id < 0 || id >= MAX_FONTS || NULL == dispFonts[id].data)
	{
		LogWriter::LOGX("[Disp::setfont]: error font id");
		return -1;
	}

	old_font_id = curr_font_id;
	curr_font_id = id;
	return old_font_id;
}

INT Disp::setfontHeight(INT height)
{
	if(height < 10 || height > 60)
	{
		return -1;
	}
	else
	{
		g_s_fontZoomSize = height;
		return 0;
	}
}

INT Disp::getfontHeight(void)
{
	return g_s_fontZoomSize;
}

void Disp::lcd_init(const PlatformDispInitParam *pParam)
{
	//TO DO ...
	dispInitParam = *pParam;
}

void Disp::SetClientSize(HWND hWnd, INT width, INT height)
{
	if(!hWnd)
	{
		TCHAR szError[MAX_LOADSTRING];
		LoadString(lcd_Instance,IDS_SET_CLIENT_FAIL,szError,MAX_LOADSTRING);
		WinAbnormalExit(hWnd,szError);
		return;
	}
	RECT  rectProgram,rectClient;
	GetWindowRect(hWnd, &rectProgram);   //获得程序窗口位于屏幕坐标
	GetClientRect(hWnd, &rectClient);      //获得客户区坐标
	//非客户区宽,高
	int nWidth = rectProgram.right - rectProgram.left -(rectClient.right - rectClient.left); 
	int nHeiht = rectProgram.bottom - rectProgram.top -(rectClient.bottom - rectClient.top); 
	nWidth += width;
	nHeiht += height;
	rectProgram.right =  nWidth;
	rectProgram.bottom =  nHeiht;
	int showToScreenx = GetSystemMetrics(SM_CXSCREEN)/2-nWidth/2;    //居中处理
	int showToScreeny = GetSystemMetrics(SM_CYSCREEN)/2-nHeiht/2;
	MoveWindow(hWnd, showToScreenx, showToScreeny, rectProgram.right, rectProgram.bottom, false);
}

void Disp::MTKLoadImage(HDC* hMemDC, LPCSTR buffer, SIZE* sz)
{
	CHAR path[512];
	BITMAP bitmap;
	HBITMAP hBitmap;

	GetCurrentDirectory(512, path);
	strcat(path,"\\");
	strcat(path,buffer);
	hBitmap = (HBITMAP)LoadImage(NULL,path,IMAGE_BITMAP,
		0,0,LR_LOADFROMFILE);
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	sz->cx = bitmap.bmWidth;
	sz->cy = bitmap.bmHeight;
	*hMemDC = CreateCompatibleDC(NULL);
	SelectObject(*hMemDC, hBitmap);
}

void Disp::ReadOneImageInformation(
	LPCSTR pcszImageName, 
	UINT* plWidth, UINT* plHeight,
	HDC* pbmpImage, HDC hdcTplt,
	LPCSTR pcszIniSection, LPCSTR pcszIniPath)
{
	CHAR	buffer[512];
	INT buffer_size = 512;
	HBITMAP img = NULL;
	BITMAP	bmp = {0,0,0,0,0,0, NULL};
	SIZE sz;

	GetPrivateProfileString(pcszIniSection, pcszImageName, NULL, buffer, buffer_size, pcszIniPath);
	{
		MTKLoadImage(pbmpImage, (LPCSTR)buffer, &sz);
		(*plWidth) = sz.cx;
		(*plHeight) = sz.cy;
	}

}

/*****************************************************************************
* FUNCTION
*  ReadOneKeyInformation
* DESCRIPTION
*  read one key information from the INI file
*
*****************************************************************************/
void Disp::ReadOneKeyInformation(LPCSTR pcszKeyname, RECT* prectKey, 
	HDC hdcTplt, LPCSTR pcszIniSection, LPCSTR pcszIniPath)
{
	char szKeyLayout[128];
	char buffer[512];
	int buffer_size = 512;

	memset(szKeyLayout, 0, 128);
	memset(buffer, 0, 512);

	if(NULL == pcszKeyname)
		return;

	strcpy( szKeyLayout, pcszKeyname);
	strcat( szKeyLayout, "_LAYOUT");
	GetPrivateProfileString(pcszIniSection, szKeyLayout, NULL, buffer, buffer_size, pcszIniPath);
	if (strlen(buffer) <= 0)
		return;

	WinUtil::ParseLayoutString(buffer, strlen(buffer), prectKey);
}

/*****************************************************************************
* FUNCTION
*  w32_ReadKeyMapping
* DESCRIPTION
*  read the key mapping from the INI file
*
* PARAMETERS
*	in_path		IN		path for INI file
*	hwnd			IN		window handle
* RETURNS
*  void
* GLOBALS AFFECTED
*  void
*****************************************************************************/
void Disp::w32_ReadKeyMapping(CHAR in_path[], HWND hwnd)
{
	CHAR	buffer[512], path[512];
	INT i ;
	HANDLE	hFind;
	WIN32_FIND_DATA FindFileData;


	hFind = FindFirstFile(PATH_LUA_DEBUG_INI, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		/* Can't find the file "AMLuaDebug.ini" */
		return;
	}
	FindClose(hFind);

	/* get the complete name for opening the INI file */
	strcpy(path, PATH_LUA_DEBUG_INI);

	for( i = 0 ; i < MAX_KEY_SUPPORT ; ++i )
	{
		if( GetPrivateProfileString("Keyboard-Mapping", KeyMappingName[i] , NULL, buffer, sizeof(buffer), path) != 0 )
			key_mapping_table[i].lua_key_code = WinUtil::atohex(buffer);

		LogWriter::LOGX("[Disp] w32_ReadKeyMapping key_mapping_table[%d].lua_key_code=%x.",i,key_mapping_table[i].lua_key_code);
	}	
}

void Disp::w32_screen_init(HWND hWnd, HINSTANCE hInstance)
{
	lcd_Instance = hInstance;
	device.hwnd = hWnd;
	w32_dspl_init();
	SetClientSize(hWnd,
		device.window_width,
		device.window_height);
}

void Disp::w32_imitated_input(UINT16 type, POINT p)
{
	static UINT event = TOUCH_ACTION_NULL;
	UINT16 x = 0, y = 0;
	signed offsetX = 0, offsetY = 0;

	if(onlyWatchDisp == NULL || g_s_init_end_bool == FALSE)
	{
		//LogWriter::LOGX("[platform_disp] w32_imitated_input NULL.");
		return;
	}

	if((p.x > lcd_offset_x && p.x <  lcd_offset_x + skin_lcd_width) && \
		(p.y > lcd_offset_y && p.y <  lcd_offset_y + skin_lcd_height))
	{
		x = p.x - lcd_offset_x;
		y = p.y - lcd_offset_y;
	}
	else
	{
		if(type == WM_LBUTTONDOWN)
		{
			key_mapping(1, p);
		}
		else if(type == WM_LBUTTONUP)
		{
			key_mapping(0, p);
		}
		else
		{
			return;
		}
		tp_capture_stance_reset();
		LogWriter::LOGX("[platform_disp] w32_imitated_input(Null-Touch).");
		return;
	}

	if(type == WM_LBUTTONDOWN)
	{
		tp_capture_stance_reset();
		event = TP_EVENT_DOWN;

		tp_event.time_stamp = WinUtil::ust_get_current_time();
		tp_event.points.event = event;
		tp_event.points.x = x;
		tp_event.points.y = y;

		last_tp_event.time_stamp = 0;
		last_tp_event.points.event = TP_INVALID;
		last_tp_event.points.x = 0;
		last_tp_event.points.y = 0;

		offsetX = 0, offsetY = 0;
	}
	else if(type == WM_LBUTTONUP)
	{
		last_tp_event.points.event = event;
		last_tp_event.time_stamp = WinUtil::ust_get_current_time();
		last_tp_event.points.x = x;
		last_tp_event.points.y = y;
		offsetX = calcMoveOffset(last_tp_event.points.x, tp_event.points.x);
		offsetY = calcMoveOffset(last_tp_event.points.y, tp_event.points.y);

		if( tp_event.points.x == last_tp_event.points.x && \
			tp_event.points.y == last_tp_event.points.y )
		{
			if(last_tp_event.time_stamp - tp_event.time_stamp > TP_SINGLE_DEBOUNCE_TIME)
			{
				tp_capture_stance(TP_EVENT_LONGTAP_HOLD, x, y);

				event = TP_EVENT_UP;
			}			
		}
		else if( offsetX > TP_MOVE_STEP_MIN_VALUE || offsetY > TP_MOVE_STEP_MIN_VALUE)
		{
			
			if(last_tp_event.time_stamp - tp_event.time_stamp > TP_LONGTAG_DEBOUNCE_TIME)
			{
				event = TP_EVENT_UP;
			}
			else if(last_tp_event.time_stamp - tp_event.time_stamp > TP_SINGLE_DEBOUNCE_TIME)
			{
				event = TP_EVENT_UP;
			}
			else
			{
				w32_imitated_tp_reset();
				return;
			}
		}

		w32_imitated_tp_reset();
	}
	else if(type == WM_MOUSEMOVE)
	{
		if(tp_event.points.event == TP_INVALID)
		{
			return;
		}

		if(WinUtil::ust_get_current_time() - tp_event.time_stamp < TP_SINGLE_DEBOUNCE_TIME)
		{
			event = TP_EVENT_LONGTAP;
		}
		else if(WinUtil::ust_get_current_time() - tp_event.time_stamp < TP_LONGTAG_DEBOUNCE_TIME)
		{
			event = TP_EVENT_LONGTAP_HOLD;
		}
		else
		{
			event = TP_EVENT_MOVE;
		}
	}
	else
	{
		tp_capture_stance_reset();

		event = TOUCH_ACTION_NULL;

		tp_event.time_stamp = 0;
		tp_event.points.event = TP_INVALID;
		tp_event.points.x = 0;
		tp_event.points.y = 0;
		last_tp_event.time_stamp = 0;
		last_tp_event.points.event = TP_INVALID;
		last_tp_event.points.x = 0;
		last_tp_event.points.y = 0;
		offsetX = 0, offsetY = 0;
	}

	tp_capture_stance(event, x, y);
}

void Disp::w32_light_black_draw(int x, int y, int w, int h, HDC hdc, HDC lcd)
{
	RECT rc;
	BLENDFUNCTION bf;
	HBRUSH	black_brush;
	HDC memoryDC;
	HBITMAP bitmap;

	if(mainlcd_backlight_level == g_mainlcd_backlight_level_max)
		return;

	rc.left   = 0;
	rc.top    = 0;
	rc.right  = 0 + w;
	rc.bottom = 0 + h;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.AlphaFormat = 0; // ignore source alpha channel
	bf.SourceConstantAlpha =
		mainlcd_backlight_level*255 / g_mainlcd_backlight_level_max;
	black_brush = CreateSolidBrush(RGB(0, 0, 0));

	/***** Double buffering *****/
	//1. AlphaBlend
	memoryDC = CreateCompatibleDC(hdc);
	bitmap   = CreateCompatibleBitmap(hdc, w, h);
	SelectObject(memoryDC, bitmap);
	FillRect(memoryDC, &rc, black_brush);
	if(mainlcd_backlight_level > g_mainlcd_backlight_level_min && mainlcd_backlight_level < g_mainlcd_backlight_level_max)
		AlphaBlend(memoryDC, 0, 0, w, h, lcd, 0, 0, w, h, bf);	//if backlight off, don't alphablend, or the lcd will mess on some special occasions
	//2. BitBlt
	BitBlt(hdc, x, y, w, h,memoryDC, 0, 0, SRCCOPY);

	DeleteObject(black_brush);
	DeleteObject(bitmap);
	DeleteDC(memoryDC);
}

void Disp::w32_update_screen(void)
{
	HDC hdc;

	if (g_bUpdateScreen) return;
	g_bUpdateScreen = TRUE;

	hdc = GetDC(device.hwnd);
	if(mainlcd_backlight_level == g_mainlcd_backlight_level_max)
	{
		w32_put_screen(lcd_offset_x, lcd_offset_y, skin_lcd_width, skin_lcd_height, gdi_w32_lcd_buffer);
	}
	else if( mainlcd_backlight_level >= g_mainlcd_backlight_level_min && mainlcd_backlight_level<=g_mainlcd_backlight_level_max-1)
	{
		w32_light_black_draw(lcd_offset_x, lcd_offset_y, skin_lcd_width, skin_lcd_height, hdc, lcd_hdcMem) ;
	}

	ReleaseDC(device.hwnd,hdc);

	g_bUpdateScreen = FALSE;
}

void Disp::w32_dspl_init(void)
{
	HDC	hdc = NULL;

	/* initialize the information about dimension */
	skin_lcd_height	= device.main_lcd_height;
	skin_lcd_width	= device.main_lcd_width;
	lcd_offset_x = device.skin_main_lcd_x;
	lcd_offset_y = device.skin_main_lcd_y;

	/* draw to simulator's main LCD region */
	hdc	= GetDC(device.hwnd);
	if(lcd_hdcMem) { 	
		DeleteDC(lcd_hdcMem);
	}
	if(lcd_hbmp) {
		DeleteObject(lcd_hbmp);
	}
	lcd_hdcMem	= CreateCompatibleDC(hdc);
	lcd_hbmp = CreateCompatibleBitmap(hdc, skin_lcd_width, skin_lcd_height);
	SelectObject(lcd_hdcMem, lcd_hbmp);

	if(gdi_w32_lcd_buffer == NULL)
	{
		gdi_w32_lcd_buffer = 
			(BYTE*)WinUtil::L_MALLOC((skin_lcd_width+1) * (skin_lcd_height+1) * 4 * sizeof(BYTE));

		INT color = 0x000000;
		for(INT x = 0,pos=0; x < skin_lcd_width; x++)
			for(INT y = 0; y < skin_lcd_height*4; y+=4)
			{
				gdi_w32_lcd_buffer[pos++] = B_OF_RGB888(color);        //B
				gdi_w32_lcd_buffer[pos++] = G_OF_RGB888(color);        //G
				gdi_w32_lcd_buffer[pos++] = R_OF_RGB888(color);        //R
				gdi_w32_lcd_buffer[pos++] = 0;        //alpha
			}
	}

	/* set up the bitmap infomation */
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = skin_lcd_width;
	bitmap_info.bmiHeader.biHeight= skin_lcd_height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biSizeImage = skin_lcd_width * skin_lcd_height * 4;
	bitmap_info.bmiHeader.biClrUsed = 0;
	bitmap_info.bmiHeader.biClrImportant = 0;

	ReleaseDC(device.hwnd, hdc);
}

void Disp::w32_ReadKeyProfile(CHAR in_path[], HWND hwnd)
{
	/*----------------------------------------------------------------*/
	/* Local Variables                                                */
	/*----------------------------------------------------------------*/
	CHAR		buffer[512], old_path[512], path[512];
	INT			buffer_size = 512;
	HDC			hdc;
	INT			i;
	CHAR profile_section_name[256];

	SIZE main_window_size;
	RECT rcMainLcd;

	//Debug using
	CHAR watchtype[64];
	CHAR keypadtype[64];

	memset(watchtype,0,64);
	memset(keypadtype,0,64);
	GetPrivateProfileString(SEC_FEATURES,KEY_DEVICE_TYPE,
		NULL, watchtype, 64, PATH_LUA_DEBUG_INI);
	GetPrivateProfileString(SEC_FEATURES,KEY_KEYPAD_TYPE,
		NULL, keypadtype, 64, PATH_LUA_DEBUG_INI);

	/*----------------------------------------------------------------*/
	/* Code Body                                                      */
	/*----------------------------------------------------------------*/

	/* save the original working directory */
	GetCurrentDirectory(512, old_path);
	/* get the complete name for opening the INI file */
	SetCurrentDirectory(in_path);
	strcpy(path, w32_GetProfileName());

	/* Form section name*/
	memset(profile_section_name,0,256);
	strcpy(profile_section_name, watchtype);
	strcat(profile_section_name, "_");
	strcat(profile_section_name, keypadtype);

	LogWriter::LOGX("Profile section name:%s.",profile_section_name);

	/* get layout related data */
	GetPrivateProfileString(profile_section_name, "MAIN_WINDOW_SIZE", NULL, buffer, buffer_size, path);
	if(strlen(buffer) <= 0)
	{
		WinAbnormalExit(hwnd,"Sorry, LuaDebug UI skin not supported on this project!");
		return;
	}
	WinUtil::ParseSizeString(buffer, strlen(buffer), &main_window_size);
	GetPrivateProfileString(profile_section_name, "MAIN_LCD_LAYOUT", NULL, buffer, buffer_size, path);
	WinUtil::ParseLayoutString(buffer, strlen(buffer), &rcMainLcd);

	device.window_width = main_window_size.cx;
	device.window_height = main_window_size.cy;
	device.skin_main_lcd_x = rcMainLcd.left;
	device.skin_main_lcd_y = rcMainLcd.top;
	device.main_lcd_width = (rcMainLcd.right - rcMainLcd.left);
	device.main_lcd_height = (rcMainLcd.bottom - rcMainLcd.top);

	hdc = GetDC(hwnd);

	/* background image and all keys */
	MTKDeleteDC(device.main_bitmap);
	MTKDeleteDC(device.main_bitmap_down);
	ReadOneImageInformation("BACKGROUND_UP",
		&device.main_bitmap_width, &device.main_bitmap_height, 
		&device.main_bitmap, hdc,
		profile_section_name, path);
	ReadOneImageInformation("BACKGROUND_DOWN",
		&device.main_bitmap_width, &device.main_bitmap_height, 
		&device.main_bitmap_down, hdc,
		profile_section_name, path);

	for( i = 0 ; i < MAX_KEY_SUPPORT ; ++i )
	{
		device.key_location[i].left = device.key_location[i].top = 
			device.key_location[i].right = device.key_location[i].bottom = -1 ;
		ReadOneKeyInformation(KeyMappingName[i], &device.key_location[i], hdc, profile_section_name, path);
	}

	/* restore the original working directory */
	SetCurrentDirectory(old_path);

	ReleaseDC(hwnd, hdc);
	w32_ReadKeyMapping(old_path, hwnd);

	key_mapping_init(device.key_location, key_mapping_table);
}

void Disp::w32_draw_main_bitmap_roi(HDC hdc, RECT rcROI, BOOL bDownEffect)
{
	if (bDownEffect)
	{
		BitBlt(hdc, rcROI.left, rcROI.top,
			rcROI.right - rcROI.left, rcROI.bottom - rcROI.top,
			device.main_bitmap_down, 
			rcROI.left, rcROI.top, SRCCOPY);
	} 
	else
	{
		BitBlt(hdc, rcROI.left, rcROI.top,
			rcROI.right - rcROI.left, rcROI.bottom - rcROI.top,
			device.main_bitmap, 
			rcROI.left, rcROI.top, SRCCOPY);
	}
}

void Disp::w32_draw_main_bitmap(HDC hdc)
{
	RECT rcROI;
	rcROI.left = rcROI.top = 0;
	rcROI.right = device.main_bitmap_width;
	rcROI.bottom = device.main_bitmap_height;
	w32_draw_main_bitmap_roi(hdc, rcROI, FALSE);
}

void Disp::w32_put_screen(INT x, INT y, INT w, INT h, BYTE *bmp)
{
	HDC	hdc;


	hdc = GetDC(device.hwnd);

	if(w == skin_lcd_width && h==skin_lcd_height)
	{
		UINT8* destbmp =(UINT8*)WinUtil::L_MALLOC((skin_lcd_width+1)*(skin_lcd_height+1)*4*sizeof(BYTE));
		RotateBmpBuf(skin_lcd_width,skin_lcd_height,gdi_w32_lcd_buffer,destbmp,360);
		SetDIBits(NULL, lcd_hbmp, 0, skin_lcd_height, destbmp, &bitmap_info, DIB_RGB_COLORS);
		WinUtil::L_FREE(destbmp);
	}
	else
	{
		// Don't support this case.
	}

	if(mainlcd_backlight_level==g_mainlcd_backlight_level_max)
	{
		BitBlt(hdc,	x,y,w,h,lcd_hdcMem, 0, 0, SRCCOPY);
	}
	else if( mainlcd_backlight_level >=g_mainlcd_backlight_level_min && mainlcd_backlight_level<=g_mainlcd_backlight_level_max-1)
	{
		w32_light_black_draw(x,y,w,h,hdc,lcd_hdcMem);			
	}

	ReleaseDC(device.hwnd,hdc);

}

void Disp::init(PlatformDispInitParam *pParam)
{
	assert(pParam->bpp == 16 || pParam->bpp == 1 || pParam->bpp == 24);

	lua_lcd_bpp = pParam->bpp;

	lua_lcd_width = pParam->width;
	if(lua_lcd_bpp == 1) 
	{
		lua_lcd_height = (pParam->height%8 == 0) ? pParam->height : (pParam->height/8 + 1) * 8;
	} 
	else 
	{
		lua_lcd_height = pParam->height;
	}

	// 分配显示缓冲区
#if 0
	framebuffer = L_MALLOC(lua_lcd_width*lua_lcd_height*lua_lcd_bpp/8);
#endif

	pParam->framebuffer = gdi_w32_lcd_buffer;

	if(lua_lcd_bpp == 1)
	{
		disp_bkcolor = COLOR_BLACK_1;
		disp_color = COLOR_WHITE_1;

		if(pParam->hwfillcolor != -1){
			lcd_hwfillcolor = pParam->hwfillcolor;
		}
	}
	else if(lua_lcd_bpp == 16)
	{
		disp_bkcolor = COLOR_BLACK_16;
		disp_color = COLOR_WHITE_16;
	}
	else if(lua_lcd_bpp == 24)
	{
		disp_bkcolor = COLOR_BLACK_24;
		disp_color = COLOR_WHITE_24;
	}

	fontInit();

	// 初始化lcd设备
	lcd_init(pParam);

	LogWriter::LOGX("[disp::init]: [lua_lcd_xxx] bpp=%d,width=%d,height=%d.", lua_lcd_bpp,lua_lcd_width,lua_lcd_height);
}

COLORREF Disp::setcolor(DWORD p)
{
	color c;
	COLORREF old = disp_color;

	c = WinUtil::rgb_to_color(p);
	disp_color = RGB(c.r, c.g, c.b);

	return old;
}

COLORREF Disp::setbkcolor(DWORD p)
{
	color c;
	COLORREF old = disp_bkcolor;
	
	c = WinUtil::rgb_to_color(p);
	disp_bkcolor = RGB(c.b,c.g,c.r);

	return old;
}

void Disp::hdcClear(void)
{
	HBRUSH hBrush;
	HBRUSH hOldBrush;
	RECT cRect = {0,0,skin_lcd_width,skin_lcd_height};
	
	hBrush = CreateSolidBrush(disp_bkcolor);
	hOldBrush = (HBRUSH)SelectObject(lcd_hdcMem, hBrush);
	FillRect(lcd_hdcMem,&cRect,hBrush);
	SelectObject(lcd_hdcMem, hOldBrush);
	DeleteObject(hBrush);
}

void Disp::clear(void)
{
	UINT32 *buf = (UINT32*)gdi_w32_lcd_buffer;

	for(INT x = 0; x < skin_lcd_height; x++)
		for(INT y = 0; y < skin_lcd_width; y++)
		{
			buf[x * skin_lcd_width + y] = disp_bkcolor;
		}
}

void Disp::update(void)
{
	InvalidateRect(device.hwnd,NULL,TRUE);
}

#if 0
INT Disp::drawrect(
	INT startx, INT starty,
	INT endx, INT endy,
	DWORD pColor)
{
	HPEN         hPen;
	HPEN         hPenOld;
	color c;

	if(startx > endx || starty > endy)
	{
		// TO DO ...
		return;
	}
	if(startx < 0) startx = 0;
	if(starty < 0) starty = 0;
	if(endx > skin_lcd_width) endx = skin_lcd_width;
	if(endy > skin_lcd_height) endy = skin_lcd_height;

	c = WinUtil::rgb_to_color(pColor);
	hPen = CreatePen(PS_SOLID, 1, RGB(c.r,c.g,c.b));
	hPenOld = (HPEN)SelectObject(lcd_hdcMem, hPen);

	MoveToEx(lcd_hdcMem, startx, starty, NULL);
	LineTo(lcd_hdcMem, endx, starty);
	MoveToEx(lcd_hdcMem, endx, starty, NULL);
	LineTo(lcd_hdcMem, endx, endy);
	MoveToEx(lcd_hdcMem, endx, endy, NULL);
	LineTo(lcd_hdcMem, startx, endy);
	MoveToEx(lcd_hdcMem, startx, endy, NULL);
	LineTo(lcd_hdcMem, startx, starty);

	SelectObject(lcd_hdcMem, hPenOld);
	DeleteObject(hPen);

	return PLATFORM_OK;
}
#endif

INT Disp::drawrect(INT x1, INT y1,
	INT x2, INT y2, DWORD color)
{
	int i,j;
	UINT32 *buf = (UINT32*)gdi_w32_lcd_buffer;

	x1 = MIN(x1, lua_lcd_width-1);
	x2 = MIN(x2, lua_lcd_width-1);
	y1 = MIN(y1, lua_lcd_height-1);
	y2 = MIN(y2, lua_lcd_height-1);

	if(x1 >= x2 || y1 >= y2)
	{
		LogWriter::LOGX("[disp::drawrect]: range error %d %d %d %d!", x1, y1, x2, y2);
		return PLATFORM_ERR;
	}

	if(lua_lcd_bpp == 16)
	{
		if(-1 == color)
		{
			//画矩形框
			int height = y2 - y1;
			int pixels = height*lua_lcd_width;

			buf += y1*lua_lcd_width;

			for(i = x1; i <= x2; i++)
			{
				buf[i] = disp_color;
				buf[pixels+i] = disp_color;
			}

			for(j = y1; j <= y2; j++)
			{
				buf[x1] = disp_color;
				buf[x2] = disp_color;
				buf += lua_lcd_width;
			}
		}
		else
		{
			//以填充色填充矩形
			buf += y1*lua_lcd_width;

			for(j = y1; j <= y2; j++)
			{
				for(i = x1; i <= x2; i++)
				{
					buf[i] = color;
				}
				buf += lua_lcd_width;
			}
		}
	}
	else if(lua_lcd_bpp == 24)
	{
		if(-1 == color)
		{
			//画矩形框
			int height = y2 - y1;
			int pixels = height*lua_lcd_width;

			buf += y1*lua_lcd_width;

			for(i = x1; i <= x2; i++)
			{
				buf[i] = disp_color;
				buf[pixels+i] = disp_color;
			}

			for(j = y1; j <= y2; j++)
			{
                buf[x1] = disp_color;
				buf[x2] = disp_color;
				buf += lua_lcd_width;
			}
		}
		else
		{
			//以填充色填充矩形
			buf += y1*lua_lcd_width;

			for(j = y1; j <= y2; j++)
			{
				for(i = x1; i <= x2; i++)
				{
					buf[i] = color;
				}
				buf += lua_lcd_width;
			}
		}
	}
	else if(lua_lcd_bpp == 1)
	{
		if(color == COLOR_BLACK_1 || color == COLOR_WHITE_1)
		{
			UINT16 x,y,page;

			if((color^lcd_hwfillcolor) == 0)
			{

				for(x = x1; x < x2; x++)
				{
					for(y = y1;y < y2; y++)
					{
						page = y/8;
						buf[page*lua_lcd_width+x] = 0x00ffffff;
					}
				}
			}
			else
			{
				for(x = x1; x < x2; x++)
				{
					for(y = y1;y < y2; y++)
					{
						page = y/8;
						buf[page*lua_lcd_width+x] = 0x00000000;
					}
				}
			}
		}
		else
		{
			LogWriter::LOGX("[disp::drawrect]: lcd_bpp = 1,color must be balck or white");
		}
	}
	else
	{
		// 暂不支持其他屏
		LogWriter::LOGX("[disp::drawrect]: not support bpp %d", lua_lcd_bpp);
		return PLATFORM_ERR;
	}

	return PLATFORM_OK;
}

INT Disp::putimage(CHAR *filename,
	INT startx, INT starty,	DWORD transcolor,
	INT left, INT top, INT right, INT bottom, INT transtype)
{
	DWORD result;
	HBITMAP hBitmap = NULL;
	HDC hdcMem = NULL;
	BITMAP bitmap = {0,0,0,0,0,0, NULL};
	UINT8 *buf = (UINT8*)gdi_w32_lcd_buffer;
	INT clientWidth, clientHeight, dWidth, dHeight;


	if(strstr(filename,".bmp") || strstr(filename,".BMP"))
	{
		UINT8 *buff;
		FILE *fp;
		UINT32 len;

		fp = fopen(filename, "rb");

		if(NULL == fp)
		{
			LogWriter::LOGX("[putimage]: %s file not exist", filename);
			return PLATFORM_ERR;
		}

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buff = (UINT8*)WinUtil::L_MALLOC(len);
		if(NULL == buff)
		{
			LogWriter::LOGX("[putimage]:not enough memory filename=%s, len=%d.", len);
			fclose(fp);
			return PLATFORM_ERR;
		}

		fread(buff, 1, len, fp);
		fclose(fp);

		result = put_bmp_file_buff(buff, startx, starty, transcolor, left, top, right, bottom);
		WinUtil::L_FREE(buff);
		GetObject(hBitmap, sizeof(BITMAP), &bitmap);

		GetDIBits(lcd_hdcMem,hBitmap,0,(UINT)bitmap.bmHeight,buf,&bitmap_info,DIB_RGB_COLORS);
	}
	else if(strstr(filename,".png") || strstr(filename,".PNG"))
	{
      result = put_png_file_buff(filename,startx,starty,transcolor,left,top,right,bottom,transtype);
	  if(result != PLATFORM_OK)
	  {
		  CHAR buff[128] = {0}, *sub;
		  sprintf(buff, "Load PNG-Image error: ");
		  sub = strrchr(filename, '\\');
		  if(sub == NULL)
		  {
			strcpy(buff + strlen(buff), filename);
		  }
		  else
		  {
			strcpy(buff + strlen(buff), sub+1);
		  }
		  WinAbnormalExit(device.hwnd, buff);
		  return PLATFORM_ERR;
	  }

	  hBitmap = CreateCompatibleBitmap(lcd_hdcMem,bitmap.bmWidth,bitmap.bmHeight);
	  SetBitmapBits(hBitmap,
		  bitmap.bmWidth*bitmap.bmHeight*bitmap.bmWidthBytes,
	  	  bitmap.bmBits);
	  GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	  
	  GetDIBits(lcd_hdcMem,hBitmap,0,(UINT)bitmap.bmHeight,buf,&bitmap_info,DIB_RGB_COLORS);
	}
	else
	{
		MessageBox(device.hwnd, "Load Unknown Image!", "WatchDebug", MB_OK | MB_ICONWARNING);
		PostMessage(device.hwnd, WM_CLOSE, 0, 0);
		return PLATFORM_ERR;
	}

	clientWidth = skin_lcd_width - startx;
	clientHeight = skin_lcd_height - starty;
	dWidth = bitmap.bmWidth > clientWidth ? clientWidth : bitmap.bmWidth;  
	dHeight = bitmap.bmHeight > clientHeight ? clientHeight : bitmap.bmHeight;

	hdcMem = CreateCompatibleDC(lcd_hdcMem);
	SelectObject(hdcMem,hBitmap);
	BitBlt(lcd_hdcMem, startx, starty, dWidth, dHeight, hdcMem, 0, 0, SRCCOPY);

	if(bitmap.bmBits != NULL)WinUtil::L_FREE(bitmap.bmBits);
	if(hBitmap != NULL) DeleteObject(hBitmap);
	if(hdcMem != NULL) DeleteDC(hdcMem);

	return PLATFORM_OK;
}



void Disp::DumpScreen2RGB(ColorMapObject *ColorMap,
			   GifRowType *ScreenBuffer,
			   int ScreenWidth, int ScreenHeight)
{
    int i, j;
    GifRowType GifRow;
    GifColorType *ColorMapEntry;

    unsigned char *Buffer, *BufferP;

	Buffer = gdi_w32_lcd_buffer;
	BufferP = Buffer;
    for (i = 0; i < ScreenHeight; i++) {
        GifRow = ScreenBuffer[i];
		BufferP = Buffer + i * skin_lcd_width * 4;
        for (j = 0; j < ScreenWidth; j++) {
            ColorMapEntry = &ColorMap->Colors[GifRow[j]];
            *BufferP++ = ColorMapEntry->Blue;
            *BufferP++ = ColorMapEntry->Green;
            *BufferP++ = ColorMapEntry->Red;
			*BufferP++ = 0xff;
        }
    }

	update();
}

void Disp::playgif(const char* gif_file_name, int x, int y,  int times)
{
    int	i, j, Size, Row, Col, Width, Height, ExtCode, Count;
    GifRecordType RecordType;
    GifByteType *Extension;
    GifRowType *ScreenBuffer;
    GifFileType *GifFile;
    int
	InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
	InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
    int ImageNum = 0;
    ColorMapObject *ColorMap;
    int Error;

	if ((GifFile = DGifOpenFileName(gif_file_name, &Error)) == NULL) {
		LogWriter::LOGX("[platform_disp] playgif Error=%d.", Error);
		return;
	}

    if (GifFile->SHeight == 0 || GifFile->SWidth == 0) {
		LogWriter::LOGX("[platform_disp] playgif Image of width or height 0.");
		return;
    }

    /* 
     * Allocate the screen as vector of column of rows. Note this
     * screen is device independent - it's the screen defined by the
     * GIF file parameters.
     */
    if ((ScreenBuffer = (GifRowType *)
		malloc(GifFile->SHeight * sizeof(GifRowType))) == NULL)
	{
	    LogWriter::LOGX("[platform_disp] playgif Failed to allocate memory required, aborted.");
		return;
	}

    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((ScreenBuffer[0] = (GifRowType) malloc(Size)) == NULL) /* First row. */
	{
		LogWriter::LOGX("[platform_disp] playgif Failed to allocate memory required, aborted.");
		return;
	}
	

    for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;
    for (i = 1; i < GifFile->SHeight; i++)
	{
		/* Allocate the other rows, and set their color to background too: */
		if ((ScreenBuffer[i] = (GifRowType) malloc(Size)) == NULL)
		{
			LogWriter::LOGX("[platform_disp] playgif Failed to allocate memory required, aborted.");
			return;
		}
		memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
    }

	bIsExitThread = FALSE;

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			LogWriter::LOGX("[platform_disp] playgif[Scan] Error=%d.", Error);
			return;
		}

		switch (RecordType) {
		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
				LogWriter::LOGX("[platform_disp] playgif[RecordType] Error=%d.", GifFile->Error);
				return;
			}
			Row = GifFile->Image.Top; /* Image Position relative to Screen. */
			Col = GifFile->Image.Left;
			Width = GifFile->Image.Width;
			Height = GifFile->Image.Height;
			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
			   GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
				LogWriter::LOGX("[platform_disp] playgif Image %d is not confined to screen dimension, aborted.",ImageNum);
				return;
			}
			if (GifFile->Image.Interlace) {
				/* Need to perform 4 passes on the images: */
				for (Count = i = 0; i < 4; i++)
				for (j = Row + InterlacedOffset[i]; j < Row + Height;
							 j += InterlacedJumps[i]) {
					if (DGifGetLine(GifFile, &ScreenBuffer[j][Col],
					Width) == GIF_ERROR) {
					LogWriter::LOGX("[platform_disp] playgif[Interlace=1, DGifGetLine] Error=%d.", GifFile->Error);
					return;
					}
				}
			}
			else {
				for (i = 0; i < Height; i++) {
				if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col],
					Width) == GIF_ERROR) {
					LogWriter::LOGX("[platform_disp] playgif[Interlace=0, DGifGetLine] Error=%d.", GifFile->Error);
					return;
				}
				}
			}

			ColorMap = (GifFile->Image.ColorMap
				? GifFile->Image.ColorMap
				: GifFile->SColorMap);
			if (ColorMap == NULL) {
				LogWriter::LOGX("[platform_disp] playgif Gif Image does not have a colormap.");
				return;
			}

			DumpScreen2RGB(ColorMap,
				ScreenBuffer, 
				GifFile->SWidth, GifFile->SHeight);
			Sleep(300);
			break;

		case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
				LogWriter::LOGX("[platform_disp] playgif[DGifGetExtension] Error=%d.", GifFile->Error);
				return;
			}
			while (Extension != NULL) {
				if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
				LogWriter::LOGX("[platform_disp] playgif[DGifGetExtensionNext] Error=%d.", GifFile->Error);
				return;
				}
			}
			break;
		case TERMINATE_RECORD_TYPE:
			times --;
			break;

		default:		    /* Should be trapped by DGifGetRecordType. */
			break;
		}
    } while (RecordType != TERMINATE_RECORD_TYPE && bIsExitThread == FALSE && times > 0);

    (void)free(ScreenBuffer);
	bIsExitThread = TRUE;

    if (DGifCloseFile(GifFile, &Error) == GIF_ERROR) {
		LogWriter::LOGX("[platform_disp] playgif[DGifCloseFile] Error=%d.", GifFile->Error);
		return;
    }
}

void Disp::stopgif(void)
{
	bIsExitThread = true;
	hdcClear();
	LogWriter::LOGX("[platform_disp] stopgif");
}

UINT16* Disp::puttext(CHAR *text, INT startx, INT starty)
{
    int len = strlen(text);
    const UINT8 *pText = (const UINT8 *)text;
    int i;
    DispBitmap bitmap;
    UINT16 charcode,cutlr=0, offset[2]={0};
    UINT16 display_x, display_y;
    
    if(startx >= dispInitParam.width)
        startx = 0;

    if(starty >= dispInitParam.height)
        starty = 0;

    for(i = 0; i < len; )
    {
        /*
          0x0D -- '\r'
          0x0A -- '\n'
        */
        if(pText[i] == 0x0D && pText[i+1] == 0x0A){
            i+=2; offset[1]++;
            if(cutlr == 1)
              { cutlr++; goto next_char; }
            else
              { goto next_line; }
        } else if(pText[i] == 0x0D  || pText[i] == 0x0A) {
            i++;
            if(cutlr == 1)
              { cutlr++; goto next_char; }
            else
              { goto next_line; }
        }
        cutlr=0;
        if(pText[i] > 0x81)
        {
            if(1)//(pText[i+1]&0x80)
            {
                // gb2312 chinese char
                charcode = pText[i]<<8 | pText[i+1];
                i += 2;
            }
            else
            {
                charcode = '?';
                i += 1;
            }
        }
        else
        {
            // ascii char
            charcode = pText[i];
            i += 1;
        }

        getCharBitmap(&bitmap, charcode);
        
        display_x = startx;
        display_y = starty;
  
        if(dispInitParam.bpp == 1)
        {
            if(bitmap.width == 14 && bitmap.height == 14)
            {
                bitmap.width = 16;
                bitmap.height = 16;
                display_x = startx+1;
                display_y = starty+1;
            }
        }

        if(display_x + bitmap.width > dispInitParam.width)
        {
            display_y += bitmap.height;
            display_x = 0;        
        }

        if(dispInitParam.bpp == 1)
            charBitmap_bpp1(&bitmap, display_x, display_y);
        else if(dispInitParam.bpp == 16)
            charBitbmp_bpp16(&bitmap, display_x, display_y);
        else if(dispInitParam.bpp == 24)
            charBitbmp_bpp24(&bitmap, display_x, display_y);

        startx = display_x;
        starty = display_y;
        
        if(startx + bitmap.width >= dispInitParam.width) {
           cutlr=1;
           goto next_line;
        } else {
           startx += bitmap.width;
           goto next_char;
        }
next_line:   
        // 自动换行显示
        starty += bitmap.height;
        startx = 0;
      
next_char:
        //自动回显
        if( starty >= device.main_lcd_height) {
            starty = 0;
            break;
        }
    }
    offset[0] = i;

    return offset;
}

void Disp::platform_lcd_powersave(int sleep_wake)
{   
    if(sleep_wake)
    {
        mainlcd_backlight_level = g_mainlcd_backlight_level_min;
    }
    else
    {
        mainlcd_backlight_level = g_mainlcd_backlight_level_max;
    }
	update();
}


void Disp::put_qr_code_buff(unsigned char* buff, int width)
{
	int i;

	UINT32 margin;
	UINT32 size;
#if defined(_WIN32)
	UINT32 pixel_size = 4;
#else
	UINT32 pixel_size = lua_lcd_bpp / 8;
#endif

	unsigned char *row, *p, *q;
	int x, y, xx, yy;
	int realwidth;

	int height = 0;
	UINT8* wbuff = (UINT8*)gdi_w32_lcd_buffer;
	int pos;

	size = lua_lcd_width  / width;
	margin = (lua_lcd_width % width ) /2;

	realwidth = lua_lcd_width;


	row = (unsigned char *)WinUtil::L_MALLOC(realwidth * pixel_size);

	/* top margin */
	memset(row, 0xff, realwidth * pixel_size );

	for(y=0; y< margin ; y++) {
		pos = height * lua_lcd_width * pixel_size;
		for(i=0; i<realwidth; i++)
		{
			wbuff[pos++] = row[i*pixel_size+0];
			wbuff[pos++] = row[i*pixel_size+1];
			wbuff[pos++] = row[i*pixel_size+2];
			wbuff[pos++] = 0x00;
		}
		height++;
	}

	/* data */
	p = buff;
	for(y=0; y< width; y++) {

		memset(row, 0xff, (realwidth* pixel_size));
		q = row;
		q += margin * pixel_size ;

		for(x=0; x < width; x++) {
			for(xx=0; xx<size; xx++) {
				if((*p & 1))
					memset(q, 0, pixel_size);
				q += pixel_size;
			}
			p++;
		}
		for(yy=0; yy<size; yy++) {
			pos = height * lua_lcd_width * pixel_size;
			for(i=0; i<realwidth; i++)
			{
				wbuff[pos++] = row[i*pixel_size+0];
				wbuff[pos++] = row[i*pixel_size+1];
				wbuff[pos++] = row[i*pixel_size+2];
				wbuff[pos++] = 0x00;
			}
			height++;
		}
	}
	/* bottom margin */
	memset(row, 0xff, (realwidth* pixel_size));

	for(y=0; y<margin; y++) {
		pos = height * lua_lcd_width * pixel_size;
		for(i=0; i<realwidth; i++)
		{
			wbuff[pos++] = row[i*pixel_size+0];
			wbuff[pos++] = row[i*pixel_size+1];
			wbuff[pos++] = row[i*pixel_size+2];
			wbuff[pos++] = 0x00;
		}
		height++;
	}

	WinUtil::L_FREE(row);
}

void Disp::poweroff(void)
{
	PostMessage(GetDispWnd(), WM_COMMAND, IDM_FILE_EXIT, NULL);
}

void Disp::platform_layer_hang_stop(void)
{
	BOOL dealBool = FALSE;
	update();
}