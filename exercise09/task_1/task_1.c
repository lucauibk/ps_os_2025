#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "myqueue.h"

#if USE_MY_MUTEX
    #include "my_mutex.h"
    my_mutex_t lock;
    #define MUTEX_INIT() my_mutex_init(&lock)
    #define MUTEX_LOCK() my_mutex_lock(&lock)
    #define MUTEX_UNLOCK() my_mutex_unlock(&lock)
#else
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    #define MUTEX_INIT() (void)0
    #define MUTEX_LOCK() pthread_mutex_lock(&lock)
    #define MUTEX_UNLOCK() pthread_mutex_unlock(&lock)
#endif

myqueue queue;

void* producer(void* arg){
    for (int i = 0; i < 10000000; i++) {
        MUTEX_LOCK();
        myqueue_push(&queue, 1);
        MUTEX_UNLOCK();
    }
    MUTEX_LOCK();
    myqueue_push(&queue, 0); // Terminator
    MUTEX_UNLOCK();
    return NULL;
}
void* consumer(void* arg){
    int64_t sum = 0;
    while(1){
        MUTEX_LOCK();
        if(!myqueue_is_empty(&queue)){
            int val = myqueue_pop(&queue);
            MUTEX_UNLOCK();
            if(val == 0) break; // Terminator
            sum += val;
        }else{
            MUTEX_UNLOCK();
        }
        printf("%lld\n", sum);
        return NULL;
    }
}

int main(void){
    myqueue_init(&queue);
    MUTEX_INIT();
    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    return 0;
}
