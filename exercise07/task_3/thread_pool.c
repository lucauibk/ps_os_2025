#include "thread_pool.h"

#include "thread_pool.h"

static struct job_queue_entry* create_job(job_function function, job_arg argument) {
    static job_id next_id = 1;
    struct job_queue_entry* job = malloc(sizeof(struct job_queue_entry));
    if (!job) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    job->id = next_id++;
    job->function = function;
    job->argument = argument;
    job->next = NULL;
    job->status = JOB_PENDING;
    pthread_mutex_init(&job->mutex, NULL);
    pthread_cond_init(&job->cond, NULL);
    return job;
}

static void* thread_worker(void* arg){
    thread_pool* pool = (thread_pool*)arg;
    while(1){
        pthread_mutex_lock(&pool->queue_mutex);
        while(pool->queue == NULL && !pool->shutdown){
            pthread_cond_wait(&pool->queue_not_empty, &pool->queue_mutex);
        }
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        struct job_queue_entry* job = pool->queue;
        pool->queue = job->next;
        pthread_mutex_unlock(&pool->queue_mutex);

        job->function(job->argument);
        
        pthread_mutex_lock(&job->mutex);
        job->status = JOB_FINISHED;
        pthread_cond_signal(&job->cond);
        pthread_mutex_unlock(&job->mutex);
    }
}

void pool_create(thread_pool* pool, size_t size) {
    pool->threads = malloc(size * sizeof(pthread_t));
    if (!pool->threads) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    pool->size = size;
    pool->queue = NULL;
    pool->shutdown = false;

    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_not_empty, NULL);

    for (size_t i = 0; i < size; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, pool);
    }
}

job_id pool_submit(thread_pool* pool, job_function start_routine, job_arg arg) {
    struct job_queue_entry* new_job = create_job(start_routine, arg);
    if(pool->queue == NULL){
        pool->queue = new_job;
    }else{
        struct job_queue_entry* current = pool->queue;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = new_job;
    }
    return new_job->id; // Return the unique ID of the job
}

void pool_await(job_id id) {
    
}

void pool_destroy(thread_pool* pool) {
    struct job_queue_entry* current = pool->queue;
    while (current != NULL) {
        struct job_queue_entry* next = current->next;
        free(current);
        current = next;
    }
    free(pool->threads);
    pool->threads = NULL;
    pool->queue = NULL;
}
