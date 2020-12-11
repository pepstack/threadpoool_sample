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
 * @filename   misc.h
 *    miscellaneous tools for application.
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.10
 * @create     2017-08-28 11:12:10
 * @update     2020-12-12 22:55:37
 */
#ifndef _MISC_H_
#define _MISC_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include "cstrbuf.h"

#define ERROR_STRING_LEN_MAX   1024

#if defined(__WINDOWS__)
# include <Windows.h>
  typedef HANDLE filehandle_t;

# define filehandle_invalid INVALID_HANDLE_VALUE
# define getprocessid()  ((int)GetCurrentProcessId())
# define getthreadid()   ((int) GetCurrentThreadId())

static void sleep_msec(int milliseconds)
{
    Sleep(milliseconds);
}


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

        // get cpuFreq
        if (!QueryPerformanceFrequency(&lval)) {
            return;
        }
        cpuFreq = (double)lval.QuadPart;
        if (cpuFreq < 1000) {
            return;
        }

        // get startTime
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

#else /* non-windows: Linux or Cygwin */

# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <fcntl.h>
# include <unistd.h>    /* usleep() */

# if defined(__CYGWIN__)
#   include <Windows.h>
#   include <pthread.h>    /* pthread_self() */
NOWARNING_UNUSED(static) pid_t getthreadid(void)
{
    pthread_t tid = pthread_self();
    return (int)ptr_cast_to_int(tid);
}
# else /* non-cygwin */
# include <sys/syscall.h> /* syscall(SYS_gettid) */

NOWARNING_UNUSED(static) pid_t getthreadid(void)
{
    return syscall(SYS_gettid);
}
# endif 

# define getprocessid()   ((int)getpid())

typedef int filehandle_t;
# define filehandle_invalid ((filehandle_t)(-1))

# define _TIMESPEC_DEFINED

// sleep in milliseconds
NOWARNING_UNUSED(static) void sleep_msec(int milliseconds)
{
    if (milliseconds > 0) {
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, 0);
    }
}

// sleep in micro seconds, do not use usleep
NOWARNING_UNUSED(static) void sleep_usec(int us)
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

#endif


/**
 * time api
 */
NOWARNING_UNUSED(static)
const char *timezone_format(long tz, char *tzfmt)
{
    if (tz < 0) {
        snprintf_chkd_V1(tzfmt, 5 + 1, "+%02d%02d", -(int)(tz / 3600), -(int)((tz % 3600) / 60));
    } else if (tz > 0) {
        snprintf_chkd_V1(tzfmt, 5 + 1, "-%02d%02d", (int)(tz / 3600), (int)((tz % 3600) / 60));
    } else {
        /* UTC */
        memcpy(tzfmt, "+0000", 5);
    }
    tzfmt[5] = 0;
    return tzfmt;
}


NOWARNING_UNUSED(static)
long timezone_compute(time_t ts, char *tzfmt)
{
    long tz;
    time_t ut;
    struct tm t;

#if defined(__WINDOWS__)
    if (gmtime_s(&t, &ts) != (errno_t)0) {
        /* error */
        perror("gmtime_s\n");
        return (-1);
    }
    ut = mktime(&t);
#else
    if (!gmtime_r(&ts, &t)) {
        /* error */
        perror("gmtime\n");
        return (-1);
    }
    ut = mktime(&t);
#endif

    tz = (long)difftime(ut, ts);
    if (tzfmt) {
        timezone_format(tz, tzfmt);
    }

    return tz;
}


NOWARNING_UNUSED(static)
int daylight_compute(time_t ts)
{
    struct tm tm;
    tm.tm_isdst = 0;

#if defined(__WINDOWS__)
    localtime_s(&tm, &ts);
#else
    localtime_r(&ts, &tm);
#endif

    return tm.tm_isdst;
}


NOWARNING_UNUSED(static)
void getnowtimeofday(struct timespec *now)
{
#if defined(_WIN32)
    FILETIME tmfile;
    ULARGE_INTEGER _100nanos;

    GetSystemTimeAsFileTime(&tmfile);

    _100nanos.LowPart   = tmfile.dwLowDateTime;
    _100nanos.HighPart  = tmfile.dwHighDateTime; 
    _100nanos.QuadPart -= 0x19DB1DED53E8000;

    /* Convert 100ns units to seconds */
    now->tv_sec = (time_t)(_100nanos.QuadPart / (10000 * 1000));

    /* Convert remainder to nanoseconds */
    now->tv_nsec = (long) ((_100nanos.QuadPart % (10000 * 1000)) * 100);
#else
    if (clock_gettime(CLOCK_REALTIME, now) == -1) {
        /* must be successful */
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
#endif
}


/**
 * taken from: redis/src/localtime.c
 *   although localtime_s on windows is a bit faster than getlocaltime_safe,
 *   here we use getlocaltime_safe due to it cross-platforms.
 */
NOWARNING_UNUSED(static)
void getlocaltime_safe(struct tm *loc, int64_t t, int tz, int dst)
{
/**
 * A year not divisible by 4 is not leap.
 * If div by 4 and not 100 is surely leap.
 * If div by 100 *and* 400 is not leap.
 * If div by 100 and not by 400 is leap.
 */
#define YEAR_IS_LEAPYEAR(year) ((year) % 4 ? 0 : ((year) % 100 ? 1 : ((year) % 400 ? 0 : 1)))
#define SECONDS_PER_MINUTE ((time_t)60)
#define SECONDS_PER_HOUR ((time_t)3600)
#define SECONDS_PER_DAY ((time_t)86400)

    /* We need to calculate in which month and day of the month we are. To do
     * so we need to skip days according to how many days there are in each
     * month, and adjust for the leap year that has one more day in February.
     */
    static const int mean_mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    static const int leap_mdays[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* Adjust for timezone. 0 for UTC */
    t -= tz;

    /* Adjust for daylight time. 0 default */
    t += 3600 * dst;

    /* Days passed since epoch. */
    int64_t days = t / SECONDS_PER_DAY;

    /* Remaining seconds. */
    int64_t seconds = t % SECONDS_PER_DAY;

    loc->tm_isdst = dst;
    loc->tm_hour = (int)(seconds / SECONDS_PER_HOUR);
    loc->tm_min = (int)((seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    loc->tm_sec = (int)((seconds % SECONDS_PER_HOUR) % SECONDS_PER_MINUTE);

    /* 1/1/1970 was a Thursday, that is, day 4 from the POV of the tm structure
     * where sunday = 0, so to calculate the day of the week we have to add 4
     * and take the modulo by 7. */
    loc->tm_wday = (int)((days + 4) % 7);

    /* Calculate the current year. */
    loc->tm_year = 1970;

    while (1) {
        /* Leap years have one day more. */
        int64_t days_this_year = 365 + YEAR_IS_LEAPYEAR(loc->tm_year);
        if (days_this_year > days)
            break;
        days -= days_this_year;
        loc->tm_year++;
    }

    /* Number of day of the current year. */
    loc->tm_yday = (int)days;
    loc->tm_mon = 0;

    if (YEAR_IS_LEAPYEAR(loc->tm_year)) {
        while (days >= leap_mdays[loc->tm_mon]) {
            days -= leap_mdays[loc->tm_mon];
            loc->tm_mon++;
        }
    } else {
        while (days >= mean_mdays[loc->tm_mon]) {
            days -= mean_mdays[loc->tm_mon];
            loc->tm_mon++;
        }
    }

    /* Add 1 since our 'days' is zero-based. */
    loc->tm_mday = (int)(days + 1);

    /* Surprisingly tm_year is year-1900. */
    loc->tm_year -= 1900;

#undef SECONDS_PER_MINUTE
#undef SECONDS_PER_HOUR
#undef SECONDS_PER_DAY
#undef YEAR_IS_LEAPYEAR
}


NOWARNING_UNUSED(static)
sb8 difftime_msec(const struct timespec *oldtms, const struct timespec *newtms)
{
    sb8 sec = 0;
    sb8 nsec = 0;

    if (!oldtms && !newtms) {
        /* get current timestamp in ms */
        struct timespec now;
        getnowtimeofday(&now);
        sec = now.tv_sec;
        nsec = now.tv_nsec;
    } else if (oldtms && newtms) {
        sec = (sb8)(newtms->tv_sec - oldtms->tv_sec);
        nsec = (sb8)(newtms->tv_nsec - oldtms->tv_nsec);
    } else if (newtms) {
        sec = (sb8)(newtms->tv_sec);
        nsec = (sb8)(newtms->tv_nsec);
    } else if (oldtms) {
        sec = (sb8)(oldtms->tv_sec);
        nsec = (sb8)(oldtms->tv_nsec);
    }

    if (sec > 0) {
        if (nsec >= 0) {
            return ((sec * 1000UL) + nsec / 1000000UL);
        } else { /* nsec < 0 */
            return (sec - 1) * 1000UL + (nsec + 1000000000UL) / 1000000UL;
        }
    } else if (sec < 0) {
        if (nsec <= 0) {
            return ((sec * 1000UL) + nsec / 1000000UL);
        } else{ /* nsec > 0 */
            return (sec + 1) * 1000UL + (nsec - 1000000000UL) / 1000000UL;
        }
    } else { /* sec = 0 */
        return nsec / 1000000UL;
    }
}


NOWARNING_UNUSED(static)
const char * format_nowtimeofday (char *datefmt)
{
    struct timespec now;
    struct tm loc;

    getnowtimeofday(&now);
    getlocaltime_safe(&loc, now.tv_sec, 0, 0);

    snprintf_chkd_V1(datefmt, 24, "%04d-%02d-%02d %02d:%02d:%02d UTC", loc.tm_year + 1900, loc.tm_mon + 1, loc.tm_mday, loc.tm_hour, loc.tm_min, loc.tm_sec);

    datefmt[23] = 0;
    return datefmt;
}


/**
 * error message api
 */
#if defined(__WINDOWS__) || defined(__CYGWIN__)
NOWARNING_UNUSED(static)
const char *format_win32_syserror (int werrnum, char *errmsgbuf, size_t bufsize)
{
    int len;
    *errmsgbuf = '\0';
    len = (int) FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD)werrnum, 0, errmsgbuf, (DWORD)bufsize, NULL);
    if (! len) {
        snprintf(errmsgbuf, bufsize, "Failed to FormatMessageA for Windows Error(%d): LastError=%d", werrnum, (int) GetLastError());
    } else if (len < (int) bufsize) {
        errmsgbuf[len] = '\0';
    }
    errmsgbuf[bufsize - 1] = '\0';
    return errmsgbuf;
}
#else
NOWARNING_UNUSED(static)
const char *format_posix_syserror (int errnum, char *errmsgbuf, size_t bufsize)
{
    char *str = errmsgbuf;
    *errmsgbuf = '\0';

    /* GNU-specific */
    str = strerror_r(errnum, errmsgbuf, bufsize);

    errmsgbuf[bufsize - 1] = '\0';
    return str;
}
#endif


/**
 * file api
 */
#if defined(__WINDOWS__)

NOWARNING_UNUSED(static)
filehandle_t file_create(const char *pathname, int flags, int mode)
{
    filehandle_t hf = CreateFileA(pathname, (DWORD)(flags), FILE_SHARE_READ,
                                  NULL,
                                  CREATE_NEW,
                                  (DWORD)(mode),
                                  NULL);
    return hf;
}

NOWARNING_UNUSED(static)
filehandle_t file_open_read(const char *pathname)
{
    filehandle_t hf = CreateFileA(pathname,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
    return hf;
}

NOWARNING_UNUSED(static)
int file_close(filehandle_t *phf)
{
    if (phf) {
        filehandle_t hf = *phf;

        if (hf != filehandle_invalid) {
            *phf = filehandle_invalid;

            if (CloseHandle(hf)) {
                return 0;
            }
        }
    }
    return (-1);
}

NOWARNING_UNUSED(static)
int file_readbytes(filehandle_t hf, char *bytesbuf, ub4 sizebuf)
{
    BOOL ret;
    DWORD cbread, cboffset = 0;

    while (cboffset != (DWORD)sizebuf) {
        ret = ReadFile(hf, (void *)(bytesbuf + cboffset), (DWORD)(sizebuf - cboffset), &cbread, NULL);

        if (!ret) {
            /* read on error: uses GetLastError() for more */
            return (-1);
        }

        if (cbread == 0) {
            /* reach to end of file */
            break;
        }

        cboffset += cbread;
    }

    /* success: actual read bytes */
    return (int)cboffset;
}

NOWARNING_UNUSED(static)
int file_writebytes(filehandle_t hf, const char *bytesbuf, ub4 bytestowrite)
{
    BOOL ret;
    DWORD cbwritten, cboffset = 0;

    while (cboffset != (DWORD)bytestowrite) {
        ret = WriteFile(hf, (const void *)(bytesbuf + cboffset), (DWORD)(bytestowrite - cboffset), &cbwritten, NULL);

        if (!ret) {
            /* write on error */
            return (-1);
        }

        cboffset += cbwritten;
    }

    /* success */
    return 0;
}

NOWARNING_UNUSED(static)
int pathfile_exists(const char *pathname)
{
    if (pathname) {
        WIN32_FIND_DATAA FindFileData;
        HANDLE handle = FindFirstFileA(pathname, &FindFileData);

        if (handle != INVALID_HANDLE_VALUE) {
            FindClose(handle);

            /* found */
            return 1;
        }
    }

    /* not found */
    return 0;
}

NOWARNING_UNUSED(static)
int pathfile_remove(const char *pathname)
{
    if (DeleteFileA(pathname)) {
        return 0;
    }

    return (-1);
}

NOWARNING_UNUSED(static)
int pathfile_move(const char *pathnameOld, const char *pathnameNew)
{
    return MoveFileA(pathnameOld, pathnameNew);
}

#else

/* Linux? */
NOWARNING_UNUSED(static)
filehandle_t file_create(const char *pathname, int flags, int mode)
{
    int fd = open(pathname, flags | O_CREAT | O_EXCL, (mode_t)(mode));
    return fd;
}

NOWARNING_UNUSED(static)
filehandle_t file_open_read(const char *pathname)
{
    int fd = open(pathname, O_RDONLY | O_EXCL, S_IRUSR | S_IRGRP | S_IROTH);
    return fd;
}

NOWARNING_UNUSED(static)
int file_close(filehandle_t *phf)
{
    if (phf) {
        filehandle_t hf = *phf;

        if (hf != filehandle_invalid) {
            *phf = filehandle_invalid;

            return close(hf);
        }
    }
    return (-1);
}

NOWARNING_UNUSED(static)
int file_readbytes(filehandle_t hf, char *bytesbuf, ub4 sizebuf)
{
    size_t cbread, cboffset = 0;

    while (cboffset != (size_t)sizebuf) {
        cbread = read(hf, (void *)(bytesbuf + cboffset), (size_t)(sizebuf - cboffset));

        if (cbread == -1) {
            /* read on error: uses strerror(errno) for more */
            return (-1);
        }

        if (cbread == 0) {
            /* reach to end of file */
            break;
        }

        cboffset += cbread;
    }

    /* success: actual read bytes */
    return (int)cboffset;
}

/* https://linux.die.net/man/2/write */
NOWARNING_UNUSED(static)
int file_writebytes(filehandle_t hf, const char *bytesbuf, ub4 bytestowrite)
{
    ssize_t cbret;
    off_t woffset = 0;

    while (woffset != (off_t)bytestowrite) {
        cbret = write(hf, (const void *)(bytesbuf + woffset), bytestowrite - woffset);

        if (cbret == -1) {
            /* error */
            return (-1);
        }

        if (cbret > 0) {
            woffset += cbret;
        }
    }

    /* success */
    return 0;
}

NOWARNING_UNUSED(static)
int pathfile_exists(const char *pathname)
{
    if (pathname && access(pathname, F_OK) != -1) {
        /* file exists */
        return 1;
    }

    /* file doesn't exist */
    return 0;
}

NOWARNING_UNUSED(static)
int pathfile_remove(const char *pathname)
{
    return remove(pathname);
}

NOWARNING_UNUSED(static)
int pathfile_move(const char *pathnameOld, const char *pathnameNew)
{
    return rename(pathnameOld, pathnameNew);
}

#endif

NOWARNING_UNUSED(static)
cstrbuf get_proc_pathfile(void)
{
    int r, bufsize = 128;
    char *pathbuf = alloca(bufsize);

#if defined(__WINDOWS__)

    while ((r = (int)GetModuleFileNameA(0, pathbuf, (DWORD)bufsize)) >= bufsize) {
        bufsize += 128;
        pathbuf = alloca(bufsize);
    }

    if (r <= 0) {
        printf("GetModuleFileNameA failed.\n");
        exit(EXIT_FAILURE);
    }

#else

    while ((r = readlink("/proc/self/exe", pathbuf, bufsize)) >= (int)bufsize) {
        bufsize += 128;
        pathbuf = alloca(bufsize);
    }

    if (r <= 0) {
        printf("readlink failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

#endif

    pathbuf[r] = '\0';

    return cstrbufNew(0, pathbuf, r);
}


/**
 * get_proc_abspath
 *   get absolute path for current process.
 */
NOWARNING_UNUSED(static)
cstrbuf get_proc_abspath(void)
{
    cstrbuf pathfile = get_proc_pathfile();

    char *p = strrchr(pathfile->str, '\\');
    char *q = strrchr(pathfile->str, '/');

    if (p && q) {
        (p > q)? (*p = 0) : (*q = 0);
    } else if (p) {
        *p = 0;
    } else if (q) {
        *q = 0;
    }

    pathfile->len = (ub4)strnlen(pathfile->str, pathfile->len);

    return pathfile;
}

NOWARNING_UNUSED(static)
cstrbuf find_config_pathfile(const char *cfgpath, const char *cfgname, const char *envvarname, const char *etcconfpath)
{
    cstrbuf config = 0;
    cstrbuf dname = 0;
    cstrbuf tname = 0;
    cstrbuf pname = 0;

    tname = cstrbufCat(0, "/%s", cfgname);
    pname = cstrbufCat(0, "%c%s", PATH_SEPARATOR_CHAR, cfgname);

    if (cfgpath) {
        int pathlen = cstr_length(cfgpath, -1);
        if (cstr_endwith(cfgpath, pathlen, pname->str, (int)pname->len) ||
            cstr_endwith(cfgpath, pathlen, tname->str, (int)tname->len)) {
            config = cstrbufNew(0, cfgpath, pathlen);
        } else {
            char endchr = cfgpath[pathlen - 1];

            if (endchr == PATH_SEPARATOR_CHAR || endchr == '/') {
                config = cstrbufCat(0, "%.*s%.*s", pathlen, cfgpath, (int)pname->len - 1, cfgname);
            } else {
                config = cstrbufCat(0, "%.*s%c%.*s", pathlen, cfgpath, PATH_SEPARATOR_CHAR, (int)pname->len - 1, cfgname);
            }
        }

        printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
        goto finish_up;
    } else {
        char *p;
        dname = get_proc_abspath();

        // 1: "$(appbin_dir)/clogger.cfg"
        config = cstrbufConcat(dname, pname, 0);

        if (config) {
            printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
        } else {
            printf("[misc.h:%d] null config\n", __LINE__);
        }

        if (pathfile_exists(config->str)) {
            goto finish_up;
        }
        cstrbufFree(&config);

        // 2: "$(appbin_dir)/conf/clogger.cfg"
        config = cstrbufCat(0, "%.*s%cconf%.*s", cstrbufGetLen(dname), cstrbufGetStr(dname), PATH_SEPARATOR_CHAR, cstrbufGetLen(pname), cstrbufGetStr(pname));
        printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
        if (pathfile_exists(config->str)) {
            goto finish_up;
        }

        // 3: "$appbindir/../conf/clogger.cfg"
        cstrbufTrunc(config, dname->len);
        p = strrchr(config->str, PATH_SEPARATOR_CHAR);
        if (p) {
            cstrbufTrunc(config, (ub4)(p - config->str));
            config = cstrbufCat(config, "%cconf%.*s", PATH_SEPARATOR_CHAR, cstrbufGetLen(pname), cstrbufGetStr(pname));

            if (config) {
                printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
            } else {
                printf("[misc.h:%d] null config\n", __LINE__);
            }

            if (pathfile_exists(config->str)) {
                goto finish_up;
            }
        }
        cstrbufFree(&config);

        if (envvarname) {
            // 4: $CLOGGER_CONF=/path/to/clogger.cfg
            printf("[misc.h:%d] check environment: %s\n", __LINE__, envvarname);

            p = getenv(envvarname);
            if (p) {
                char endchr;
                config = cstrbufNew(cstr_length(p, -1) + pname->len + 1, p, -1);

                if (cstr_endwith(config->str, config->len, pname->str, (int)pname->len) ||
                    cstr_endwith(config->str, config->len, tname->str, (int)tname->len)) {
                    printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
                    goto finish_up;
                }

                endchr = config->str[config->len - 1];
                if (endchr == PATH_SEPARATOR_CHAR || endchr == '/') {
                    config = cstrbufCat(config, "%.*s", (int)pname->len - 1, cfgname);
                } else {
                    config = cstrbufCat(config, "%.*s", cstrbufGetLen(pname), cstrbufGetStr(pname));
                }

                printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
                goto finish_up;
            }
        }

        if (etcconfpath) {
            printf("[misc.h:%d] check os path: %s\n", __LINE__, etcconfpath);

            config = cstrbufCat(0, "%s%.*s", etcconfpath, cstrbufGetLen(pname), cstrbufGetStr(pname));
            printf("[misc.h:%d] check config: %.*s\n", __LINE__, cstrbufGetLen(config), cstrbufGetStr(config));
            goto finish_up;
        }
    }

finish_up:
    cstrbufFree(&tname);
    cstrbufFree(&pname);
    cstrbufFree(&dname);

    /* dont need to know whether config exists or not at all */
    return config;
}

#ifdef __cplusplus
}
#endif
#endif /* _MISC_H_ */
