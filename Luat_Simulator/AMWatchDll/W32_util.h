#ifndef __W32_UTIL_H__
#define __W32_UTIL_H__

#include "LogWriter.h"

#define MAX_LOADSTRING 100

#ifndef MAX
	#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define COLOR_MAX_VALUE (0xFFFFFF)

/* ARGB8888 */
#define R_OF_RGB888(c)                  (((c) <<  8) >> 24)
#define G_OF_RGB888(c)                  (((c) << 16) >> 24)
#define B_OF_RGB888(c)                  (((c) << 24) >> 24)
#define A_OF_ARGB8888(c)                ((c) >> 24)

/* RGB565 */
#define R_OF_RGB565(p)      ((p) >> 11)
#define G_OF_RGB565(p)      (((p) << 21) >> 26)
#define B_OF_RGB565(p)      (((p) << 27) >> 27)

typedef struct {
	BYTE r;
	BYTE g;
	BYTE b;
}color;

/*Safely delete a DC*/
#define MTKDeleteDC(hdc) \
	if(NULL != hdc)\
	{\
	DeleteDC(hdc);\
	hdc = NULL;\
	}\
	\

class WinUtil {
public:
	static DWORD rgb565_to_rgb888(DWORD p);
	static DWORD rgb888_to_rgb565(DWORD p);
	static color rgb_to_color(DWORD p);
	static DWORD ust_get_current_time();
	static UINT atohex(CHAR *in);
	static void ParseSizeString(CHAR* szIn, INT lengthIn, SIZE* prectOut);
	static void ParseLayoutString(CHAR* szIn, INT lengthIn, RECT* prectOut);
	static void separator_strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl);
	static int trim(char s[]);
	static void* L_MALLOC(INT size);
	static void L_FREE(void *mem);
	static void* L_CALLOC(unsigned num, unsigned size);
	static void* L_REALLOC(void* memblock, unsigned size);
private:
	static UINT _hex2int(CHAR c);
};

#define READ_NET_DATA16(buffer, pos) \
	(((UINT16)buffer[pos+1]) | (((UINT16)buffer[pos  ]) << 8))
#define READ_NET_DATA32(buffer, pos) \
	(((UINT32)buffer[pos+3]) | (((UINT32)buffer[pos+2]) << 8) | (((UINT32)buffer[pos+1]) << 16) | (((UINT32) buffer[pos])) << 24)

#define NET_STORE_LEN(buffer, value) \
	*(buffer++) = value >> 8; \
	*(buffer++) = value & 0x00ff; \

#define NET_STORE_ID(buffer, value) \
	*(buffer++) = value; \

#define NET_STORE_DATA(buffer, value) \
	*(buffer++) = value; \

#endif //__W32_UTIL_H__