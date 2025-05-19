#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include "common.h"

typedef struct {
    int socket;
    char username[MSG_SIZE];
    int is_admin;
} Client;

Client* clients[MAX_CLIENTS];
int client_count = 0;
char admin_list[MAX_ADMINS][MSG_SIZE];
int admin_count = 0;
int shutting_down = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t listener_thread;

void broadcast(const char *message, int sender_socket) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket != sender_socket) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void remove_client(int socket) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->socket == socket) {
            close(clients[i]->socket);
            free(clients[i]);
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

int is_admin(const char* username) {
    for (int i = 0; i < admin_count; i++) {
        if (strcmp(username, admin_list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void* client_handler(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char username[MSG_SIZE];
    ssize_t len = recv(client_socket, username, MSG_SIZE, 0);
    if (len <= 0) {
        close(client_socket);
        return NULL;
    }
    username[strcspn(username, "\n")] = '\0';

    Client* client = malloc(sizeof(Client));
    client->socket = client_socket;
    strncpy(client->username, username, MSG_SIZE);
    client->is_admin = is_admin(username);

    pthread_mutex_lock(&client_mutex);
    clients[client_count++] = client;
    pthread_mutex_unlock(&client_mutex);

    printf("%s connected%s.\n", username, client->is_admin ? " (admin)" : "");

    char buffer[MSG_SIZE];
    while ((len = recv(client_socket, buffer, MSG_SIZE, 0)) > 0) {
        buffer[strcspn(buffer, "\n")] = '\0';

        char full_msg[MSG_SIZE + MSG_SIZE];
        snprintf(full_msg, sizeof(full_msg), "%s: %s\n", client->username, buffer);
        printf("%s", full_msg);
        broadcast(full_msg, client_socket);

        if (strcmp(buffer, "/shutdown") == 0) {
            if (client->is_admin && !shutting_down) {
                shutting_down = 1;
                printf("Server is shutting down.\n");

                pthread_cancel(listener_thread);

                pthread_mutex_lock(&client_mutex);
                int remaining = client_count - 1;
                if (remaining > 0)
                    printf("Waiting for %d client(s) to disconnect.\n", remaining);
                pthread_mutex_unlock(&client_mutex);
            } else {
                const char* msg = "Only admins can shut down the server.\n";
                send(client_socket, msg, strlen(msg), 0);
            }
        }
    }

    printf("%s disconnected%s.\n", client->username, client->is_admin ? " (admin)" : "");
    remove_client(client_socket);
    return NULL;
}

void* listen_for_clients(void* arg) {
    int server_socket = *(int*)arg;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addrlen);
        if (*client_socket < 0) {
            free(client_socket);
            if (errno == EINTR || shutting_down) break;
            perror("accept");
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_socket);
        pthread_detach(tid);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <admin1> [admin2 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    for (int i = 2; i < argc && i < 2 + MAX_ADMINS; i++) {
        strncpy(admin_list[admin_count++], argv[i], MSG_SIZE);
    }

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d.\n", port);
    pthread_create(&listener_thread, NULL, listen_for_clients, &server_socket);

    pthread_join(listener_thread, NULL);

    // Wait for all clients to disconnect
    while (1) {
        pthread_mutex_lock(&client_mutex);
        if (client_count == 0) {
            pthread_mutex_unlock(&client_mutex);
            break;
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(1);
    }

    close(server_socket);
    return 0;
}
