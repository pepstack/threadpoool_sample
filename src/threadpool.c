/**********************************************************************
 * 2018-11-20: last modified by cheungmine
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
 *
 * cpu affinity:
 *   http://man7.org/linux/man-pages/man3/pthread_create.3.html
 *   http://man7.org/linux/man-pages/man3/pthread_setaffinity_np.3.html
 *********************************************************************/

/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 *
 * @update: 2019-11-12
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* memcpy */

#include "threadpool.h"


#if !defined(__WINDOWS__)
# include <unistd.h>
# include <sys/sysinfo.h>
#endif


#if defined(__WINDOWS__) && !defined(__CYGWIN__)
# if defined (_MSC_VER)
// link to pthread-w32 lib only for MS Windows
#   pragma comment(lib, "pthreadVC2.lib")
# else
#   pragma comment(lib, "pthreadGC2.lib")
# endif
#endif


/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var shutdown     Flag indicating if the pool is shutting down
 */
struct threadpool_t
{
    pthread_mutex_t lock;
    pthread_cond_t notify;

    int head;
    int tail;
    int count;
    int shutdown;
    int started;

    int thread_count;
    int queue_size;
    int task_arg_size;
    int task_size;  /* total sizeof task */

    threadpool_task_t *queues;

    thread_context_t thread_ctxs[0];
};


#if defined(__WINDOWS__)
# define pool_count_get(pool)  InterlockedCompareExchange(&pool->count, 0, 0)
# define pool_count_add(pool)  InterlockedIncrement(&pool->count)
# define pool_count_sub(pool)  InterlockedDecrement(&pool->count)
#else
# define pool_count_get(pool)  __sync_add_and_fetch(&pool->count, 0)
# define pool_count_add(pool)  __sync_add_and_fetch(&pool->count, 1)
# define pool_count_sub(pool)  __sync_sub_and_fetch(&pool->count, 1)
#endif


#define threadpool_get_task_at(pool, offset)  \
    ((threadpool_task_t *) ((unsigned char *) pool->queues + offset * pool->task_size))


thread_context_t * threadpool_get_context (threadpool_t *pool, int id)
{
	return &(pool->thread_ctxs[id - 1]);
}


/**
 * @function void *threadpool_run(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *threadpool_run (void *threadpool);

int threadpool_free(threadpool_t *pool);

threadpool_t *threadpool_create(int thread_count, int queue_size, int stack_size, int affinity_cpus, void **thread_args, size_t task_arg_size)
{
    int i;

    threadpool_t *pool = NULL;

    pthread_attr_t attr;

#if !defined(__WINDOWS__) && !defined(__CYGWIN__)
    cpu_set_t cpuset;
#endif

    /* Check thread_count for negative or otherwise very big input parameters */
    if (thread_count < 0 || thread_count > POOL_MAX_THREADS) {
        goto err;
    }
    if (thread_count == 0) {
        thread_count = POOL_DEFAULT_THREADS;
    }

    /* Check queue_size for negative or otherwise very big input parameters */
    if (queue_size < 0 || queue_size > POOL_MAX_QUEUES) {
        goto err;
    }

    if (queue_size == 0) {
        queue_size = POOL_DEFAULT_QUEUES;
    }

    /* Check task_arg_size for negative or otherwise very big input parameters */
    if (task_arg_size == (size_t)(-1) || task_arg_size > POOL_TASK_ARG_SIZE_MAX) {
        goto err;
    }

    /* create threadpool */
    if ( (pool = (threadpool_t *) malloc (sizeof(threadpool_t) +
            sizeof(thread_context_t) * thread_count +
            (sizeof(threadpool_task_t) + task_arg_size) * queue_size)
        ) == NULL ) {
        goto err;
    }

    /* Initialize */
    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->task_arg_size = (int) task_arg_size;
    pool->task_size = (int) (sizeof(threadpool_task_t) + task_arg_size);
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;
    pool->queues = (threadpool_task_t *) (& pool->thread_ctxs[thread_count]);

    /* Initialize mutex and conditional variable first */
    if ((pthread_mutex_init (&(pool->lock), NULL) != 0) ||
       (pthread_cond_init (&(pool->notify), NULL) != 0)) {
        goto err;
    }

	/* http://man7.org/linux/man-pages/man3/pthread_create.3.html */
	if (pthread_attr_init_config(&attr, 0, PTHREAD_SCOPE_SYSTEM, PTHREAD_CREATE_JOINABLE) != 0) {
		goto err;
	}

    /* Start worker threads */
    for (i = 0; i < thread_count; i++) {
        thread_context_t * pctx = & pool->thread_ctxs[i];

        /* set thread id: 1 based */
        pctx->id = i + 1;

        /* set pool to each thread context */
        pctx->pool = (void*) pool;

        /* assign thread argument if valid */
        if (thread_args) {
            pctx->thread_arg = thread_args[i];
        } else {
            pctx->thread_arg = 0;
        }

		/* Set affinity mask to include CPUs 0 to 7 */
# if !defined(__WINDOWS__) && !defined(__CYGWIN__)
		if (affinity_cpus > 0) {
			thread_set_affinity_cpus(pctx->id, affinity_cpus, &cpuset);

			if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset) != 0) {
				printf("pthread_attr_setaffinity_np error: %s\n", strerror(errno));
				threadpool_destroy(pool);
				pthread_attr_destroy(&attr);
				return NULL;
			}
		}
# endif

        if ( pthread_create (& pctx->thread, &attr, threadpool_run, (void*) pctx) != 0) {
            threadpool_destroy(pool);
            pthread_attr_destroy(&attr);
            return NULL;
        } else {
# if !defined(__WINDOWS__) && !defined(__CYGWIN__)
			CPU_ZERO(&cpuset);

            if (pthread_getaffinity_np (pctx->thread, sizeof(cpu_set_t), &cpuset) != 0) {
				printf("pthread_attr_setaffinity_np error: %s\n", strerror(errno));
				threadpool_destroy(pool);
				pthread_attr_destroy(&attr);
				return NULL;
			}
# endif

            pool->started++;
        }
    }

	/* Destroy the thread attributes object, since it is no longer needed */
	if (pthread_attr_destroy(&attr) != 0) {
		printf("pthread_attr_destroy error: %s\n", strerror(errno));
		threadpool_destroy(pool);
		return NULL;
	}

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add (threadpool_t *pool, void (*function)(thread_context_t *), void *argument, void *task_arg, int arg_size, ub8 flags)
{
    int err = 0;
    int next;

    if ( pool == NULL || function == NULL ) {
        return threadpool_invalid;
    }

    if (arg_size > pool->task_arg_size) {
        return threadpool_task_arg_overflow;
    }

    if (pthread_mutex_lock (&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    next = pool->tail + 1;
    next = (next == pool->queue_size) ? 0 : next;

    do {
        /* Are we shutting down ? */
        if (pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Are we full ? */
        if (pool_count_get(pool) == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Add task to queues */
        do {
            threadpool_task_t *ptask = threadpool_get_task_at(pool, pool->tail);

            ptask->function = function;
            ptask->argument = argument;

            if (arg_size > 0) {
                /* task_arg is enabled */
                ptask->arg_size = arg_size;
                memcpy((void*) ptask->task_arg, task_arg, arg_size);
            } else {
                /* task_arg not enabled */
                ptask->arg_size = 0;
            }

            /* Use flags to determine whether argument or task_arg is enabled */
            ptask->flags = flags;
        } while(0);

        pool->tail = next;

        /* pool->count += 1; */
        pool_count_add(pool);

        /* pthread_cond_broadcast */
        if (pthread_cond_signal (&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    if (pthread_mutex_unlock (&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}


int threadpool_unused_queues (threadpool_t *pool)
{
    if ( !pool || pool->shutdown ) {
        return threadpool_invalid;
    } else {
        return (pool->queue_size - pool_count_get(pool));
    }
}


int threadpool_get_threads_count (threadpool_t *pool)
{
    return pool ? pool->thread_count : 0;
}


int threadpool_destroy (threadpool_t *pool)
{
    int i, err = 0;

    if (pool == NULL) {
        return threadpool_invalid;
    }

    if (pthread_mutex_lock (&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        if (pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        pool->shutdown = 1;

        /* Wake up all worker threads */
        if ((pthread_cond_broadcast(&(pool->notify)) != 0) ||
           (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        for (i = 0; i < pool->thread_count; i++) {
            if (pthread_join (pool->thread_ctxs[i].thread, NULL) != 0) {
                err = threadpool_run_failure;
            }
        }
    } while(0);

    if (pthread_mutex_unlock (&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    /* Only if everything went well do we deallocate the pool */
    if (!err) {
        threadpool_free (pool);
    }
    return err;
}

int threadpool_free (threadpool_t *pool)
{
    if (pool == NULL || pool->started > 0) {
        return -1;
    }

    pthread_mutex_lock (&(pool->lock));
    pthread_mutex_destroy (&(pool->lock));
    pthread_cond_destroy (&(pool->notify));

    free(pool);
    return 0;
}

/**
 * each thread run function
 */
static void *threadpool_run (void * param)
{
    thread_context_t *thread_ctx = (thread_context_t *) param;
    threadpool_t *pool = thread_ctx->pool;
    threadpool_task_t *taskcpy = (threadpool_task_t *) malloc(pool->task_size);

    for (;;) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        while ((pool_count_get(pool) == 0) && (!pool->shutdown)) {
            pthread_cond_wait (&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            break;
        }

        /* Grab our task */
        memcpy(taskcpy, threadpool_get_task_at(pool, pool->head), pool->task_size);

        thread_ctx->task = (threadpool_task_t *) taskcpy;

        pool->head += 1;
        pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;

        /* pool->count -= 1; */
        pool_count_sub(pool);

        /* Unlock */
        pthread_mutex_unlock (&(pool->lock));

        /* Get to work */
        (*(taskcpy->function)) (thread_ctx);
    }

    pool->started--;
    free(taskcpy);

    pthread_mutex_unlock (&(pool->lock));
    pthread_exit(0);

    return 0;
}


int pthread_attr_init_config (pthread_attr_t *pattr, int stack_size, int scope, int joinable)
{
	if (pthread_attr_init(pattr) != 0) {
		printf("pthread_attr_init error: %s\n", strerror(errno));
		return -1;
	}

	if (stack_size > 0) {
		if (pthread_attr_setstacksize(pattr, stack_size) != 0) {
			printf("pthread_attr_setstacksize error: %s\n", strerror(errno));
			pthread_attr_destroy(pattr);
			return -1;
		}
	}

	/**
	 *  scope specified the value PTHREAD_SCOPE_PROCESS, which is not supported on Linux.
	 *  Linux ONLY supports PTHREAD_SCOPE_SYSTEM, not the PTHREAD_SCOPE_PROCESS.
	 */
    if (pthread_attr_setscope(pattr, scope) != 0 && scope != PTHREAD_SCOPE_PROCESS) {
        if (errno) {
		    printf("pthread_attr_setscope error: %s\n", strerror(errno));
		    pthread_attr_destroy(pattr);
		    return -1;
        }
	}

	if (pthread_attr_setdetachstate(pattr, joinable) != 0) {
		printf("pthread_attr_setdetachstate error: %s\n", strerror(errno));
		pthread_attr_destroy(pattr);
		return -1;
	}

	return 0;
}

#if !defined(__WINDOWS__) && !defined(__CYGWIN__)

/**
 * thread_set_affinity_cpus
 *   计算线程亲和的 cpu id
 *
 *   thrid: 线程编号, 0,1,2,3,...
 *   affinity_cpus: 亲和度, 表示每个线程可运行在几个 cpu 上. 亲和度=４: 表示每个线程可运行在 4 个 cpu 上
 *
 * set cpu affinity for thread
 *   https://blog.csdn.net/guotianqing/article/details/80958281
 */
int thread_set_affinity_cpus (int thrid, int affinity_cpus, cpu_set_t *cpuset)
{
	int cpu_id;

	CPU_ZERO(cpuset);

	/* 可用的 cpu 数 */
	int onln_cpus = get_nprocs();

	/* 把可用的　cpu 按照亲和度分为组: cpu_grps = 2 [0,1] */
	int cpu_grps = 1;

	if (affinity_cpus == -1) {
		affinity_cpus = POOL_CPU_ID_MAX + 1;
	} else if (affinity_cpus > POOL_CPU_ID_MAX) {
		affinity_cpus = POOL_CPU_ID_MAX + 1;
	}

	if (affinity_cpus > 0) {
		cpu_grps = onln_cpus / affinity_cpus;
	}

	if (cpu_grps < 1) {
		cpu_grps = 1;
	}

	/* 计算任意一个线程在亲和在哪组 cpu 上　*/
	int grp_id = thrid % cpu_grps;

	/* 给定 grp_id, 计算其所有的　cpu_id */
	for (cpu_id = affinity_cpus * grp_id; cpu_id < affinity_cpus * (grp_id + 1); cpu_id++) {
		CPU_SET(cpu_id, cpuset);
	}

	return onln_cpus;
}


int thread_check_affinity_cpus (pthread_t thread, int *cpuid_list, int cnt)
{
	int j, cid = 0;

	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);

	if (pthread_getaffinity_np (thread, sizeof(cpu_set_t), &cpuset) != 0) {
		return -1;
	} else {
		for (j = 0; j < CPU_SETSIZE; j++) {
			if (CPU_ISSET(j, &cpuset)) {
				if (cid < cnt) {
					cpuid_list[cid++] = j;
				}
			}
		}
	}

	return cid;
}

#endif