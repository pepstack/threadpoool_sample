/***********************************************************************
* Copyright (c) 2008-2080 pepstack.com, 350137278@qq.com
*
* ALL RIGHTS RESERVED.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************/

/**
* @filename   unitypes.h
*   Universal Standard definitions and types, Bob Jenkins, Liang Zhang
*
* @author     Liang Zhang <350137278@qq.com>
* @version    0.0.10
* @create     2019-09-30 12:37:44
* @update     2019-10-31 11:55:51
*/
#ifndef UNITYPES_H_INCLUDED
#define UNITYPES_H_INCLUDED

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
    * There are predefined macros that are used by most compilers.
    * Here is an example for gcc:
    */
#ifdef _WIN32
    // define something for Windows (32-bit and 64-bit, this part is common)
# if !defined(__WINDOWS__)
#   define __WINDOWS__
# endif

    // same as: <unistd.h>
# include <io.h>
# include <process.h>

# undef _USE_32BIT_TIME_T
# include <sys/stat.h>

    // 92: '\\'
# define PATH_SEPARATOR_CHAR       ((char) 92)
# define stat64buf_t  struct __stat64
# define lstat64      _stat64

    /*
    #ifdef _WIN64
    // define something for Windows (64-bit only)
    #else
    // define something for Windows (32-bit only)
    #endif
    */
#elif __APPLE__
# include "TargetConditionals.h"

# if TARGET_IPHONE_SIMULATOR
    // iOS Simulator
# elif TARGET_OS_IPHONE
    // iOS device
# elif TARGET_OS_MAC
    // Other kinds of Mac OS
# else
#   error "Unknown Apple platform"
# endif
#elif __linux__
# if _POSIX_C_SOURCE < 200809L
#   undef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE  200809L
#endif

# ifndef _LARGE_TIME_API
#   define _LARGE_TIME_API
#endif

# include <sys/stat.h>

# include <limits.h>
# include <unistd.h>

    // 47: '/'
# define PATH_SEPARATOR_CHAR       ((char) 47)
# define stat64buf_t  struct stat64
    // # define lstat64      lstat64
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif


#ifndef STDIO
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>
# include <stdarg.h>

# include <float.h>
# include <limits.h>
# include <stdint.h>
# define STDIO
#endif

#ifndef STDDEF
# include <stddef.h>
# define STDDEF
#endif

#ifndef BCOPY
# if defined(__WINDOWS__)
#   define bcopy(s,d,n)    memcpy((void*)(d), (const void*)(s), (size_t)(n))
# endif
# define BCOPY
#endif

#ifndef BZERO
# if defined(__WINDOWS__)
#   define bzero(s,n)    memset((void*)(s), 0, (size_t)(n))
# endif
# define BZERO
#endif

#ifndef SNPRINTF
# if defined(__WINDOWS__)
#   define snprintf    _snprintf
# endif
# define SNPRINTF
#endif

#ifndef STRTOK
# if defined(__WINDOWS__)
#   define strtok_r    strtok_s
# endif
# define STRTOK
#endif

#if defined(__WINDOWS__)
# if defined (_MSC_VER)
    // warning C4996: 'vsnprintf': This function or variable may be unsafe.
    // Consider using vsnprintf_s instead.
    //  To disable deprecation, use _CRT_SECURE_NO_WARNINGS
#   pragma warning(disable:4996)

#   if defined(_DEBUG)
    /** memory leak auto-detect in MSVC
    * https://blog.csdn.net/lyc201219/article/details/62219503
    */
#     define _CRTDBG_MAP_ALLOC
#     include <stdlib.h>
#     include <malloc.h>
#     include <crtdbg.h>
#     define WINDOWS_CRTDBG_ON  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#   else
#     include <stdlib.h>
#     include <malloc.h>
#     define WINDOWS_CRTDBG_ON
#   endif

# endif /* _MSC_VER */

# include <basetsd.h>  /* Type definitions for the basic sized types. */
    typedef SSIZE_T ssize_t;

# if !defined (NDEBUG) && defined (DEBUG_PRINT_ENABLED)
#   define DEBUG_PRINT_MSG(message, ...)  do { \
            time_t t; \
            struct tm *pt; \
            t = time(NULL); \
            pt = localtime(&t); \
            printf("(%04d-%02d-%02d %02d:%02d:%02d - %s:%d %s) - " \
                message, \
                pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec, \
                __FILE__, __LINE__, __FUNCTION__, \
                __VA_ARGS__); \
            } while(0)
# else
#   define DEBUG_PRINT_MSG(message, ...)
# endif /* DEBUG_PRINT_ENABLED */

#else /* non __WINDOWS__ */

# if !defined (NDEBUG) && defined (DEBUG_PRINT_ENABLED)
#   define DEBUG_PRINT_MSG(message, args...)  do { \
            time_t t; \
            struct tm *pt; \
            t = time(NULL); \
            pt = localtime(&t); \
            printf("(%04d-%02d-%02d %02d:%02d:%02d - %s:%d %s) - " \
                message, \
                pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec, \
                __FILE__, __LINE__, __FUNCTION__, \
                ##args); \
            } while(0)
# else
#   define DEBUG_PRINT_MSG(message, args...)
# endif /* DEBUG_PRINT_ENABLED */

# define WINDOWS_CRTDBG_ON
#endif


#if defined (_SVR4) || defined (SVR4) || defined (__OpenBSD__) || \
    defined (_sgi) || defined (__sun) || defined (sun) || \
    defined (__digital__) || defined (__HP_cc)
# include <inttypes.h>
#elif defined (_MSC_VER) && _MSC_VER < 1600
    /* VS 2010 (_MSC_VER 1600) has stdint.h */
    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#elif defined (_AIX)
# include <sys/inttypes.h>
#else
# include <inttypes.h>
#endif


    typedef uint64_t ub8;
#define UB8MAXVAL 0xffffffffffffffffLL
#define UB8BITS 64

    typedef int64_t sb8;
#define SB8MAXVAL 0x7fffffffffffffffLL

    /* unsigned 4-byte quantities */
    typedef uint32_t ub4;
#define UB4MAXVAL 0xffffffff

    typedef int32_t sb4;
#define UB4BITS 32
#define SB4MAXVAL 0x7fffffff

    typedef uint16_t ub2;
#define UB2MAXVAL 0xffff
#define UB2BITS 16

    typedef int16_t sb2;
#define SB2MAXVAL 0x7fff

    /* unsigned 1-byte quantities */
    typedef unsigned char ub1;
#define UB1MAXVAL 0xff
#define UB1BITS 8

    /* signed 1-byte quantities */
    typedef signed char sb1;
#define SB1MAXVAL 0x7f


    /**
    * snprintf_chkd_V1()
    *   A checked V1 version of snprintf() for both GCC and MSVC
    *   No error.
    * see:
    *   <stdarg.h>
    *   https://linux.die.net/man/3/snprintf
    *
    *   The functions snprintf() and vsnprintf() do not write more than size bytes
    *    (including the terminating null byte ('\0')).
    *   If the output was truncated due to this limit then the return value is the
    *    number of characters (excluding the terminating null byte) which would have
    *    been written to the final string if enough space had been available.
    *   Thus, a return value of size or more means that the output was truncated.
    */
    static int snprintf_chkd_V1(char *outputbuf, size_t bufsize, const char *format, ...)
    {
        int len;

        va_list args;
        va_start(args, format);
        len = vsnprintf(outputbuf, bufsize, format, args);
        va_end(args);

        if (len < 0 || len >= (int)bufsize) {
            /* output was truncated due to bufsize limit */
            len = (int)bufsize - 1;

            /* for MSVC */
            outputbuf[len] = '\0';
        }

        return len;
    }


    /**
    * snprintf_chkd_V2()
    *   A crashed on error version of snprintf.
    *
    *    If exitcode not given (= 0), same as snprintf_safe()
    */
    static int snprintf_chkd_V2(int exitcode, char *outputbuf, size_t bufsize, const char *format, ...)
    {
        int len;

        va_list args;
        va_start(args, format);
        len = vsnprintf(outputbuf, bufsize, format, args);
        va_end(args);

        if (len < 0 || len >= (int)bufsize) {
            /* output was truncated due to bufsize limit */
            len = (int)bufsize - 1;

            /* for MSVC */
            outputbuf[len] = '\0';

            /* exit on error if exitcode given (not 0) */
            if (exitcode) {
                fprintf(stderr, "(%s:%d) fatal: output was truncated. (%s...)\n", __FILE__, __LINE__, outputbuf);
                exit(exitcode);
            }
        }

        return len;
    }

#define snprintf_V1    snprintf


#if defined(__WINDOWS__)
# include <WinSock2.h>     /* select(), struct timeval */
# include <time.h>         /* struct timespec */

# define HAVE_STRUCT_TIMESPEC

    // imp_select error: Need to link with Ws2_32.lib
# pragma comment(lib, "ws2_32.lib")

    // sleep in milliseconds
    static void sleep_msec(int ms)
    {
        if (ms >= 0) {
            Sleep(ms);
        }
    }

    // sleep in micro seconds, do not use usleep
    static void sleep_usec(int microseconds)
    {
        int ms = microseconds / 1000;
        int us = microseconds % 1000;

        if (ms > 0) {
            Sleep(ms);
        }

        if (us > 0) {
            LARGE_INTEGER lval = { 0 };
            LONGLONG startTime;
            double cpuFreq, delayTimeUs = 0;

            // 获得计数器的时钟频率: cpuFreq
            if (!QueryPerformanceFrequency(&lval)) {
                return;
            }
            cpuFreq = (double)lval.QuadPart;
            if (cpuFreq < 1000) {
                return;
            }

            // 获得初始值: startTime
            if (!QueryPerformanceCounter(&lval)) {
                return;
            }
            startTime = lval.QuadPart;

            do {
                Sleep(0);

                if (!QueryPerformanceCounter(&lval)) {
                    return;
                }
                delayTimeUs = (((double)(lval.QuadPart - startTime)) / cpuFreq) * 1000000;
            } while (delayTimeUs < us);
        }
    }

#elif __linux__
# include <time.h>      /* nanosleep() */
# include <unistd.h>    /* usleep() */

# define HAVE_STRUCT_TIMESPEC

    // sleep in milliseconds
    static void sleep_msec(int milliseconds)
    {
        if (milliseconds > 0) {
            struct timespec ts;
            ts.tv_sec = milliseconds / 1000;
            ts.tv_nsec = (milliseconds % 1000) * 1000000;
            nanosleep(&ts, 0);
        }
    }


    // sleep in micro seconds, do not use usleep
    static void sleep_usec(int us)
    {
        if (us > 0) {
            /**
            * 1 sec = 1000 ms (millisec)
            * 1 ms = 1000 us (microsec)
            * 1 us = 1000 ns (nanosec)
            * 1 sec = 1000 000 000 ns (nanosec)
            */
            struct timespec ts;

            ts.tv_sec = us / 1000000;
            ts.tv_nsec = (us % 1000000) * 1000;

            nanosleep(&ts, 0);
        }
    }
#else
# include <time.h>      /* nanosleep() */
# include <unistd.h>    /* usleep() */

# define sleep_msec(ms)  usleep((ms) * 1000)
# define sleep_usec(us)  usleep(us)

# define HAVE_STRUCT_TIMESPEC
#endif

#ifdef __cplusplus
}
#endif

#endif /* UNITYPES_H_INCLUDED */
