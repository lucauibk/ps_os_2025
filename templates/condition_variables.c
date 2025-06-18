#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX 10
int buffer[MAX];
int count = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    for (int i = 0; i < 100; i++) {
        pthread_mutex_lock(&mtx);
        while (count == MAX)
            pthread_cond_wait(&not_full, &mtx);

        buffer[count++] = i;
        printf("Produced: %d\n", i);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < 100; i++) {
        pthread_mutex_lock(&mtx);
        while (count == 0)
            pthread_cond_wait(&not_empty, &mtx);

        int val = buffer[--count];
        printf("Consumed: %d\n", val);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}
