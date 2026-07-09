#include "thread_pool.h"
#include <stdlib.h>

static void *worker_thread(void *arg) {
    struct thread_pool *pool = arg;

    while(1) {
        pthread_mutex_lock(&pool->mutex);

        while (queue_empty(&pool->queue) && !pool->shutdown) {
            pthread_cond_wait(&pool->condition, &pool->mutex);
        }

        if (pool->shutdown && queue_empty(&pool->queue)) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        struct job *job = dequeue(&pool->queue);
        pthread_mutex_unlock(&pool->mutex);
        job->function(job->arg);
    }

    return NULL;
}

int thread_pool_init(struct thread_pool *pool, int num_threads) {
    pool->num_threads = num_threads;
    pool->workers = malloc(sizeof(pthread_t) * num_threads);

    if (pool->workers == NULL) {
        return -1;
    }

    queue_init(&pool->queue);

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->condition, NULL);

    pool->shutdown = 0;

    for (int i = 0; i < num_threads; i ++) {
        if (pthread_create(&pool->workers[i], NULL, worker_thread, pool) != 0) {
            return -1;
        }
    }

    return 0;
}