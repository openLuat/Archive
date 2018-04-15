/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    LogWriter.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/12
 *
 * Description:
 *          Implement 'LogWriter' class.
 * History:
 *     panjun 16/10/12 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "share.h"

#define MAXFRACT     10000
#define NumFract        4
#define LOG_POOL_LENGTH 2048

static CHAR logpool[LOG_POOL_LENGTH] = {0};
LogWriter* LogWriter::theOnlyLogWriter = NULL;

LogWriter::LogWriter()
{
	// determine the log file's name, yyyyMMddhhmmss.log
	SYSTEMTIME st;
	::GetLocalTime(&st);
	::sprintf_s(this->fileName,1024,"AMLuaDebug.log", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	fp = ::_fsopen(this->fileName, "w", _SH_DENYNO); // Permits read and write access
	memset(logpool, 0, sizeof(logpool));
}

void itof(char **buf, int i)
{
	char *s;
	#define LEN	20
	int rem, j;
	static char rev[LEN+1];

	rev[LEN] = 0;
	s = &rev[LEN];
	for (j= 0 ; j < NumFract ; j++)
	{
		rem = i % 10;
		*--s = rem + '0';
		i /= 10;
	}
	while (*s)
	{
		(*buf)[0] = *s++;
		++(*buf);
	}
}

void itoa(CHAR **buf, int val, int radix)
{
	CHAR *str;

	str = _itoa(val, *buf, radix);
	(*buf) += strlen(str);
}

void LogWriter::dbg_print(CHAR *buf, char *fmt, va_list ap)
{
	double dval;
	int ival;
	char *p, *sval;
	char *bp, cval;
	int fract;

	bp= buf;
	*bp= 0;

	for (p= fmt; *p; p++)
	{
		if (*p != '%')
		{
			*bp++= *p;
			continue;
		}
		switch (*++p) {
		case 'd':
			ival= va_arg(ap, int);
			if (ival < 0){
				*bp++= '-';
				ival= -ival;
			}
			itoa(&bp, ival, 10);
			break;

		case 'o':
			ival= va_arg(ap, int);
			if (ival < 0){
				*bp++= '-';
				ival= -ival;
			}
			*bp++= '0';
			itoa(&bp, ival, 8);
			break;

		case 'x':
			ival= va_arg(ap, int);
			if (ival < 0){
				*bp++= '-';
				ival= -ival;
			}
			*bp++= '0';
			*bp++= 'x';
			itoa(&bp, ival, 16);
			break;

		case 'c':
			cval= va_arg(ap, int);
			*bp++= cval;
			break;

		case 'f':
			dval= va_arg(ap, double);
			if (dval < 0){
				*bp++= '-';
				dval= -dval;
			}
			if (dval >= 1.0)
				itoa(&bp, (int)dval, 10);
			else
				*bp++= '0';
			*bp++= '.';
			fract= (int)((dval- (double)(int)dval)*(double)(MAXFRACT));
			itof(&bp, fract);
			break;

		case 's':
			for (sval = va_arg(ap, char *) ; *sval ; sval++ )
				*bp++= *sval;
			break;
		}
	}
	*bp= 0;
}

void LogWriter::LOGX(CHAR *fmt,...)
{
	va_list ap;
	CHAR buffer[1024];
	SYSTEMTIME st;
	va_start (ap, fmt);
	::GetLocalTime(&st);
	memset(buffer, 0, sizeof(buffer));
	sprintf_s(buffer,1024,"%04d-%02d-%02d %02d:%02d:%02d ", \
			  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	GetLogWriter()->dbg_print(buffer+strlen(buffer), fmt, ap);
	strcat_s(buffer,sizeof(buffer),"\n");

	if(LOG_POOL_LENGTH - strlen(logpool) - 1 > strlen(buffer))
	{
		strcat_s(logpool+strlen(logpool), sizeof(logpool), buffer);
	} else {
		LogWriter::GetLogWriter()->Log(logpool);
		memset(logpool, 0, sizeof(logpool));
		strcat_s(logpool, buffer);
	}

	va_end(ap);
}

void LogWriter::Log(CHAR* log)
{
	if (fp != NULL)
	{
		::fprintf(this->fp, "%s", log);
		::fflush(this->fp);
	}
}
LogWriter::~LogWriter()
{
	if (this->fp != NULL)
	{
		GetLogWriter()->Log(logpool);
		::fclose(this->fp);
	}
}

LogWriter* LogWriter::GetLogWriter()
{
	if (theOnlyLogWriter == NULL)
	{
		theOnlyLogWriter = new LogWriter();
	}
	return theOnlyLogWriter;
}

void LogWriter::DeleteLogWriter()
{
	if (theOnlyLogWriter != NULL)
	{
		delete theOnlyLogWriter;
		theOnlyLogWriter = NULL;
	}
}