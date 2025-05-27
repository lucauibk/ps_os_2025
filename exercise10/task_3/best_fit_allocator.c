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

static size_t pool_size = 0;
static _Thread_local void* memory_pool = NULL;
static _Thread_local BlockHeader* free_list = NULL;

void my_allocator_init(size_t size) {
    if (memory_pool != NULL) return;

    pool_size = ALIGN(size);
    memory_pool = mmap(NULL, pool_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        memory_pool = NULL;
        return;
    }

    free_list = (BlockHeader*)memory_pool;
    free_list->size = pool_size;
    free_list->next = NULL;
    free_list->free = true;
}



void my_allocator_destroy(void) {
    if (memory_pool) {
        munmap(memory_pool, pool_size);
        memory_pool = NULL;
        free_list = NULL;
    }
}



void* my_malloc(size_t size) {
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

    if (!best) return NULL;

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

    return (void*)((char*)best + HEADER_SIZE);
}



void my_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&alloc_mutex);
    BlockHeader* block = (BlockHeader*)((char*)ptr - HEADER_SIZE);
    block->free = true;

    // Sortiert nach Adresse einfügen
    if (!free_list || block < free_list) {
        block->next = free_list;
        free_list = block;
    } else {
        BlockHeader* curr = free_list;
        while (curr->next && curr->next < block) {
            curr = curr->next;
        }
        block->next = curr->next;
        curr->next = block;
    }

    // Merge benachbarter Blöcke
    BlockHeader* curr = free_list;
    while (curr && curr->next) {
        char* curr_end = (char*)curr + curr->size;
        if ((char*)curr->next == curr_end && curr->next->free) {
            curr->size += curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    pthread_mutex_unlock(&alloc_mutex);
}

    if (!ptr) return;

    BlockHeader* block = (BlockHeader*)((char*)ptr - HEADER_SIZE);
    block->free = true;
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
