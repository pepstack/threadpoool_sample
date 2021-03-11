/**
 * @filename   main.c
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @create     2019-11-11
 */
#include "timeut.h"
#include "misc.h"

#include "threadpool.h"

#define TEST_THREADS_COUNT  20
#define TEST_QUEUES_COUNT   512
#define TEST_STACK_SIZE     0
#define TEST_TASK_BUFSIZE   1024


// pseudocode simulates as a database connection
typedef struct {
    int connected;
    int dbconnid;
} db_connection_t;


// pseudocode simulates as a task argument
typedef struct {
    int len;
    char name[0];
} pertask_argument_t;


static void doThreadTask(thread_context_t *thread_ctx)
{
    ub8 flags = thread_ctx->task->flags;

    // 每个线程都拥有一个数据库连接
    // 从任务队列（TEST_QUEUES_COUNT）中取出一个任务交给一个线程去执行
    // dbconn 仅仅被当前线程使用，是线程安全的，不会发生竞争问题
    db_connection_t * dbconn = (db_connection_t *)thread_ctx->thread_arg;

    // 每个任务附加的用户数据。用户负责分配(malloc)和释放(free)
    pertask_argument_t * argument = (pertask_argument_t *)thread_ctx->task->argument;

    // task_arg 是任务附加数据，与argument不同的是，这个数据不需要分配和释放，存储在任务的
    // 缓存数据块中(TEST_TASK_BUFSIZE)。防止频繁分配和释放内存。
    // 尽量使用task_arg参数而不是 argument。
    // thread_ctx->task->task_arg

    printf("\n[worker] doThreadTask#%"PRIu64":\n", thread_ctx->task->flags);
    printf("  task_arg =%.*s\n", (int)thread_ctx->task->arg_size - 1, (const char*)thread_ctx->task->task_arg);
    printf("  dbconnid =%d\n", dbconn->dbconnid);
    printf("  argument =%.*s\n", argument->len, argument->name);

    // do some circumlocution work here
    sleep_msec(100);

    // 释放用户指定的任务数据
    free(argument);
}


int main(int argc, char *argv[])
{
    WINDOWS_CRTDBG_ON

    int i, err, count = 0;

    // optional task_arg can not large than TEST_TASK_BUFSIZE
    char task_arg[TEST_TASK_BUFSIZE];
    int task_arg_len;

    printf("[main] create connection pool to database.\n");
    db_connection_t * dbpool[TEST_THREADS_COUNT];
    for (i = 0; i < TEST_THREADS_COUNT; i++) {
        // create a connection to database
        dbpool[i] = (db_connection_t *)malloc(sizeof(db_connection_t));
        dbpool[i]->connected = 1;
        dbpool[i]->dbconnid = (i + 1);
    }

    printf("[main] create threadpool with task buffer size=%d.\n", TEST_TASK_BUFSIZE);
    threadpool_t * pool = threadpool_create(TEST_THREADS_COUNT, TEST_QUEUES_COUNT, TEST_STACK_SIZE, 0, (void**)dbpool, TEST_TASK_BUFSIZE);
    if (!pool) {
        printf("[main] threadpool_create failed !\n");
        exit(EXIT_FAILURE);
    }

    while (count++ < 1000) {
        pertask_argument_t * argument = (pertask_argument_t *)malloc(sizeof(pertask_argument_t) + 256);
        argument->len = snprintf_chkd_V1(argument->name, 256, "This is per-task argument for task#%d", count);

        task_arg_len = snprintf_chkd_V1(task_arg, sizeof(task_arg), "%s", "task_arg was copied if threadpool_add success.");

        err = threadpool_add(pool, doThreadTask, argument, task_arg, task_arg_len + 1, count);
        if (err) {
            printf("[main] threadpool_add error: %s\n", threadpool_error_messages[-err]);
        }
        else {
            printf("[main] threadpool_add task#%d success\n", count);
        }

        sleep_msec(10);
    }

    printf("[main] destroy threadpool.\n");
    threadpool_destroy(pool);

    printf("[main] close all connections in pool.\n");
    for (i = 0; i < TEST_THREADS_COUNT; i++) {
        free(dbpool[i]);
    }
    return 0;
}