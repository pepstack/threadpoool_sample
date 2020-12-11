/***********************************************************************
 * 2014-06-18: last modified by cheungmine
 *
 * Copyright (c) 2011, Mathias Brossard <mathias@brossard.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

/**
 * threadpool.h
 *   both for windows (required pthreads-w32) and linux
 * @update: 2019-11-12
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __USE_GNU
#  define __USE_GNU
#endif

// both for Linux and Windows
#include "unitypes.h"

#include <sched.h>
#include <pthread.h>


#ifndef POOL_MAX_THREADS
#  define POOL_MAX_THREADS             16384
#endif

#ifndef POOL_MAX_QUEUES
#  define POOL_MAX_QUEUES              131072
#endif

#ifndef POOL_DEFAULT_THREADS
#  define POOL_DEFAULT_THREADS         16
#endif

#ifndef POOL_DEFAULT_QUEUES
#  define POOL_DEFAULT_QUEUES          256
#endif

#ifndef POOL_TASK_ARG_SIZE_MAX
#  define POOL_TASK_ARG_SIZE_MAX       16384
#endif

#if !defined(__WINDOWS__) && !defined(__CYGWIN__)
/* 0-based cpu id */
# ifndef POOL_CPU_ID_MAX
#   define POOL_CPU_ID_MAX  255
# endif
#endif


/**
 * @file threadpool.h
 * @brief Threadpool Header file
 */

typedef struct threadpool_t threadpool_t;


/**
 * @file threadpool.h
 * @brief thread_context_t
 *   thread can take itself argument
 *   added by cheungmine.
 *   2014-06-17
 *   2018-11-20: task_arg for threadpool_task_t
 */
typedef struct thread_context_t
{
    int id;
    void *pool;
    pthread_t thread;
    void *thread_arg;

    struct threadpool_task_t *task;
} thread_context_t;


/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */
typedef struct threadpool_task_t
{
    void (*function)(thread_context_t *);

    ub8  flags;       /* user defined 64 bits value */

    void *argument;   /* reference to user-specified argument */

    size_t arg_size;  /* actual size in bytes stored in task_arg */
    unsigned char task_arg[0];
} threadpool_task_t;


typedef enum
{
    threadpool_success             =  0,
    threadpool_invalid             = -1,
    threadpool_lock_failure        = -2,
    threadpool_queue_full          = -3,
    threadpool_shutdown            = -4,
    threadpool_run_failure         = -5,
    threadpool_out_memory          = -6,
    threadpool_task_arg_overflow   = -7
} threadpool_error_t;


static const char* threadpool_error_messages[] = {
    "threadpool_success",
    "threadpool_invalid",
    "threadpool_lock_failure",
    "threadpool_queue_full",
    "threadpool_shutdown",
    "threadpool_run_failure",
    "threadpool_out_memory",
    "threadpool_task_arg_overflow",
    0
};


/**
 * @function threadpool_create
 * @brief Creates a threadpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue for tasks.
 * @param thread_args  array of arguments with count of thread_count, NULL if ignored.
 * @param task_arg_size pre-allocated buffer for per-task if it > 0.
 * @return a newly created thread pool or NULL
 */
extern threadpool_t *threadpool_create (int thread_count, int queue_size, int stack_size, int affinity_cpus, void **thread_args, size_t task_arg_size);


/**
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool     Thread pool to which add the task.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @param flags    Caller-specified parameter.
 * @return 0 if all goes well, negative values in case of error (@see
 * threadpool_error_t for codes).
 */
extern int threadpool_add (threadpool_t *pool, void (*routine)(thread_context_t *), void *argument, void *task_arg, int arg_size, ub8 flags);


/**
 * @function threadpool_unused_queues
 * @brief get unused size of queues in thread pool
 * @param pool     Thread pool to which get size of queues
 * @return 0 if queues are full, positive values for unused queues.
 *    negative values in case of error (@see threadpool_error_t for codes).
 */
extern int threadpool_unused_queues (threadpool_t *pool);

/**
 * @function threadpool_get_threads_count
 * @brief get size of pool (number of threads)
 * @param pool     Thread pool to which get number of threads
 * @return number of threads.
 */
extern int threadpool_get_threads_count (threadpool_t *pool);


/**
 * @function threadpool_get_context
 * @brief get thread_context_t by id
 * @param pool     Thread pool
 * @param id       thread index 1 based
 * @return 0 if queues are full, positive values for unused queues.
 *    negative values in case of error (@see threadpool_error_t for codes).
 */
extern thread_context_t * threadpool_get_context (threadpool_t *pool, int id);


/**
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 * @param flags Unused parameter.
 */
extern int threadpool_destroy (threadpool_t *pool);

/**
 * pthread_attr_init_config
 *   pthread_attr_init and config
 */
extern int pthread_attr_init_config (pthread_attr_t *pattr, int stack_size, int scope, int joinable);

#if !defined(__WINDOWS__) && !defined(__CYGWIN__)

/**
 * pthread_set_affinity_cpus
 *   计算线程亲和的 cpu id
 *
 *   thrid: 线程编号, 0,1,2,3,...
 *   affinity_cpus: 亲和度, 表示每个线程可运行在几个 cpu 上. 亲和度=４: 表示每个线程可运行在 4 个 cpu 上
 *
 * set cpu affinity for thread
 *   https://blog.csdn.net/guotianqing/article/details/80958281
 *
 * 命令查看进程的 cpu 亲和力列表: # taskset -cp pid
 */
extern int thread_set_affinity_cpus (int thrid, int affinity_cpus, cpu_set_t *cpuset);


/**
 * thread_check_affinity_cpus
 *   Check the actual affinity mask assigned to the thread
 */
extern int thread_check_affinity_cpus (pthread_t thread, int *cpuid_list, int cnt);

#endif

#if defined(__cplusplus)
}
#endif

#endif /* _THREADPOOL_H_ */
