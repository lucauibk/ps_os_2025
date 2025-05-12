#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h> // for size_t
#include <pthread.h> // for pthread_t and related functions
#include <stdbool.h> // for bool type
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef void* (*job_function)(void*);
typedef void* job_arg;
typedef size_t job_id;

/***
 * This is the stub of a simple job queue.
 */
typedef enum{
	JOB_PENDING,
	JOB_RUNNING,
	JOB_FINISHED,
} job_status;

struct job_queue_entry {
	job_id id;
	job_function function;
	job_arg argument;
	struct job_queue_entry* next;
	// Synchronisation für `pool_await`
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    job_status status;
};

/***
 * This is the stub for the thread pool that uses the queue.
 * Implement at LEAST the Prototype functions below.
 */

typedef struct {
	pthread_t * threads;
	size_t size; // Number of threads in the pool
	struct job_queue_entry * queue;
	pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    bool shutdown;
} thread_pool;

// Prototypes for REQUIRED functions
void pool_create(thread_pool* pool, size_t size);
job_id pool_submit(thread_pool* pool, job_function start_routine,
                   job_arg arg); // You need to define a datatype for the job_id (chose wisely).
void pool_await(job_id id);
void pool_destroy(thread_pool* pool);

#endif
