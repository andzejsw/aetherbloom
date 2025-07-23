#include "threadpool.h"
#include "chunkmesh.h"
#include "world.h"
#include <stdlib.h>
#include <stdio.h>

void threadpool_init(ThreadPool *pool, int num_threads) {
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond_nonempty, NULL);
    pthread_cond_init(&pool->cond_nonfull, NULL);
    pool->queue_count = 0;
    pool->queue_front = 0;
    pool->queue_back = 0;
    pool->shutdown = false;

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, threadpool_worker, pool);
    }
}

void threadpool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->cond_nonempty);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pool->threads[i] != 0) {
            pthread_join(pool->threads[i], NULL);
        }
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond_nonempty);
    pthread_cond_destroy(&pool->cond_nonfull);
}

bool threadpool_add_task(ThreadPool *pool, struct Chunk *chunk) {
    pthread_mutex_lock(&pool->mutex);

    while (pool->queue_count >= MAX_QUEUE && !pool->shutdown) {
        pthread_cond_wait(&pool->cond_nonfull, &pool->mutex);
    }

    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }

    pool->queue[pool->queue_back] = chunk;
    pool->queue_back = (pool->queue_back + 1) % MAX_QUEUE;
    pool->queue_count++;

    pthread_cond_signal(&pool->cond_nonempty);
    pthread_mutex_unlock(&pool->mutex);

    return true;
}

void *threadpool_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (true) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->queue_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond_nonempty, &pool->mutex);
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        struct Chunk *chunk = pool->queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % MAX_QUEUE;
        pool->queue_count--;

        pthread_cond_signal(&pool->cond_nonfull);
                pthread_mutex_unlock(&pool->mutex);

        pthread_mutex_lock(&chunk->mesh->mutex);
        chunkmesh_generate(chunk->mesh);
        chunk->mesh->flags.dirty = false;
        chunk->mesh->flags.finalize = true;
        pthread_mutex_unlock(&chunk->mesh->mutex);

        pthread_mutex_lock(&pool->mutex);
        chunk->world->throttles.mesh.count--;
        pthread_mutex_unlock(&pool->mutex);
    }

    return NULL;
}
