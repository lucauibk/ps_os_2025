#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MSG_SIZE 128

typedef struct server_state server_state_t;

typedef struct client_info {
    int socket_fd;
    pthread_t thread_id;
    char username[MSG_SIZE];
    int is_admin;
    server_state_t* server_state;
} client_info_t;

struct server_state {
    int listener_fd;
    int shutdown_requested;
    pthread_t listener_thread_id;
    pthread_mutex_t lock;
    int active_clients;
    char admins[5][MSG_SIZE];
    int num_admins;
};

// Hilfsfunktion zum Entfernen von '\n'
void strip_newline(char* str) {
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

int is_admin(const char* username, server_state_t* server_state) {
    for (int i = 0; i < server_state->num_admins; i++) {
        if (strcmp(username, server_state->admins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void shutdown_server(server_state_t* server_state) {
    close(server_state->listener_fd);
    pthread_mutex_destroy(&server_state->lock);
}

void* client_handler(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    char buffer[MSG_SIZE];

    int bytes_received = recv(client_info->socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        close(client_info->socket_fd);
        free(client_info);
        return NULL;
    }

    buffer[bytes_received] = '\0';
    strip_newline(buffer);
    strncpy(client_info->username, buffer, MSG_SIZE - 1);
    client_info->username[MSG_SIZE - 1] = '\0';

    client_info->is_admin = is_admin(client_info->username, client_info->server_state);

    if (client_info->is_admin) {
        printf("%s connected (admin).\n", client_info->username);
    } else {
        printf("%s connected.\n", client_info->username);
    }

    while (1) {
        bytes_received = recv(client_info->socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';
        strip_newline(buffer);

        if (client_info->is_admin && strcmp(buffer, "/shutdown") == 0) {
            pthread_mutex_lock(&client_info->server_state->lock);
            if (!client_info->server_state->shutdown_requested) {
                client_info->server_state->shutdown_requested = 1;
                pthread_cancel(client_info->server_state->listener_thread_id);
                printf("Server is shutting down. Waiting for %d client(s) to disconnect.\n", client_info->server_state->active_clients);
            }
            pthread_mutex_unlock(&client_info->server_state->lock);
            continue;
        }

        if (strcmp(buffer, "/quit") == 0) {
            if (client_info->is_admin) {
                printf("%s disconnected (admin).\n", client_info->username);
            } else {
                printf("%s disconnected.\n", client_info->username);
            }
            break;
        }

        printf("%s: %s\n", client_info->username, buffer);
    }

    close(client_info->socket_fd);
    pthread_mutex_lock(&client_info->server_state->lock);
    client_info->server_state->active_clients--;
    int remaining = client_info->server_state->active_clients;
    pthread_mutex_unlock(&client_info->server_state->lock);

    free(client_info);

    if (client_info->server_state->shutdown_requested && remaining == 0) {
        printf("All clients disconnected, shutting down server...\n");
        shutdown_server(client_info->server_state);
    }

    return NULL;
}

void* listener(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    server_state_t* server_state = (server_state_t*)arg;

    while (1) {
        int clientfd = accept(server_state->listener_fd, NULL, NULL);
        if (clientfd < 0) {
            perror("accept");
            continue;
        }

        client_info_t* client_info = malloc(sizeof(client_info_t));
        if (client_info == NULL) {
            perror("malloc");
            close(clientfd);
            continue;
        }

        client_info->socket_fd = clientfd;
        client_info->server_state = server_state;

        pthread_create(&client_info->thread_id, NULL, client_handler, client_info);
        pthread_detach(client_info->thread_id);

        pthread_mutex_lock(&server_state->lock);
        server_state->active_clients++;
        pthread_mutex_unlock(&server_state->lock);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 7) {
        fprintf(stderr, "Usage: %s <port> [admin1] [admin2] [admin3] [admin4] [admin5]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    server_state_t server_state = {0};
    pthread_mutex_init(&server_state.lock, NULL);

    server_state.num_admins = 0;
    for (int i = 0; i < 5 && (i + 2) < argc; i++) {
        strncpy(server_state.admins[i], argv[i + 2], MSG_SIZE - 1);
        server_state.admins[i][MSG_SIZE - 1] = '\0';
        server_state.num_admins++;
    }

    server_state.listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_state.listener_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };

    if (bind(server_state.listener_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_state.listener_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_state.listener_fd, 5) < 0) {
        perror("listen");
        close(server_state.listener_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    if (pthread_create(&server_state.listener_thread_id, NULL, listener, &server_state) != 0) {
        perror("pthread_create");
        close(server_state.listener_fd);
        exit(EXIT_FAILURE);
    }

    pthread_join(server_state.listener_thread_id, NULL);

    return 0;
}
