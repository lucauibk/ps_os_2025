#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>


int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <message_queue_name>\n", argv[0]);
        return 1;
    }
    mqd_t mq;
    mq = mq_open(argv[1], O_CREAT | O_RDONLY, 0666, NULL);
    if (mq == (mqd_t)-1){
        perror("mq_open");
        return 1;
    }
    char buffer[1024];
    while(1){
        ssize_t bytes_read = mq_receive(mq, buffer, sizeof(buffer), NULL);
        if (bytes_read >= 0){
            buffer[bytes_read] = '\0'; // Null-terminate the string
            printf("Received: %s\n", buffer);
        }
    }
    
}