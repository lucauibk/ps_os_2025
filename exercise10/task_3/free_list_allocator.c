#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "allocator.h"
#include "allocator_tests.h"
#include "membench.h"

// TODO: Changes might be needed 

#define BLOCK_SIZE 1024

// Header besteht nur aus einem Zeiger auf den nächsten freien Block
typedef struct FreeBlock {
    struct FreeBlock* next;
} FreeBlock;

static void* memory_pool = NULL; 
static size_t pool_size = 0;
static FreeBlock* free_list_head = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void my_allocator_init(size_t total_size) {
	pthread_mutex_lock(&lock);
	if(memory_pool != NULL){
		pthread_mutex_unlock(&lock);
		return;
	}
	size_t aligned_size = total_size - (total_size % BLOCK_SIZE);
	pool_size = aligned_size;
	memory_pool = mmap(NULL, pool_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory_pool == MAP_FAILED) {
		memory_pool = NULL;
		pool_size = 0;
		pthread_mutex_unlock(&lock);
		exit(EXIT_FAILURE);
	}
	size_t num_blocks = pool_size / BLOCK_SIZE;
	char* block = (char*)memory_pool;
	for (size_t i = 0; i < num_blocks; ++i) {
        FreeBlock* current = (FreeBlock*)(block + i * BLOCK_SIZE);
        FreeBlock* next = (i < num_blocks - 1) ? (FreeBlock*)(block + (i + 1) * BLOCK_SIZE) : NULL;
        current->next = next;
    }

    free_list_head = (FreeBlock*)memory_pool;

    pthread_mutex_unlock(&lock);
}

void my_allocator_destroy(void) {
	pthread_mutex_lock(&lock);
	if(memory_pool != NULL){
		munmap(memory_pool, pool_size);
		memory_pool = NULL;
		pool_size = 0;
		free_list_head = NULL;
	}
	pthread_mutex_unlock(&lock);
}

void* my_malloc(size_t size) {
	if(size > BLOCK_SIZE - sizeof(FreeBlock)) {
		return NULL; // Size exceeds block size
	}
	pthread_mutex_lock(&lock);
	if(free_list_head == NULL) {
		pthread_mutex_unlock(&lock);
		return NULL; // No free blocks available
	}
	FreeBlock* block = free_list_head;
    free_list_head = block->next;

    pthread_mutex_unlock(&lock);

    return (void*)(block + 1); // Nutzdaten hinter dem Header
}

void my_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&lock);

    FreeBlock* block = ((FreeBlock*)ptr) - 1;
    block->next = free_list_head;
    free_list_head = block;

    pthread_mutex_unlock(&lock);
}


// ------

void run_bench(void) {
#if THREAD_LOCAL_ALLOCATOR
	run_membench_thread_local(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
#else
	run_membench_global(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
#endif
}

void run_tests(void) {
	test_free_list_allocator();
}
