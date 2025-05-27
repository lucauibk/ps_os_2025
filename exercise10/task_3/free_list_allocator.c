#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "allocator.h"
#include "allocator_tests.h"
#include "membench.h"

#define BLOCK_SIZE 1024

typedef struct FreeBlock {
    struct FreeBlock* next;
} FreeBlock;

// Thread-local allocator state
_Thread_local static void* memory_pool = NULL;
_Thread_local static size_t pool_size = 0;
_Thread_local static FreeBlock* free_list_head = NULL;

void my_allocator_init(size_t total_size) {
	if (memory_pool != NULL) {
		return; // Already initialized for this thread
	}

	size_t aligned_size = total_size - (total_size % BLOCK_SIZE);
	pool_size = aligned_size;
	memory_pool = mmap(NULL, pool_size, PROT_READ | PROT_WRITE,
	                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory_pool == MAP_FAILED) {
		memory_pool = NULL;
		pool_size = 0;
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
}

void my_allocator_destroy(void) {
	if (memory_pool != NULL) {
		munmap(memory_pool, pool_size);
		memory_pool = NULL;
		pool_size = 0;
		free_list_head = NULL;
	}
}

void* my_malloc(size_t size) {
	if (size > BLOCK_SIZE - sizeof(FreeBlock)) {
		return NULL;
	}
	if (free_list_head == NULL) {
		return NULL;
	}
	FreeBlock* block = free_list_head;
	free_list_head = block->next;

	return (void*)(block + 1); // Return data region
}

void my_free(void* ptr) {
	if (!ptr) return;

	FreeBlock* block = ((FreeBlock*)ptr) - 1;
	block->next = free_list_head;
	free_list_head = block;
}

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
