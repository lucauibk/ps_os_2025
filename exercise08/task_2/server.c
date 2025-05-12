#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>


typedef struct{
    int socketfd;
    char** admins;
    int num_admins;
}server_t;

#define MAX_CLIENTS 100
#define MAX_ADMINS 5
#define MAX_USERNAME_LEN 100
#define MAX_MSG_LEN 1024

server_t server;

int is_admin(const char* username) {
    for (int i = 0; i < server.num_admins; i++) {
        if (strcmp(username, server.admins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void* handle_client(void* arg) {
    int clientfd = *(int*)arg;
    free(arg);

    char username[MAX_USERNAME_LEN];
    int bytes = recv(clientfd, username, sizeof(username) - 1, 0);
    if (bytes <= 0) {
        close(clientfd);
        return NULL;
    }
    username[bytes] = '\0';
    if(is_admin(username)) {
        printf("%s connected (admin)\n", username);
    } else {
        printf("%s connected\n", username);
    }

    while (1) {
        char buffer[MAX_MSG_LEN];
        int received = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) break;
        buffer[received] = '\0';

        if (strcmp(buffer, "/shutdown") == 0 && is_admin(username)) {
            printf("Server is shutting down. Waiting for 1 client(s) to disconnect.");
                sleep(1);
                printf("Server is shutting down.\n");
                close(clientfd);
                exit(0); // Oder Signal an Main-Thread
        }

        if (strcmp(buffer, "/quit") == 0) {
            break;
        }
        printf("%s: %s\n", username, buffer);
    }

    close(clientfd);
    printf("User %s disconnected.\n", username);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <admins,...>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    server.num_admins = argc - 2;
    server.admins = &argv[2];

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(serverfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(serverfd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverfd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(serverfd);
        exit(EXIT_FAILURE);
    }

    server.socketfd = serverfd;

    printf("Server listening on port %d...\n", port);

    while (1) {
        int* clientfd = malloc(sizeof(int));
        *clientfd = accept(serverfd, NULL, NULL);
        if (*clientfd < 0) {
            perror("accept");
            free(clientfd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, clientfd);
        pthread_detach(tid);
    }

    close(serverfd);
    return 0;
}
