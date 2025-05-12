#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h> // for size_t
<<<<<<< HEAD
#include <pthread.h> // for pthread_t and related functions
#include <stdbool.h> // for bool type
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
=======
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
>>>>>>> e7f06ab7ba83d5cb096bcea9ff6fd76b130c2e6d

typedef void* (*job_function)(void*);
typedef void* job_arg;
typedef size_t job_id;

/***
 * This is the stub of a simple job queue.
 */
<<<<<<< HEAD
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
=======
typedef struct job_struct {
	unsigned int seed;
	uint64_t result;
	SlotMachine* slot_machine;
}job_struct;

>>>>>>> e7f06ab7ba83d5cb096bcea9ff6fd76b130c2e6d

typedef struct job_queue_entry {
	job_function function;
	job_arg argument;
	job_queue_entry* next;
}job_queue_entry;

typedef struct job_queue{
	job_queue_entry* tail;
	job_queue_entry* head;
 }job_queue;

typedef struct {
<<<<<<< HEAD
	pthread_t * threads;
	size_t size; // Number of threads in the pool
	struct job_queue_entry * queue;
	pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    bool shutdown;
=======
	job_queue_entry* queue;
	job_queue* job_queue;
	pthread_t* threads;
	size_t num_threads;
	bool stop;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
>>>>>>> e7f06ab7ba83d5cb096bcea9ff6fd76b130c2e6d
} thread_pool;

// Prototypes for REQUIRED functions
void pool_create(thread_pool* pool, size_t size);
job_id pool_submit(thread_pool* pool, job_function start_routine,
                   job_arg arg); // You need to define a datatype for the job_id (chose wisely).
void pool_await(job_id id);
void pool_destroy(thread_pool* pool);

#endif
