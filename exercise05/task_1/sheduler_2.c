#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char queue_name = argv[1];

    mqd_t mq = mq_create
}