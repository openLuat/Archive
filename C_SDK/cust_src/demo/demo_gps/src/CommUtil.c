#include <string.h>
#include <stdlib.h>

#include "CommUtil.h"


#define UTIL_MAX_STR_LENGTH 256


//---------------------------------------------------------------------------
GBOOL IsHex(const GCHAR* szStr)
{
  int i4Length;
  GINT32 i;

  i4Length = strlen(szStr);
  if ((i4Length == 0) || (i4Length > UTIL_MAX_STR_LENGTH))
  {
    return false;
  }

  for (i = 0; i < i4Length; i++)
  {
    if (!(((szStr[i] >= '0') && (szStr[i] <= '9')) ||
         ((szStr[i] >= 'a') && (szStr[i] <= 'f')) ||
         ((szStr[i] >= 'A') && (szStr[i] <= 'F'))))
    {
        return false;
    }
  }

  return true;
}

//---------------------------------------------------------------------------
GBOOL IsInteger(GCHAR* szStr)
{
  int i4Length;
  GINT32 i;


  i4Length = strlen(szStr);
  if ((i4Length == 0) || (i4Length > UTIL_MAX_STR_LENGTH))
  {
    return false;
  }

  for (i = 0; i < i4Length; i++)
  {
    if (szStr[i] == '-')
    {
      if (i != 0)
      {
        return false;
      }
    }
    else if ((szStr[i] < '0') || (szStr[i] > '9'))
    {
        return false;
    }
  }

  return true;
}

//---------------------------------------------------------------------------
GBOOL IsFloat(GCHAR* szStr)
{
  int i4Length;
  GINT32 i;
  GINT32 CommaCnt = 0;


  i4Length = strlen(szStr);
  if ((i4Length == 0) || (i4Length > UTIL_MAX_STR_LENGTH))
  {
    return false;
  }

  for (i = 0; i < i4Length; i++)
  {
    if (szStr[i] == '-')
    {
      if (i != 0)
      {
        return false;
      }
    }

    // GFLOAT has at most 1 comma
    else if(szStr[i] == '.')
    {
      CommaCnt++;
      if (CommaCnt > 1)
      {
        return false;
      }
    }

    else if ((szStr[i] < '0') || (szStr[i] > '9'))
    {
      return false;
    }
  }

  return true;
}


//---------------------------------------------------------------------------
unsigned long u4HexCharToDecInt(char w)
{
  unsigned long ret_value;

  if ((w >= '0') && (w <= '9'))
  {
    ret_value = w - '0';
  }
  else if ((w >= 'A') && (w <= 'F'))
  {
    ret_value = w - 'A' + 10;
  }
  else if ((w >= 'a') && (w <= 'f'))
  {
    ret_value = w - 'a' + 10;
  }
  else
  {
    ret_value = 0;
  }

  return ret_value;
}


//---------------------------------------------------------------------------
unsigned long u4HexToDecInt(const char *szHex, short int i2Size)
{
  int i;
  const char *pChar;
  unsigned long u4Int;


  u4Int = 0;
  pChar = szHex;
  for (i = 0; i < i2Size; i++)
  {
    if (*pChar == 0x00)
    {
      break;
    }
    u4Int = (u4Int << 4);
    u4Int += u4HexCharToDecInt(*pChar++);
  }

  return u4Int;
}


#define is_space(c) ((c) == ' ')
#define is_digit(c) ((c) >= '0' && (c) <= '9')
GDOUBLE commUtilAtoF(const char* s)
{
  double power,value;  
  int i,sign;  

  if(!s) return 0;
  for(i=0;is_space(s[i]);i++);//除去字符串前的空格  
  
  sign=(s[i]=='-')?-1:1;  
  if(s[i]=='-'||s[i]=='+')//要是有符号位就前进一位  
      i++;  
  for(value=0.0;is_digit(s[i]);i++)
  {
      value=value*10.0+(s[i]-'0');
  }
  if(s[i]=='.')  
      i++;  
  for(power=1.0;is_digit(s[i]);i++)//计算小数点后的数字  
  {  
      value=value*10.0+(s[i]-'0');  
      power*=10.0;  
  }  
  return sign*value/power;    
}


unsigned short commUtilFormatFloat(char* buf, unsigned short buflen, const double n, unsigned char prec)
{
  int offset = 0, precCount = 1, num = 0;
  memset(buf, 0, buflen);

  while(prec > 0)
  {
    precCount *= 10;
    prec--;
  }

  num = (int)n;
  
  offset = snprintf(buf, buflen, "%d", num);

  num = (int)((double)(n - (int)n)*precCount);

  offset += snprintf(buf + offset, buflen - offset, ".%d", num);

  return offset;
}