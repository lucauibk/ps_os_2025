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

<<<<<<< HEAD
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
=======
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
>>>>>>> e7f06ab7ba83d5cb096bcea9ff6fd76b130c2e6d
}
