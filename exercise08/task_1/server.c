#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct {
    int* sockfd;
    char* admins;
    struct sockaddr_in* addr;
    pthread_t listener_thread;
    int *shutdown;
}server_t;

typedef struct {
    int clientfd;
    char* username;
}client_t;

int is_admin(const char* username) {
    server_t server;
    char* admins = server.admins;
    char* token = strtok(admins, ",");
    while(token != NULL){
        if(strcmp(username, token) == 0){
            return 1;
        }
        token = strtok(NULL, ",");
    }
    return 0;
}

void* client_handler(void* arg){
    client_t* client = (client_t*)arg;
    int clientfd = client->clientfd;
    server_t server;

    char* username = recv(clientfd, client->username, sizeof(client->username), 0);
    if(is_admin(username) == 1){
        printf("%s connected (admin)\n", username);
    } else {
        printf("%s connected\n", username);
    }
    char buffer[1024];

    while(1){
        int bytes_read = recv(clientfd, buffer, sizeof(buffer), 0);
        if(buffer == "/quit"){
            printf("%s disconnected.\n", username);
            close(clientfd);
            break;
        }
        if(buffer == "/shutdown" && is_admin(client->username) == 1){
            printf("Server is shutting down.\n");
            server.shutdown = 1;
            close(clientfd);
            exit(0);
        }
        printf("%s: %s\n", username, buffer);
    }
}

void* listener(void* arg){
    server_t* server = (server_t*)arg;
    int sockfd = *(server->sockfd);
    char buffer[1024];
    while(1){
        int* clientfd = malloc(sizeof(int));
        clientfd = accept(sockfd, (struct sockaddr*)server->addr, sizeof(*(server->addr)));
        pthread_create(clientfd, NULL, client_handler, NULL);
        if(server->shutdown == 1){
            printf("Server is shutting down.\n");
            close(sockfd);
            exit(0);
        }
    }
}

int main(int argc, char* argv[]){
    if(argc < 3){
        fprintf(stderr, "Usage: %s <port> <admins,...>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    char *admins = argv[argc - 2];
    for(int i = 2; i < argc; i++){
        admins[i] = argv[i];
    }
    server_t server;
    server.admins = admins;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sockfd = &sockfd;
    const struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = {
          .s_addr = htonl(INADDR_ANY),
        },
        .sin_port = htons(port),
      };
    server.addr = &addr;
    
      if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
      }
      if(listen(sockfd, 5) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
      }
      pthread_t listener_thread;
      server.listener_thread = listener_thread;
      server.shutdown = 0;

      pthread_create(listener_thread, NULL, listener, &server);
}