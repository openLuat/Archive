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
 * @file os_port.h
 *
 * Some stuff to minimise the differences between windows and linux/unix
 */

#ifndef HEADER_OS_PORT_H
#define HEADER_OS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os_int.h"
#include "tls_config.h"
#include "iot_os.h"
#include "string.h"
#include "stdarg.h"
#include "iot_socket.h"
#include "iot_debug.h"
#define __AIR202__
#if defined(WIN32)
#define STDCALL                 __stdcall
#define EXP_FUNC                __declspec(dllexport)
#else
#define STDCALL
#define EXP_FUNC
#endif

#if defined(_WIN32_WCE)
#undef WIN32
#define WIN32
#endif

#ifdef WIN32

/* Windows CE stuff */
#if defined(_WIN32_WCE)
#include <basetsd.h>
#define abort()                 exit(1)
#else
#include <io.h>
#include <process.h>
#include <sys/timeb.h>
#include <fcntl.h>
#endif      /* _WIN32_WCE */

#include <winsock.h>
#include <direct.h>
#undef getpid
#undef open
#undef close
#undef sleep
#undef gettimeofday
#undef dup2
#undef unlink

#define SOCKET_READ(A,B,C)      recv(A,B,C,0)
#define SOCKET_WRITE(A,B,C)     send(A,B,C,0)
#define SOCKET_CLOSE(A)         closesocket(A)
#define srandom(A)              srand(A)
#define random()                rand()
#define getpid()                _getpid()
#define snprintf                _snprintf
#define open(A,B)               _open(A,B)
#define dup2(A,B)               _dup2(A,B)
#define unlink(A)               _unlink(A)
#define close(A)                _close(A)
#define read(A,B,C)             _read(A,B,C)
#define write(A,B,C)            _write(A,B,C)
#define sleep(A)                Sleep(A*1000)
#define usleep(A)               Sleep(A/1000)
#define strdup(A)               _strdup(A)
#define chroot(A)               _chdir(A)
#define chdir(A)                _chdir(A)
#define alloca(A)               _alloca(A)
#ifndef lseek
#define lseek(A,B,C)            _lseek(A,B,C)
#endif

/* This fix gets around a problem where a win32 application on a cygwin xterm
   doesn't display regular output (until a certain buffer limit) - but it works
   fine under a normal DOS window. This is a hack to get around the issue - 
   see http://www.khngai.com/emacs/tty.php  */
#define TTY_FLUSH()             if (!_isatty(_fileno(stdout))) fflush(stdout);

/*
 * automatically build some library dependencies.
 */
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "AdvAPI32.lib")

typedef int socklen_t;

EXP_FUNC void STDCALL gettimeofday(struct timeval* t,void* timezone);
EXP_FUNC int STDCALL strcasecmp(const char *s1, const char *s2);
EXP_FUNC int STDCALL getdomainname(char *buf, int buf_size);

#else   /* Not Win32 */

#ifdef __AIR202__
#define TTY_FLUSH()
#else
#define SOCKET_READ(A,B,C)      read(A,B,C)
#define SOCKET_WRITE(A,B,C)     write(A,B,C)
#define SOCKET_CLOSE(A)         if (A >= 0) close(A)
#define TTY_FLUSH()
#endif

#ifndef be64toh
#define be64toh(x) __be64_to_cpu(x)
#endif

#endif  /* Not Win32 */

/* some functions to mutate the way these work */
EXP_FUNC int STDCALL ax_open(const char *pathname, int flags); 

#ifdef CONFIG_PLATFORM_LINUX
void exit_now(const char *format, ...) __attribute((noreturn));
#else
void exit_now(const char *format, ...);
#endif

/* Mutexing definitions */
#if defined(CONFIG_SSL_CTX_MUTEXING)
#if defined(WIN32)
#define SSL_CTX_MUTEX_TYPE          HANDLE
#define SSL_CTX_MUTEX_INIT(A)       A=CreateMutex(0, FALSE, 0)
#define SSL_CTX_MUTEX_DESTROY(A)    CloseHandle(A)
#define SSL_CTX_LOCK(A)             WaitForSingleObject(A, INFINITE)
#define SSL_CTX_UNLOCK(A)           ReleaseMutex(A)
#else 
#include <pthread.h>
#define SSL_CTX_MUTEX_TYPE          pthread_mutex_t
#define SSL_CTX_MUTEX_INIT(A)       pthread_mutex_init(&A, NULL)
#define SSL_CTX_MUTEX_DESTROY(A)    pthread_mutex_destroy(&A)
#define SSL_CTX_LOCK(A)             pthread_mutex_lock(&A)
#define SSL_CTX_UNLOCK(A)           pthread_mutex_unlock(&A)
#endif
#else   /* no mutexing */
#define SSL_CTX_MUTEX_INIT(A)
#define SSL_CTX_MUTEX_DESTROY(A)
#define SSL_CTX_LOCK(A)
#define SSL_CTX_UNLOCK(A)
#endif
#ifdef __AIR202__

/* Number of bits in a `char'.  */
#   undef CHAR_BIT
#   define CHAR_BIT 8

/* Minimum and maximum values a `signed char' can hold.  */
#   undef SCHAR_MIN
#   define SCHAR_MIN (-128)
#   undef SCHAR_MAX
#   define SCHAR_MAX 127

/* Maximum value an `unsigned char' can hold.  (Minimum is 0).  */
#   undef UCHAR_MAX
#   define UCHAR_MAX 255

/* Minimum and maximum values a `char' can hold.  */
#   ifdef __CHAR_UNSIGNED__
#    undef CHAR_MIN
#    define CHAR_MIN 0
#    undef CHAR_MAX
#    define CHAR_MAX 255
#   else
#    undef CHAR_MIN
#    define CHAR_MIN (-128)
#    undef CHAR_MAX
#    define CHAR_MAX 127
#   endif

/* Minimum and maximum values a `signed short int' can hold.  */
#   undef SHRT_MIN
/* For the sake of 16 bit hosts, we may not use -32768 */
#   define SHRT_MIN (-32767-1)
#   undef SHRT_MAX
#   define SHRT_MAX 32767

/* Maximum value an `unsigned short int' can hold.  (Minimum is 0).  */
#   undef USHRT_MAX
#   define USHRT_MAX 65535

/* Minimum and maximum values a `signed int' can hold.  */
#   ifndef __INT_MAX__
#    define __INT_MAX__ 2147483647
#   endif
#   undef INT_MIN
#   define INT_MIN (-INT_MAX-1)
#   undef INT_MAX
#   define INT_MAX __INT_MAX__

/* Maximum value an `unsigned int' can hold.  (Minimum is 0).  */
#   undef UINT_MAX
#   define UINT_MAX (INT_MAX * 2U + 1)

/* Minimum and maximum values a `signed long int' can hold.
   (Same as `int').  */
#   ifndef __LONG_MAX__
#    if defined (__alpha__) || (defined (__sparc__) && defined(__arch64__)) || defined (__sparcv9)
#     define __LONG_MAX__ 9223372036854775807L
#    else
#     define __LONG_MAX__ 2147483647L
#    endif /* __alpha__ || sparc64 */
#   endif
#   undef LONG_MIN
#   define LONG_MIN (-LONG_MAX-1)
#   undef LONG_MAX
#   define LONG_MAX __LONG_MAX__

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0).  */
#   undef ULONG_MAX
#   define ULONG_MAX (LONG_MAX * 2UL + 1)

#   ifndef __LONG_LONG_MAX__
#    define __LONG_LONG_MAX__ 9223372036854775807LL
#   endif

#   if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* Minimum and maximum values a `signed long long int' can hold.  */
#    undef LLONG_MIN
#    define LLONG_MIN (-LLONG_MAX-1)
#    undef LLONG_MAX
#    define LLONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#    undef ULLONG_MAX
#    define ULLONG_MAX (LLONG_MAX * 2ULL + 1)
#   endif

#  if defined (__GNU_LIBRARY__) ? defined (__USE_GNU) : !defined (__STRICT_ANSI__)
/* Minimum and maximum values a `signed long long int' can hold.  */
#    undef LONG_LONG_MIN
#    define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#    undef LONG_LONG_MAX
#    define LONG_LONG_MAX __LONG_LONG_MAX__

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0).  */
#    undef ULONG_LONG_MAX
#    define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)
#   endif

struct tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
};

typedef uint32_t time_t;
enum
{
	false = 0,
	true,
};
#define DBG			iot_debug_print
#define TLS_DBG(X, Y...)	iot_debug_print("TLS %s %d:"X, __FUNCTION__, __LINE__, ##Y)
#define malloc		iot_os_malloc
#define realloc		iot_os_realloc
#define free		iot_os_free
#define mktime		OS_TimeToUTCTamp
#define printf(X, Y...)		iot_debug_print(X,##Y)
extern int OS_SocketRead(int SocketFd, void *Buf, uint16_t Len);
extern int OS_SocketWrite(int SocketFd, void *Buf, uint16_t Len);
#define abort()
time_t OS_TimeToUTCTamp(struct tm *_timeptr);
extern time_t time(time_t *_timer);
extern int gettimeofday(struct timeval *__p, void *__tz);
void* calloc(size_t __nmemb, size_t __size);
uint64_t __be64_to_cpu(uint64_t x);
#endif
#ifdef __cplusplus
}
#endif

#endif 
