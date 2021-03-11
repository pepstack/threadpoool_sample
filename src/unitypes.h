/***********************************************************************
* Copyright (c) 2008-2080 cheungmine, pepstack.com, 350137278@qq.com
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
***********************************************************************/

/**
 * @filename   unitypes.h
 *   Universal Definitions and Types, Bob Jenkins, Liang Zhang
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.14
 * @create     2019-09-30 12:37:44
 * @update     2020-12-09 17:26:44
 */
#ifndef UNITYPES_H_INCLUDED
#define UNITYPES_H_INCLUDED

#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32) && !defined(__SYMBIAN32__)
# define WIN32
#endif

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__CYGWIN__)
# if !(defined(_WINSOCKAPI_) || defined(_WINSOCK_H) || defined(__LWIP_OPT_H__) || defined(LWIP_HDR_OPT_H))
    /* The check above prevents the winsock2 inclusion if winsock.h already
     *  was included, since they can't co-exist without problems.
     */
#   include <winsock2.h>
#   include <ws2tcpip.h>
# endif
#endif


#undef _TIMESPEC_DEFINED

#if defined(__MINGW32__) || defined(__MINGW64__)
    # define __MINGW__   1
#endif

#if defined(WIN32)
    # if !defined(__WINDOWS__)
    #   define __WINDOWS__
    # endif

    # ifdef _WIN64
        // define something for Windows (64-bit only)
    #else
        // define something for Windows (32-bit only)
    #endif

    # define stat64buf_t  struct __stat64
    # define lstat64      _stat64

    // 92 = '\\'
    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 92)

#elif defined(__CYGWIN__) || defined(__MINGW__)

    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 47)
#elif defined(__APPLE__)
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

    // 47 = '/'
    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 47)
#elif defined(__linux__)

    # if _POSIX_C_SOURCE < 200809L
    #   undef _POSIX_C_SOURCE
    #   define _POSIX_C_SOURCE  200809L
    # endif

    # ifndef _LARGE_TIME_API
    #   define _LARGE_TIME_API
    #endif

    # include <sys/stat.h>

    # include <limits.h>
    # include <unistd.h>

    # define stat64buf_t  struct stat64

    // 47 = '/'
    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 47)
#elif defined(__unix__) // all unices not caught above
    // Unix

    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 47)
#elif defined(_POSIX_VERSION)
    // POSIX

    # undef  PATH_SEPARATOR_CHAR
    # define PATH_SEPARATOR_CHAR       ((char) 47)
#else
#   error "Unknown compiler"
#endif


#ifndef STDIO
    # include <stdio.h>
    # include <stdlib.h>
    # include <string.h>
    # include <assert.h>
    # include <stdarg.h>
    # include <malloc.h>

    # include <float.h>
    # include <limits.h>
    # include <stdint.h>

    # include <errno.h>
    # include <time.h>

    # define STDIO
#endif

#ifndef STDDEF
    # include <stddef.h>
    # include <ctype.h>
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
    # define _TIMESPEC_DEFINED

    /* Type definitions for the basic sized types. */
    # include <basetsd.h>
    typedef SSIZE_T ssize_t;

    /* DEBUG_PRINT_ENABLED */
    # if !defined (NDEBUG) && defined (DEBUG_PRINT_ENABLED)
        # define DEBUG_PRINT_MSG(message, ...)  do { \
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
        # define DEBUG_PRINT_MSG(message, ...)
    # endif

#else /* non __WINDOWS__ */
    # ifndef WINDOWS_CRTDBG_ON
    #   define WINDOWS_CRTDBG_ON
    # endif

    /* DEBUG_PRINT_ENABLED */
    # if !defined (NDEBUG) && defined (DEBUG_PRINT_ENABLED)
        # define DEBUG_PRINT_MSG(message, args...)  do { \
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
        # define DEBUG_PRINT_MSG(message, args...)
    # endif
#endif


/**
 * uniform types
 */
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


#ifndef NOWARNING_UNUSED
    # if defined(__GNUC__) || defined(__CYGWIN__)
        # define NOWARNING_UNUSED(x) __attribute__((unused)) x
    # else
        # define NOWARNING_UNUSED(x) x
    # endif
#endif


#ifndef STATIC_INLINE
    # if defined(_MSC_VER)
        # define STATIC_INLINE  NOWARNING_UNUSED(static) __forceinline
    # elif defined(__GNUC__) || defined(__CYGWIN__)
        # define STATIC_INLINE  NOWARNING_UNUSED(static) __attribute__((always_inline)) inline
    # else
        # define STATIC_INLINE  NOWARNING_UNUSED(static)
    # endif
#endif


/**
 * check and config value for integer variable if var is:
 *   0  : set by default value (defval)
 *  -1  : set by maximum value (maxval)
 *  others: set not less than minval and not more than maxval
 */
#define CHKCONFIG_INT_VALUE(defval, minval, maxval, var) \
    ((var)==0?((var)=(defval)):((var)==-1?((var)=(maxval)):((var)<(minval)?((var)=(minval)):((var)>(maxval)?((var)=(maxval)):(var)))))


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
NOWARNING_UNUSED(static)
int snprintf_chkd_V1(char *outputbuf, size_t bufsize, const char *format, ...)
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
NOWARNING_UNUSED(static)
int snprintf_chkd_V2(int exitcode, char *outputbuf, size_t bufsize, const char *format, ...)
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


/**
 * The compiler tries to warn you that you lose bits when casting from void *
 *   to int. It doesn't know that the void * is actually an int cast, so the
 *   lost bits are meaningless.
 *
 * A double cast would solve this (int)(uintptr_t)t->key. It first casts void *
 *   to uintptr_t (same size, no warning), then uintptr_t to int (number to
 *   number, no warning). Need to include <stdint.h> to have the uintptr_t type
 *   (an integral type with the same size as a pointer).
 */
#define ptr_cast_to_int(pv)      ((int) (uintptr_t) (void*) (pv))
#define int_cast_to_ptr(iv)      ((void*) (uintptr_t) (int) (iv))

#define ptr_cast_to_int64(pv)    ((int64_t) (uint64_t) (void*) (pv))
#define int64_cast_to_ptr(iv)    ((void*) (uint64_t) (int64_t) (iv))

#ifdef __cplusplus
}
#endif

#endif /* UNITYPES_H_INCLUDED */
