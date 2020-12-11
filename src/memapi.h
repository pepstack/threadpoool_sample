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
 * @filename   memapi.h
 *  memory helper api both for linux and windows.
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.10
 * @create     2018-10-25 09:09:10
 * @update     2020-12-07 10:27:33
 */
#ifndef MEMAPI_H_INCLUDED
#define MEMAPI_H_INCLUDED

#if defined(__cplusplus)
extern "C"
{
#endif

#include "mscrtdbg.h"

#include <assert.h>  /* assert */
#include <string.h>  /* memset */
#include <stdio.h>   /* printf, perror */
#include <limits.h>  /* realpath, PATH_MAX=4096 */
#include <stdbool.h> /* memset */
#include <ctype.h>
#include <stdlib.h>  /* malloc, alloc */
#include <malloc.h>  /* alloca */
#include <errno.h>


#if defined (_WIN32)
    # define MEMAPI_ENABLE_ALLOCA    0

    /* Microsoft Windows */
    # if !defined(__MINGW__)
        # pragma warning(push)
        # pragma warning(disable : 4996)
    # endif
#else
    # define MEMAPI_ENABLE_ALLOCA    1

    # ifdef MEMAPI_USE_LIBJEMALLOC
        /* need to link: libjemalloc.a */
        # include <jemalloc/jemalloc.h>
    # endif
#endif


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


#define memapi_align_bsize(bsz, alignsize)  \
        ((size_t)((((size_t)(bsz)+(alignsize)-1)/(alignsize))*(alignsize)))

#define memapi_align_psize(psz)  memapi_align_bsize(psz, sizeof(void *))


#define memapi_oom_check(p) do { \
        if (!(p)) { \
            printf("FATAL: (memapi.h:%d) out of memory.\n", __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)


/**
 * mem_alloc_zero() allocates memory for an array of nmemb elements of
 *  size bytes each and returns a pointer to the allocated memory.
 * THE MEMORY IS SET TO ZERO.
 */
STATIC_INLINE void * mem_alloc_zero (int nmemb, size_t size)
{
    void * p =
        #ifdef MEMAPI_USE_LIBJEMALLOC
            je_calloc(nmemb, size);
        #else
            calloc(nmemb, size);
        #endif

    memapi_oom_check(p);
    return p;
}


/**
 * mem_alloc_unset() allocate with THE MEMORY NOT BE INITIALIZED.
 */
STATIC_INLINE void * mem_alloc_unset (size_t size)
{
    void * p =
        #ifdef MEMAPI_USE_LIBJEMALLOC
            je_malloc(size);
        #else
            malloc(size);
        #endif
    memapi_oom_check(p);
    return p;
}


/**
 * mem_realloc() changes the size of the memory block pointed to by ptr
 *  to size bytes. The contents will be unchanged in the range from the
 *  start of the region up to the minimum of the old and new sizes.
 * If the new size is larger than the old size,
 *  THE ADDED MEMORY WILL NOT BE INITIALIZED.
 */
STATIC_INLINE void * mem_realloc (void * ptr, size_t size)
{
    void *np =
        #ifdef MEMAPI_USE_LIBJEMALLOC
            je_realloc(ptr, size);
        #else
            realloc(ptr, size);
        #endif
    memapi_oom_check(np);
    return np;
}


STATIC_INLINE char * mem_strdup (const char *s)
{
    if (s) {
        size_t sz = strlen(s) + sizeof(char);
        char * d = (char *) mem_alloc_unset(sz);
        memcpy(d, s, sz);
        return d;
    }
    return 0;
}


/**
 * mem_free() frees the memory space pointed to by ptr, which must have
 *  been returned by a previous call to malloc(), calloc() or realloc().
 *  IF PTR IS NULL, NO OPERATION IS PERFORMED.
 */
STATIC_INLINE void mem_free (void * ptr)
{
    if (ptr) {
    #ifdef MEMAPI_USE_LIBJEMALLOC
        je_free(ptr);
    #else
        free(ptr);
    #endif
    }
}


/**
 * mem_free_s() frees the memory pointed by the address of ptr and set
 *  ptr to zero. it is a safe version if mem_free().
 */
STATIC_INLINE void mem_free_s (void **pptr)
{
    if (pptr) {
        void *ptr = *pptr;

        if (ptr) {
            *pptr = 0;

        #ifdef MEMAPI_USE_LIBJEMALLOC
            je_free(ptr);
        #else
            free(ptr);
        #endif
        }
    }
}


typedef struct {
    void (*freebufcb)(void *);
    size_t bufsize;
    char buffer[0];
} alloca_buf_t;


STATIC_INLINE char * alloca_buf_new (size_t smallbufsize)
{
    size_t bufsize = memapi_align_psize(smallbufsize);
    alloca_buf_t *albuf =
#if MEMAPI_ENABLE_ALLOCA == 1
        (alloca_buf_t *)alloca(sizeof(alloca_buf_t) + sizeof(char) * bufsize);
#else
        NULL;
#endif

    if (albuf) {
        albuf->freebufcb = NULL;
    } else {
        albuf = (alloca_buf_t *) mem_alloc_unset(sizeof(alloca_buf_t) + sizeof(char) * bufsize);
        albuf->freebufcb = mem_free;
    }
    albuf->bufsize = bufsize;
    return (char *)albuf->buffer;
}


STATIC_INLINE void alloca_buf_free (char *buffer)
{
    alloca_buf_t *albuf = (alloca_buf_t *)(buffer - sizeof(alloca_buf_t));
    if (albuf->freebufcb) {
        albuf->freebufcb(albuf);
    }
}

#if defined (_MSC_VER)
    # pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif /* MEMAPI_H_INCLUDED */
