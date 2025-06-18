#include <stdio.h>      // printf, fprintf, snprintf, FILE, stdin, stdout, stderr
#include <stdlib.h>     // malloc, free, exit, atoi, rand, qsort
#include <string.h>     // strcpy, strlen, strcmp, memcpy, memset, strcat
#include <stdbool.h>    // bool, true, false
#include <stdint.h>     // int32_t, uint64_t, intptr_t etc.
#include <ctype.h>      // isdigit, isalpha, tolower, toupper
#include <errno.h>      // errno, perror, error codes
#include <time.h>       // time, clock, struct tm, nanosleep
#include <math.h>       // sin, cos, sqrt, pow, fabs
#include <pthread.h>    // pthread_create, mutex, condition variables
#include <unistd.h>     // sleep, usleep, fork, pipe, getpid
#include <fcntl.h>      // open, O_CREAT, O_RDWR etc.
#include <sys/types.h>  // pid_t, ssize_t, off_t
#include <sys/stat.h>   // stat, fstat, chmod
#include <signal.h>     // signal handling, sigaction
#include <assert.h>     // assert()
#include <stdatomic.h>

#include "myqueue.h"

//pthread barrier funktioniert nicht auf meinem Laptop und zid-gpl ist abgestürzt


typedef struct{
    pthread_mutex_t queue_lock;
    myqueue* queue;
    atomic_int customers_served;
}post_office_t;

typedef struct{
    int id;
    int served;
    post_office_t* post_office;
}customer_t;

typedef struct{
    int id;
    post_office_t* post_office;
    customer_t* customers;
    pthread_barrier_t* barrier; 
}worker_t;

void ms_sleep(int min_ms, int max_ms) {
    int duration = min_ms + rand() % (max_ms - min_ms + 1);
    usleep(duration * 1000);
}

void* customer_thread(void* args){
    customer_t* cargs = (customer_t*)args;
    int id = cargs->id;
    printf("Customer %d waiting to be served\n", id);
    pthread_mutex_lock(&cargs->post_office->queue_lock);
    myqueue_push(cargs->post_office->queue,id);
    pthread_mutex_unlock(&cargs->post_office->queue_lock);
    while(!cargs->served){
        ms_sleep(1000, 10000);
    }
    printf("Customer %d left the post office with its parcel\n", id);
    return NULL;

}

void* worker_thread(void* args){
    worker_t* wargs = (worker_t*)args;
    int id = wargs->id;
    while(1){
        pthread_mutex_lock(&wargs->post_office->queue_lock);
        if(!myqueue_is_empty(wargs->post_office->queue)){
            int customer_id = myqueue_pop(wargs->post_office->queue);
            if(customer_id == -1){
                pthread_mutex_unlock(&wargs->post_office->queue_lock);
                printf("Worker %d finshed its work and is waiting for other workers\n", id);
                int rc = pthread_barrier_wait(wargs->barrier);
                if(rc == PTHREAD_BARRIER_SERIAL_THREAD){
                    printf("Total customers served: %d", wargs->post_office->customers_served);
                }
                printf("Worker %d goes home\n", id);
                return NULL;
            }
            wargs->customers[customer_id].served = 1;
            atomic_fetch_add(&wargs->post_office->customers_served, 1);
            printf("Worker %d looking for parcel of customer %d\n", id, customer_id);
            ms_sleep(110, 545);
        }
        pthread_mutex_unlock(&wargs->post_office->queue_lock);
    }
    

}

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Usage: %s <num_customers> <num_workers>\n", argv[0]);
        return 1;
    }
    int num_customers = atoi(argv[1]);
    int num_workers = atoi(argv[2]);
    if(num_customers < 1 || num_workers < 1){

        printf("Arguments have to be at least 1\n");
        return 1;
    }
    customer_t* customer_args = malloc(num_customers * sizeof(customer_t));
    customer_args->served = 0;

    worker_t*  worker_args = malloc(num_workers * sizeof(worker_t));

    post_office_t* post_office = malloc(sizeof(post_office_t));
    worker_args->customers = customer_args;
    atomic_init(&post_office->customers_served, NULL);
    myqueue_init(post_office->queue);
    pthread_mutex_init(&post_office->queue_lock, NULL);

    worker_args->post_office = post_office;
    pthread_barrier_init(worker_args->barrier);

    pthread_t* workers = malloc(num_workers * sizeof(pthread_t));
    pthread_t* customers = malloc(num_customers * sizeof(pthread_t));
    
    for(int i = 0; i < num_workers; i++){
        worker_args[i].id = i;
        pthread_create(&workers[i], NULL, worker_thread, &worker_args[i]);
    }
    for(int i = 0; i < num_customers; i++){
        customer_args[i].id = i;
        pthread_create(&customers[i], NULL, customer_thread, &customer_args[i]);
        ms_sleep(110,110);
    }
    myqueue_push(post_office->queue, -1); //push poison value
    for(int i = 0; i < num_workers; i++){
        pthread_join(workers[i], NULL);
    }
    for(int i = 0; i < num_customers; i++){
        pthread_join(customers[i], NULL);
    }
    pthread_mutex_destroy(&post_office->queue_lock);
    free(customer_args);
    free(worker_args);
    free(post_office);
}
