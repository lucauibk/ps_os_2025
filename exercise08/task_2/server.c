#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

// Entfernt '\n' vom Ende eines Strings
void strip_newline(char* str) {
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

// Prüft, ob Benutzername ein Admin ist
int is_admin(const char* username, server_state_t* state) {
    for (int i = 0; i < state->num_admins; i++) {
        if (strcmp(username, state->admins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Server herunterfahren
void shutdown_server(server_state_t* state) {
    close(state->listener_fd);
    pthread_mutex_destroy(&state->lock);
}

<<<<<<< HEAD
void* listener(void* arg){
    server_state_t* server_state = (server_state_t*)arg;
    while(1){
        int clientfd = accept(server_state->listener_fd, NULL, NULL);
        client_info_t* client_info = malloc(sizeof(client_info_t));
        if (client_info == NULL) {
            perror("malloc");
            continue;
        }
        client_info->socket_fd = clientfd;
        client_info->is_admin = 0;
        client_info->server_state = server_state;
        pthread_detach(client_info->thread_id);
        pthread_mutex_lock(&server_state->lock);
        server_state->active_clients += 1;
        pthread_mutex_unlock(&server_state->lock);
        if(server_state->shutdown_requested && server_state->active_clients == 0){
            close(clientfd);
            shutdown_server(server_state);
            printf("All clients disconnected, shutting down server...\n");
=======
// Verarbeitung pro Client
void* client_handler(void* arg) {
    client_info_t* client = (client_info_t*)arg;
    char buffer[MSG_SIZE];

    // Benutzername empfangen
    int bytes_received = recv(client->socket_fd, buffer, MSG_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client->socket_fd);
        free(client);
        return NULL;
    }
    buffer[bytes_received] = '\0';
    strip_newline(buffer);

    strncpy(client->username, buffer, MSG_SIZE - 1);
    client->username[MSG_SIZE - 1] = '\0';

    client->is_admin = is_admin(client->username, client->server_state);

    printf("%s connected%s.\n", client->username, client->is_admin ? " (admin)" : "");

    while (1) {
        bytes_received = recv(client->socket_fd, buffer, MSG_SIZE - 1, 0);
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';
        strip_newline(buffer);

        // Admin: /shutdown
        if (client->is_admin && strcmp(buffer, "/shutdown") == 0) {
            pthread_mutex_lock(&client->server_state->lock);
            if (!client->server_state->shutdown_requested) {
                client->server_state->shutdown_requested = 1;
                pthread_cancel(client->server_state->listener_thread_id);
                printf("%s: /shutdown\n", client->username);
                printf("%s disconnected (admin).\n", client->username);
                printf("Server is shutting down. Waiting for %d client(s) to disconnect.\n",
                       client->server_state->active_clients - 1); // sich selbst nicht zählen
            }
            pthread_mutex_unlock(&client->server_state->lock);
>>>>>>> 80787f91f28a8e804393a02b79c3a4bcbfb28712
            break;
        }

        // Jeder: /quit
        if (strcmp(buffer, "/quit") == 0) {
            printf("%s disconnected%s.\n", client->username, client->is_admin ? " (admin)" : "");
            break;
        }

        // Nachricht anzeigen
        printf("%s: %s\n", client->username, buffer);
    }

    close(client->socket_fd);

    pthread_mutex_lock(&client->server_state->lock);
    client->server_state->active_clients--;
    int remaining = client->server_state->active_clients;
    pthread_mutex_unlock(&client->server_state->lock);

    free(client);

    if (client->server_state->shutdown_requested && remaining == 0) {
        printf("All clients disconnected, shutting down server...\n");
        shutdown_server(client->server_state);
    }

    return NULL;
}

// Listener-Thread: wartet auf neue Verbindungen
void* listener(void* arg) {
    server_state_t* state = (server_state_t*)arg;

    // pthread_cancel soll funktionieren
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) {
        int client_fd = accept(state->listener_fd, NULL, NULL);
        if (client_fd < 0) {
            if (state->shutdown_requested) break;
            perror("accept");
            continue;
        }

        client_info_t* client = malloc(sizeof(client_info_t));
        if (!client) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        client->socket_fd = client_fd;
        client->server_state = state;

        pthread_create(&client->thread_id, NULL, client_handler, client);
        pthread_detach(client->thread_id);

        pthread_mutex_lock(&state->lock);
        state->active_clients++;
        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 7) {
        fprintf(stderr, "Usage: %s <port> [admin1] [admin2] [admin3] [admin4] [admin5]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    server_state_t state = {0};
    pthread_mutex_init(&state.lock, NULL);

    // Admins übernehmen
    for (int i = 0; i < 5 && i + 2 < argc; i++) {
        strncpy(state.admins[i], argv[i + 2], MSG_SIZE - 1);
        state.admins[i][MSG_SIZE - 1] = '\0';
        state.num_admins++;
    }

    // Socket anlegen
    state.listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (state.listener_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };

    // Binden
    if (bind(state.listener_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(state.listener_fd);
        return EXIT_FAILURE;
    }

    // Lauschen
    if (listen(state.listener_fd, 5) < 0) {
        perror("listen");
        close(state.listener_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d\n", port);

    // Listener-Thread starten
    if (pthread_create(&state.listener_thread_id, NULL, listener, &state) != 0) {
        perror("pthread_create");
        close(state.listener_fd);
        return EXIT_FAILURE;
    }

    pthread_join(state.listener_thread_id, NULL);
    return 0;
}
