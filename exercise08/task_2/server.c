#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 100
#define MSG_SIZE 128
#define MAX_ADMINS 5

typedef struct {
    int socket_fd;
    char username[MSG_SIZE];
    int is_admin;
} ClientInfo;

typedef struct {
    ClientInfo *clients[MAX_CLIENTS];
    int client_count;
    char admin_names[MAX_ADMINS][MSG_SIZE];
    int admin_count;
    pthread_mutex_t mutex;
    int shutdown_requested;
    pthread_t listener_thread;
    int server_socket_fd;
} ServerState;

void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int is_admin(ServerState *state, const char *name) {
    for (int i = 0; i < state->admin_count; ++i) {
        if (strcmp(state->admin_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

void remove_client(ServerState *state, ClientInfo *client) {
    pthread_mutex_lock(&state->mutex);
    for (int i = 0; i < state->client_count; ++i) {
        if (state->clients[i] == client) {
            for (int j = i; j < state->client_count - 1; ++j) {
                state->clients[j] = state->clients[j + 1];
            }
            state->client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&state->mutex);
}

void *client_thread(void *arg) {
    ServerState *state = ((ServerState **)arg)[0];
    int client_fd = *((int *)((ServerState **)arg)[1]);
    free(arg);  // Freigeben der dynamisch reservierten Argumente

    char buffer[MSG_SIZE];
    ssize_t n;

    // Empfang des Benutzernamens
    n = recv(client_fd, buffer, MSG_SIZE - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[n] = '\0';
    trim_newline(buffer);

    ClientInfo *client = malloc(sizeof(ClientInfo));
    client->socket_fd = client_fd;
    strncpy(client->username, buffer, MSG_SIZE);
    client->is_admin = is_admin(state, buffer);

    pthread_mutex_lock(&state->mutex);
    state->clients[state->client_count++] = client;
    pthread_mutex_unlock(&state->mutex);

    printf("%s connected%s.\n", client->username, client->is_admin ? " (admin)" : "");

    while ((n = recv(client_fd, buffer, MSG_SIZE - 1, 0)) > 0) {
        buffer[n] = '\0';
        trim_newline(buffer);
        printf("%s: %s\n", client->username, buffer);

        if (strcmp(buffer, "/shutdown") == 0 && client->is_admin) {
            pthread_mutex_lock(&state->mutex);
            if (!state->shutdown_requested) {
                state->shutdown_requested = 1;
                printf("Server is shutting down.\n");
                pthread_cancel(state->listener_thread);
                int remaining = state->client_count;
                if (remaining > 1)
                    printf("Waiting for %d client(s) to disconnect.\n", remaining - 1);
            }
            pthread_mutex_unlock(&state->mutex);
        }
    }

    close(client_fd);
    remove_client(state, client);
    printf("%s disconnected%s.\n", client->username, client->is_admin ? " (admin)" : "");
    free(client);
    return NULL;
}

void *listener_thread_func(void *arg) {
    ServerState *state = (ServerState *)arg;

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(state->server_socket_fd, (struct sockaddr *)&client_addr, &len);
        if (*client_fd < 0) {
            free(client_fd);
            continue;
        }

        // Übergabeparameter vorbereiten
        void **args = malloc(2 * sizeof(void *));
        args[0] = state;
        args[1] = client_fd;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, args);
        pthread_detach(tid);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <admin1> [admin2 ... admin5]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    ServerState state = {
        .client_count = 0,
        .admin_count = 0,
        .shutdown_requested = 0,
    };
    pthread_mutex_init(&state.mutex, NULL);

    for (int i = 2; i < argc && state.admin_count < MAX_ADMINS; ++i) {
        strncpy(state.admin_names[state.admin_count++], argv[i], MSG_SIZE);
    }

    state.server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (state.server_socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(state.server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };

    if (bind(state.server_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(state.server_socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(state.server_socket_fd, 10) < 0) {
        perror("listen");
        close(state.server_socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d.\n", port);
    pthread_create(&state.listener_thread, NULL, listener_thread_func, &state);
    pthread_join(state.listener_thread, NULL);

    // Warten auf Verbindungsende aller Clients
    while (1) {
        pthread_mutex_lock(&state.mutex);
        if (state.client_count == 0) {
            pthread_mutex_unlock(&state.mutex);
            break;
        }
        pthread_mutex_unlock(&state.mutex);
        usleep(100000); // 100ms warten
    }

    close(state.server_socket_fd);
    pthread_mutex_destroy(&state.mutex);
    return 0;
}
