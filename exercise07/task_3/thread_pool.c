#include "thread_pool.h"
job_queue* jq_init(void){
    job_queue* jq = malloc(sizeof(job_queue));
    if (jq == NULL) {
        fprintf(stderr, "Could not allocate memory for job queue.\n");
        exit(EXIT_FAILURE);
    }
    jq->head = NULL;
    jq->tail = NULL;
    return jq;
}
void jq_push(job_queue* jq, job_queue* entry) {
    if (jq->tail == NULL) {
        jq->head = entry;
        jq->tail = entry;
    } else {
        jq->tail->next = entry;
        jq->tail = entry;
    }
}
void jq_pop(job_queue* jq) {
    if (jq->head == NULL) {
        return;
    }
    job_queue* temp = jq->head;
    jq->head = jq->head->next;
    free(temp);
    if (jq->head == NULL) {
        jq->tail = NULL;
    }
}
bool jq_is_empty(job_queue* jq) {
    return jq->size == NULL;
}

void pool_create(thread_pool* pool, size_t size){
    pool->threads = malloc(size * sizeof(pthread_t) * size);
    if(!pool->threads){
        fprintf(stderr, "Could not allocate memory for thread pool.\n");
        exit(EXIT_FAILURE);
    }
    pool->num_threads = size;
    pool->job_queue = jq_init();
    pool->queue = jq;
    pool->stop = false;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    for (size_t i = 0; i < size; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    }

}

void* worker_thread(void* arg){
    thread_pool* pool = (thread_pool*)arg;
    while(true){
        pthread_mutex_lock(&pool->mutex);
        while(jq_is_empty(pool->queue) && !pool->stop){
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (pool->stop && jq_is_empty(pool->queue)) {
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        }
        job_id job = jq_deque(pool->queue);
        pthread_mutex_unlock(&pool->mutex);

        if(job != NULL){
            job->function(job->arg);
            pthread_mutex_lock(&job->mutex);
            job->finished = true;
            pthread_mutex_unlock(&job->mutex);
            pthread_cond_signal(&job->cond);
        }
    }
    return NULL;
}

job_id pool_submit(thread_pool* pool, job_function start_routine, job_arg arg) {
    static atomic_size_t next_id = ATOMIC_VAR_INIT(0);
    job_id new_job = malloc(sizeof(new_job));
    if (new_job == NULL) {
        fprintf(stderr, "Could not allocate memory for new job.\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&new_job->mutex, NULL);
    pthread_cond_init(&new_job->cond, NULL);

    new_job->function = start_routine;
    new_job->arg = arg;
    new_job->id = atomic_fetch_add(&next_id, 1);
    new_job->next = NULL;
    new_job->finished = false;

    pthread_mutex_lock(&pool->mutex);
    jq_push(pool->queue, new_job);
    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_signal(&pool->cond);
    return new_job->id;
}

void pool_await(job_id id) {
    pthread_mutex_lock(&id->mutex);
    while (!id->finished) {
        pthread_cond_wait(&id->cond, &id->mutex);
    }
    pthread_mutex_unlock(&id->mutex);
    
    pthread_mutex_destroy(&id->mutex);
    pthread_cond_destroy(&id->cond);
    free(id);
}

void pool_destroy(thread_pool* pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->stop = true;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (size_t i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    free(pool->job_queue);
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
}
