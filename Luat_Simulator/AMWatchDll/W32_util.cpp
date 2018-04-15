#include "stdafx.h"
#include "W32_util.h"

DWORD WinUtil::rgb565_to_rgb888(DWORD p)
{
	/*----------------------------------------------------------------*/
	/* Local Variables                                                */
	/*----------------------------------------------------------------*/   
	DWORD     result;

	/*----------------------------------------------------------------*/
	/* Code Body                                                      */
	/*----------------------------------------------------------------*/
	/* extend p's B[4:0] to result B[7:0] */
	result = B_OF_RGB565(p) << (0 + 3);

	/* extend p's G[10:5] to result G[15:8] */
	result |= G_OF_RGB565(p) << (8 + 2);

	/* extend p's R[15:11] to result R[23:16] */
	result |= R_OF_RGB565(p) << (16 + 3);

	return result;
}

DWORD WinUtil::rgb888_to_rgb565(DWORD p)
{
	/*----------------------------------------------------------------*/
	/* Local Variables                                                */
	/*----------------------------------------------------------------*/   
	UINT32     result;

	/*----------------------------------------------------------------*/
	/* Code Body                                                      */
	/*----------------------------------------------------------------*/
	/* reduce p's B[7:0] to result[4:0] */
	result = B_OF_RGB888(p) >> 3;

	/* reduce p's G[15:8] to result[10:5] */
	result |= (G_OF_RGB888(p) >> 2) << 5;

	/* reduce p's R[15:8] to result[15:11] */
	result |= (R_OF_RGB888(p) >> 3) << 11;

	return result;
}

color WinUtil::rgb_to_color(DWORD p)
{
	color c;

	if(p > COLOR_MAX_VALUE)
	{
		//Invalid parameter.
		p = COLOR_MAX_VALUE;
	}

	c.r = R_OF_RGB888(p);
	c.g = G_OF_RGB888(p);
	c.b = B_OF_RGB888(p);

	return c;
}

DWORD WinUtil::ust_get_current_time()
{
	return GetTickCount(); //ms
}

UINT WinUtil::_hex2int(CHAR c)
{
	if (c >= '0' && c <= '9')
		return (UINT)(c-'0');
	if (c >= 'a' && c <= 'f')
		return (UINT)(c-'a'+10);
	if (c >= 'A' && c <= 'F')
		return (UINT)(c-'A'+10);

	return 0;
}

UINT WinUtil::atohex(CHAR *in)
{
	UINT value = 0;
	if (in[0]=='0' && (in[1]=='x' || in[1]=='X'))
	{
		in = in+2;
		while (*in != 0)
		{
			value = value*16 + _hex2int(*in);
			in++;
		}
	}
	return value;
}

void WinUtil::ParseSizeString(CHAR* szIn, INT lengthIn, SIZE* prectOut)
{
	CHAR szTemp[8];
	CHAR* pTemp=NULL;
	INT nLayout[2];
	INT i=0, j=0;

	memset(szTemp, 0, 8);
	pTemp = szTemp;

	for (i=0; i < lengthIn; i++)
	{
		char c=szIn[i];
		if(c == '(')
			continue;
		else if(c == ')')
		{
			*pTemp = '\0';
			nLayout[j++] = atoi(szTemp);
			break;
		}
		else if(c == ',')
		{
			*pTemp = '\0';
			nLayout[j++] = atoi(szTemp);
			memset(szTemp, 0, 8);
			pTemp = szTemp;
		}
		else
		{
			*pTemp++ = c;
		}
	}

	prectOut->cx = nLayout[0];
	prectOut->cy = nLayout[1];
}


void WinUtil::ParseLayoutString(CHAR* szIn, INT lengthIn, RECT* prectOut)
{
	CHAR szTemp[8];
	CHAR* pTemp=NULL;
	INT nLayout[4];
	INT i=0, j=0;

	memset(szTemp, 0, 8);
	pTemp = szTemp;

	for (i=0; i < lengthIn; i++)
	{
		char c=szIn[i];
		if(c == '(')
			continue;
		else if(c == ')')
		{
			*pTemp = '\0';
			nLayout[j++] = atoi(szTemp);
			break;
		}
		else if(c == ',')
		{
			*pTemp = '\0';
			nLayout[j++] = atoi(szTemp);
			memset(szTemp, 0, 8);
			pTemp = szTemp;
		}
		else
		{
			*pTemp++ = c;
		}
	}

	prectOut->left = nLayout[0];
	prectOut->top = nLayout[1];
	prectOut->right = nLayout[0] + nLayout[2];
	prectOut->bottom = nLayout[1] + nLayout[3];
}

void WinUtil::separator_strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl)
{ 
	char* pi = pSrcIn; 
	char* po = pDstOut; 

	size_t nSrcRplLen = strlen(pSrcRpl); 
	size_t nDstRplLen = strlen(pDstRpl); 

	char *p = NULL; 
	intptr_t nLen = 0; 

	do 
	{
		p = strstr(pi, pSrcRpl); 

		if(p != NULL) 
		{ 
			nLen = p - pi; 
			memcpy(po, pi, nLen);

			memcpy(po + nLen, pDstRpl, nDstRplLen); 
		} 
		else 
		{ 
			strcpy(po, pi); 

			break;
		} 

		pi = p + nSrcRplLen; 
		po = po + nLen + nDstRplLen;    
	} while (p != NULL); 
}

int WinUtil::trim(char s[])
{
	int n;
	for (n = (int)strlen(s)-1; n >= 0; n--)
		if (s[n] != ' ' && s[n] != '\r' && s[n] != '\n')
			break;
	s[n+1] = '\0';
	return n;
}

void* WinUtil::L_MALLOC(INT size)
{
	return malloc(size);
}

void WinUtil::L_FREE(void *mem)
{
	free(mem);
}

void* WinUtil::L_CALLOC(unsigned num, unsigned size)
{
	return calloc(num, size);
}

void* WinUtil::L_REALLOC(void* memblock, unsigned size)
{
	return realloc(memblock, size);
}