#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>

#define FIFO_DIR "/tmp/"
#define BUFFER_SIZE 256

typedef struct {
    char name[BUFFER_SIZE];
    char fifo_path[BUFFER_SIZE];
    int fd;
} Client;

void cleanup_fifos(Client *clients, int num_clients) {
    for (int i = 0; i < num_clients; i++) {
        close(clients[i].fd);
        unlink(clients[i].fifo_path);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <client1> <client2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_clients = argc - 1;
    Client clients[num_clients];
    struct pollfd fds[num_clients]; 
    int message_counter = 0;

    

    
}
