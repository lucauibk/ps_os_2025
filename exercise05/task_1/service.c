#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define MAX_MSG_SIZE 256 //50 zahlen mit je 3 zeichen ungefähr 200
#define MAX_MESSAGES 10

int main(int argc, char* argv[]){
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char* queue_name = argv[1];
    unsigned int prio = atoi(argv[2]);

    mqd_t mq = mq_open(queue_name, O_WRONLY); //opens only if it exists and only for writing
    if(mq == (mqd_t)-1){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_MSG_SIZE];

    if (!fgets(buffer, MAX_MSG_SIZE, stdin)) {
        fprintf(stderr, "Failed to read from stdin.\n");
        mq_close(mq);
        exit(EXIT_FAILURE);
    }
    if(mq_send(mq, buffer, strlen(buffer), prio) == -1){
        perror("mq_send");
        mq_close(mq);
        exit(EXIT_FAILURE);
    }
    mq_close(mq);
    return 0;

}