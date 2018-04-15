/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    LogWriter.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/12
 *
 * Description:
 *          Define 'LogWriter' class.
 * History:
 *     panjun 16/10/12 Initially create file.
 **************************************************************************/
#ifndef __LOG_WRITER_H__
#define __LOG_WRITER_H__

class LogWriter
{
protected:
	LogWriter();
	~LogWriter();
private:
	CHAR fileName[1024];
	FILE* fp;	
public:
	void dbg_print(CHAR *buf, CHAR *fmt, va_list ap);
	void Log(CHAR* log);
public:
	static void LOGX(CHAR *fmt,...);
	static LogWriter* GetLogWriter();
	static void DeleteLogWriter();
private:
	static LogWriter* theOnlyLogWriter;
};

#define LOG(fmt,x) \
	LogWriter::LOGX(fmt,x);

#define ENDLOG \
	LogWriter::DeleteLogWriter();

#endif //__LOG_WRITER_H__
