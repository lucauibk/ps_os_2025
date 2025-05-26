#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "allocator.h"
#include "allocator_tests.h"
#include "membench.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct BlockHeader {
    size_t size; // Größe des Blocks, inkl. Header
    struct BlockHeader* next;
    bool free;
} BlockHeader;

#define HEADER_SIZE ALIGN(sizeof(BlockHeader))

static void* memory_pool = NULL;
static size_t pool_size = 0;
static BlockHeader* free_list = NULL;
static pthread_mutex_t alloc_mutex = PTHREAD_MUTEX_INITIALIZER;


void my_allocator_init(size_t size) {
    pthread_mutex_lock(&alloc_mutex);
    pool_size = ALIGN(size);
    memory_pool = mmap(NULL, pool_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        memory_pool = NULL;
        pthread_mutex_unlock(&alloc_mutex);
        return;
    }

    free_list = (BlockHeader*)memory_pool;
    free_list->size = pool_size;
    free_list->next = NULL;
    free_list->free = true;
    pthread_mutex_unlock(&alloc_mutex);
}

void my_allocator_destroy(void) {
    pthread_mutex_lock(&alloc_mutex);
    if (memory_pool) {
        munmap(memory_pool, pool_size);
        memory_pool = NULL;
        free_list = NULL;
        pool_size = 0;
    }
    pthread_mutex_unlock(&alloc_mutex);
}


void* my_malloc(size_t size) {
    pthread_mutex_lock(&alloc_mutex);
    size_t total_size = ALIGN(size) + HEADER_SIZE;
    BlockHeader *best = NULL, *prev = NULL, *curr = free_list, *best_prev = NULL;

    while (curr) {
        if (curr->free && curr->size >= total_size) {
            if (!best || curr->size < best->size) {
                best = curr;
                best_prev = prev;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    if (!best) {
        pthread_mutex_unlock(&alloc_mutex);
        return NULL;
    }

    if (best->size >= total_size + HEADER_SIZE + 8) {
        BlockHeader* split = (BlockHeader*)((char*)best + total_size);
        split->size = best->size - total_size;
        split->next = best->next;
        split->free = true;

        best->size = total_size;
        best->next = split;
    }

    best->free = false;

    if (best_prev)
        best_prev->next = best->next;
    else
        free_list = best->next;

    pthread_mutex_unlock(&alloc_mutex);
    return (void*)((char*)best + HEADER_SIZE);
}


void my_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&alloc_mutex);
    BlockHeader* block = (BlockHeader*)((char*)ptr - HEADER_SIZE);
    block->free = true;

    // Füge Block sortiert ein (optional)
    block->next = free_list;
    free_list = block;

    // Mergen
    BlockHeader* curr = free_list;
    while (curr) {
        char* curr_end = (char*)curr + curr->size;
        BlockHeader* next = curr->next;

        if (next && (char*)next == curr_end && next->free) {
            curr->size += next->size;
            curr->next = next->next;
        } else {
            curr = curr->next;
        }
    }

    pthread_mutex_unlock(&alloc_mutex);
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
	test_best_fit_allocator();
}
