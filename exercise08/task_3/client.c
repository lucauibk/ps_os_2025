#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

int server_socket;

void* receive_thread(void* arg) {
    char buffer[MSG_SIZE + MSG_SIZE];
    while (1) {
        ssize_t len = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    const char* username = argv[2];

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
    };

    if (connect(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    send(server_socket, username, strlen(username), 0);

    pthread_t recv_tid;
    pthread_create(&recv_tid, NULL, receive_thread, NULL);

    char msg[MSG_SIZE];
    while (fgets(msg, sizeof(msg), stdin)) {
        if (strcmp(msg, "/quit\n") == 0 || strcmp(msg, "/shutdown\n") == 0) {
            send(server_socket, msg, strlen(msg), 0);
            break;
        }
        send(server_socket, msg, strlen(msg), 0);
    }

    close(server_socket);
    pthread_cancel(recv_tid);
    pthread_join(recv_tid, NULL);
    return 0;
}
