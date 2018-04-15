/*
 * Copyright (c) 2007-2016, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file os_port.c
 *
 * OS specific functions.
 */
#include "os_port.h"

#ifdef WIN32
/**
 * gettimeofday() not in Win32 
 */
EXP_FUNC void STDCALL gettimeofday(struct timeval* t, void* timezone)
{       
#if defined(_WIN32_WCE)
    t->tv_sec = time(NULL);
    t->tv_usec = 0;                         /* 1sec precision only */ 
#else
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    t->tv_sec = (long)timebuffer.time;
    t->tv_usec = 1000 * timebuffer.millitm; /* 1ms precision */
#endif
}

/**
 * strcasecmp() not in Win32
 */
EXP_FUNC int STDCALL strcasecmp(const char *s1, const char *s2)
{
    while (tolower(*s1) == tolower(*s2++))
    {
        if (*s1++ == '\0')
        {
            return 0;
        }
    }

    return *(unsigned char *)s1 - *(unsigned char *)(s2 - 1);
}


EXP_FUNC int STDCALL getdomainname(char *buf, int buf_size)
{
    HKEY hKey;
    unsigned long datatype;
    unsigned long bufferlength = buf_size;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters"),
                        0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return -1;

    RegQueryValueEx(hKey, "Domain", NULL, &datatype, buf, &bufferlength);
    RegCloseKey(hKey);
    return 0; 
}
#endif


static const short lmos[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
static const short mos[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
#define MONTAB(year) ((year & 0x03) == 2 ? lmos : mos)

time_t OS_TimeToUTCTamp(struct tm *_timeptr)
{
	int year, days, secs;

	year = _timeptr->tm_year - 70;
	days = (year + 1) / 4 + 365 * year;
	days += MONTAB(year)[_timeptr->tm_mon] + _timeptr->tm_mday - 1;

	secs = 3600 * _timeptr->tm_hour + 60 * _timeptr->tm_min + _timeptr->tm_sec;

	return (86400 * days + secs);
}

time_t time(time_t *_timer)
{
	unsigned int year, days, secs;
	T_AMOPENAT_SYSTEM_DATETIME DT;
	iot_os_get_system_datetime(&DT);
	if (DT.nYear < 2000)
	{
		DT.nYear += 2000;
	}
	year = DT.nYear - 1970;
	days = (year + 1) / 4 + 365 * year;
	days += MONTAB(year)[DT.nMonth - 1] + DT.nDay - 1;

	secs = 3600 * DT.nHour + 60 * DT.nMin + DT.nSec;
	if (_timer)
	{
		*_timer = 86400 * days + secs;
	}
	return (86400 * days + secs);
}

void* calloc(size_t __nmemb, size_t __size)
{
	void *P = iot_os_malloc(__nmemb * __size);
	if (P)
	{
		memset(P, 0, __nmemb * __size);
		return P;
	}
	else
	{
		return NULL;
	}
}

uint64_t __be64_to_cpu(uint64_t x)
{
	uint8_t *Org = (uint8_t *)&x;
	uint64_t Result;
	uint8_t *Temp;
	int i;
	Temp = (uint8_t *)&Result;
	for (i = 0; i < 8; i++)
	{
		Temp[i] = Org[7 - i];
	}
	return Result;
}

int gettimeofday(struct timeval *__p, void *__tz)
{
	time(&__p->tv_sec);
	__p->tv_usec = 0;
	return __p->tv_sec;
}
