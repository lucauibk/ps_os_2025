#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include "myqueue.h"

typedef struct {
    int id;
    myqueue* queue;
    pthread_mutex_t* mutex;
    int result;
} consumer_arg;

void* consumer_thread(void* arg) {
    consumer_arg* carg = (consumer_arg*)arg;
    int sum = 0;

    while (1) {
        pthread_mutex_lock(carg->mutex);
        if (!myqueue_is_empty(carg->queue)) {
            int val = myqueue_pop(carg->queue);
            pthread_mutex_unlock(carg->mutex);

            if (val == INT_MAX) {
                printf("Consumer %d sum: %d\n", carg->id, sum);
                carg->result = sum;
                return NULL;
            }

            sum += val;
        } else {
            pthread_mutex_unlock(carg->mutex);
            // Sleep briefly to avoid busy waiting
            sched_yield(); // or usleep(100)
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_consumers> <num_elements>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int c = atoi(argv[1]);
    int n = atoi(argv[2]);

    pthread_t* threads = malloc(sizeof(pthread_t) * c);
    consumer_arg* args = malloc(sizeof(consumer_arg) * c);
    myqueue queue;
    myqueue_init(&queue);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    // Create consumer threads
    for (int i = 0; i < c; ++i) {
        args[i].id = i;
        args[i].queue = &queue;
        args[i].mutex = &mutex;
        args[i].result = 0;
        pthread_create(&threads[i], NULL, consumer_thread, &args[i]);
    }

    // Producer: push n alternating values (i and -i)
    for (int i = 1; i <= n; ++i) {
        pthread_mutex_lock(&mutex);
        myqueue_push(&queue, (i % 2 == 1) ? i : -i);
        pthread_mutex_unlock(&mutex);
    }

    // Push INT_MAX c times as termination signal
    for (int i = 0; i < c; ++i) {
        pthread_mutex_lock(&mutex);
        myqueue_push(&queue, INT_MAX);
        pthread_mutex_unlock(&mutex);
    }

    // Join threads and sum up results
    int total_sum = 0;
    for (int i = 0; i < c; ++i) {
        pthread_join(threads[i], NULL);
        total_sum += args[i].result;
    }

    printf("Final sum: %d\n", total_sum);

    // Cleanup
    pthread_mutex_destroy(&mutex);
    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
