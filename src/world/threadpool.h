#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>
#include "../util/types.h"
#include "chunk.h"

#define MAX_THREADS 8
#define MAX_QUEUE 256

typedef struct {
    pthread_t threads[MAX_THREADS];
    struct Chunk *queue[MAX_QUEUE];
    int queue_count;
    int queue_front;
    int queue_back;
    pthread_mutex_t mutex;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
    bool shutdown;
} ThreadPool;

void threadpool_init(ThreadPool *pool, int num_threads);
void threadpool_destroy(ThreadPool *pool);
bool threadpool_add_task(ThreadPool *pool, struct Chunk *chunk);
void *threadpool_worker(void *arg);

#endif // THREADPOOL_H
