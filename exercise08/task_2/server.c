#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#define MSG_SIZE 128

typedef struct client_info {
    int socket_fd;                // Socket-Dateideskriptor für diesen Client
    pthread_t thread_id;         // Thread, der diesen Client verarbeitet
    char username[MSG_SIZE];     // Benutzername
    int is_admin;                // 1 = Admin, 0 = normaler Nutzer
    server_state_t* server_state; // Pointer auf den Server-Zustand
} client_info_t;

typedef struct {
    int listener_fd;                       // Der Socket für eingehende Verbindungen
    int shutdown_requested;                // Flag, ob /shutdown empfangen wurde
    pthread_t listener_thread_id;          // Thread-ID des Listener-Threads
    pthread_mutex_t lock;                  // Mutex für gemeinsamen Zugriff
    int active_clients;                    // Anzahl der aktuell verbundenen Clients
    char admins[5][MSG_SIZE];              // Liste der maximal 5 Admin-Benutzernamen
    int num_admins;                        // Tatsächliche Anzahl an Admins
} server_state_t;

int is_admin(const char* username, server_state_t* server_state){
    for (int i = 0; i < 5; i++) {
        if (strcmp(username, server_state->admins[i]) == 0) {
            return 1; // User is admin
        }
    }
    return 0; // User is not admin
}

void shutdown_server(server_state_t* server_state){
    close(server_state->listener_fd);
    pthread_detach(server_state->listener_thread_id);
    pthread_mutex_destroy(&server_state->lock);
}

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
            break;
        }
    }
    return NULL;

}

int main(int argc, char* argv[]){
    if (argc < 2 || argc > 7){
        fprintf(stderr, "Usage: %s <port> [admin1] [admin2] [admin3] [admin4] [admin5]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    
    server_state_t server_state;
    server_state.shutdown_requested = 0;
    server_state.active_clients = 0;
    pthread_mutex_init(&server_state.lock, NULL);
    for (int i = 0; i < 5; i++) {
        if (i + 2 < argc) {
            strncpy(server_state.admins[i], argv[i + 2], MSG_SIZE - 1);
            server_state.admins[i][MSG_SIZE - 1] = '\0'; // Ensure null termination
            server_state.num_admins++;
        }
    }
    server_state.listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    const struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = {
          .s_addr = htonl(INADDR_ANY),
        },
        .sin_port = htons(port),
      };
    if(bind(server_state.listener_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(server_state.listener_fd, 5) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", port);
    if(pthread_create(&server_state.listener_thread_id, NULL, listener, &server_state) != 0){
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    close(server_state.listener_fd);
    pthread_join(server_state.listener_thread_id, NULL);
}