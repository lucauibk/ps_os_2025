#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include "myqueue.h"

typedef struct {
    int partial_sum;
    int id;
} thread_data_t;


void* consumer(void* arg){
    thread_data_t *data = (thread_data_t*) arg;

    while(1){
        int value = myqueue_pop();
        if(value == INT_MAX){
            printf("Consumer %d partial sum: %d\n", data->id, data->partial_sum);
            break;
        }
        data->partial_sum += value;
    }
}

int main(int argc, char* argv[]){
    int c = atoi(argv[1]);
    int n = atoi(argv[2]);

    pthread_t consumer[c];

    for(int i = 0; i < c; i++){
        if(pthread_create(&consumer[i], NULL, consumer, NULL) != 0){
            fprintf("thread not created correctly: %s\n", err);
            return EXIT_FAILURE;
        }
    }

}