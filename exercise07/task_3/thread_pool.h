#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h> // for size_t
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

typedef void* (*job_function)(void*);
typedef void* job_arg;
typedef /* TODO */ job_id;

/***
 * This is the stub of a simple job queue.
 */
typedef struct job_struct {
	unsigned int seed;
	uint64_t result;
	SlotMachine* slot_machine;
}job_struct;


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
	job_queue_entry* queue;
	job_queue* job_queue;
	pthread_t* threads;
	size_t num_threads;
	bool stop;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} thread_pool;

// Prototypes for REQUIRED functions
void pool_create(thread_pool* pool, size_t size);
job_id pool_submit(thread_pool* pool, job_function start_routine,
                   job_arg arg); // You need to define a datatype for the job_id (chose wisely).
void pool_await(job_id id);
void pool_destroy(thread_pool* pool);

#endif
