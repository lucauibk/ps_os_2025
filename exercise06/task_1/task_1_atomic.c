#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#define NUM_THREADS 500
#define NUM_ITERATIONS 50000

atomic_int counter = 0;

void* incrementCounter() {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        if (i % 2 == 0) {
            atomic_fetch_add(&counter, 73);
        } else {
            atomic_fetch_sub(&counter, 71);
        }
    }
    pthread_exit(NULL);
}

int main(void){
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

    printf("Final counter value: %d\n", counter);
    return 0;
}