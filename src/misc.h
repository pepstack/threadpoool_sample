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
 * @update     2021-03-13 22:55:37
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
# define fseek_pos_set    ((int)FILE_BEGIN)
# define fseek_pos_cur    ((int)FILE_CURRENT)
# define fseek_pos_end    ((int)FILE_END)

# define getprocessid()  ((int)GetCurrentProcessId())
# define getthreadid()   ((int) GetCurrentThreadId())

#else /* non-windows: Linux or Cygwin */

/* See feature_test_macros(7) */
#ifndef _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE
#endif

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

# define fseek_pos_set    ((int)SEEK_SET)
# define fseek_pos_cur    ((int)SEEK_CUR)
# define fseek_pos_end    ((int)SEEK_END)

#endif


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
filehandle_t file_write_new(const char *pathname)
{
    return file_create(pathname, GENERIC_WRITE, FILE_ATTRIBUTE_NORMAL);
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
sb8 file_seek(filehandle_t hf, sb8 distance, int fseekpos)
{
    LARGE_INTEGER li;
    li.QuadPart = distance;
    if (SetFilePointerEx(hf, li, &li, fseekpos)) {
        return (sb8)li.QuadPart;
    }
    /* error */
    return (sb8)(-1);
}

NOWARNING_UNUSED(static)
sb8 file_size(filehandle_t hf)
{
    LARGE_INTEGER li;
    if (GetFileSizeEx(hf, &li)) {
        /* success */
        return (sb8)li.QuadPart;
    }

    /* error */
    return (sb8)(-1);
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

#else /* Linux? */

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
filehandle_t file_write_new(const char *pathname)
{
    return file_create(pathname, O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
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
sb8 file_seek(filehandle_t hf, sb8 distance, int fseekpos)
{
    #ifdef WIN32
        /* warning: cygwin */
        return  (sb8) lseek(hf, distance, fseekpos);
    #else
        return (sb8) lseek64(hf, (off64_t)distance, fseekpos);
    #endif
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

        printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
        goto finish_up;
    } else {
        char *p;
        dname = get_proc_abspath();

        // 1: "$(appbin_dir)/clogger.cfg"
        config = cstrbufConcat(dname, pname, 0);

        if (config) {
            printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
        } else {
            printf("(misc.h:%d) [%d] null config\n", __LINE__, getprocessid());
        }

        if (pathfile_exists(config->str)) {
            goto finish_up;
        }
        cstrbufFree(&config);

        // 2: "$(appbin_dir)/conf/clogger.cfg"
        config = cstrbufCat(0, "%.*s%cconf%.*s", cstrbufGetLen(dname), cstrbufGetStr(dname), PATH_SEPARATOR_CHAR, cstrbufGetLen(pname), cstrbufGetStr(pname));
        printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
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
                printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
            } else {
                printf("(misc.h:%d) [%d] null config\n", __LINE__, getprocessid());
            }

            if (pathfile_exists(config->str)) {
                goto finish_up;
            }
        }
        cstrbufFree(&config);

        if (envvarname) {
            // 4: $CLOGGER_CONF=/path/to/clogger.cfg
            printf("(misc.h:%d) [%d] check environment: %s\n", __LINE__, getprocessid(), envvarname);

            p = getenv(envvarname);
            if (p) {
                char endchr;
                config = cstrbufNew(cstr_length(p, -1) + pname->len + 1, p, -1);

                if (cstr_endwith(config->str, config->len, pname->str, (int)pname->len) ||
                    cstr_endwith(config->str, config->len, tname->str, (int)tname->len)) {
                    printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
                    goto finish_up;
                }

                endchr = config->str[config->len - 1];
                if (endchr == PATH_SEPARATOR_CHAR || endchr == '/') {
                    config = cstrbufCat(config, "%.*s", (int)pname->len - 1, cfgname);
                } else {
                    config = cstrbufCat(config, "%.*s", cstrbufGetLen(pname), cstrbufGetStr(pname));
                }

                printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
                goto finish_up;
            }
        }

        if (etcconfpath) {
            printf("(misc.h:%d) [%d] check os path: %s\n", __LINE__, getprocessid(), etcconfpath);

            config = cstrbufCat(0, "%s%.*s", etcconfpath, cstrbufGetLen(pname), cstrbufGetStr(pname));
            printf("(misc.h:%d) [%d] check config: %.*s\n", __LINE__, getprocessid(), cstrbufGetLen(config), cstrbufGetStr(config));
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
