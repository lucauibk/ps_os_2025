#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#define NUM_THREADS 500
#define NUM_ITERATIONS 50000

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void* incrementCounter() {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_mutex_lock(&counter_mutex);
        if (i % 2 == 0) {
            counter += 73;
        } else {
            counter -= 71;
        }
        pthread_mutex_unlock(&counter_mutex);
    }
    pthread_exit(NULL);
}

int main(void){
    if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
        fprintf(stderr, "Error initializing mutex\n");
        return 1;
    }
    pthread_t threads[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; i++){
        if(pthread_create(&threads[i], NULL, incrementCounter, NULL) != 0){
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    // Threads joinen
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            return 1;
        }
    }
    pthread_mutex_destroy(&counter_mutex);

    printf("Final counter value: %d\n", counter);
    return 0;
}