#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define MAX_ADMINS 5
#define MAX_USERNAME_LEN 100
#define MAX_MSG_LEN 1024

typedef struct {
    int socketfd;
    char* admins[MAX_ADMINS];
    int num_admins;
    pthread_t listener_tid;
} listener_args;

typedef struct {
    int clientfd;
    char username[MAX_USERNAME_LEN];
    char* admins[MAX_ADMINS];
    int num_admins;
    pthread_t listener_tid;
} client_args;

int is_admin(const char* username, char* admins[], int num_admins) {
    for (int i = 0; i < num_admins; i++) {
        if (strcmp(username, admins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void* client_handler(void* arg) {
    client_args* args = (client_args*)arg;
    char buffer[MAX_MSG_LEN];
    ssize_t bytes_read;

    int admin_flag = is_admin(args->username, args->admins, args->num_admins);
    if (admin_flag) {
        printf("%s connected (admin).\n", args->username);
    } else {
        printf("%s connected.\n", args->username);
    }

    while ((bytes_read = recv(args->clientfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, "/shutdown\n") == 0 && admin_flag) {
            printf("Shutdown command received from admin %s.\n", args->username);
            pthread_cancel(args->listener_tid);
            break;
        } else {
            printf("%s: %s", args->username, buffer);
        }
    }

    if (admin_flag) {
        printf("%s disconnected (admin).\n", args->username);
    } else {
        printf("%s disconnected.\n", args->username);
    }

    close(args->clientfd);
    free(args);
    return NULL;
}

void* listener(void* arg) {
    listener_args* args = (listener_args*)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int clientfd = accept(args->socketfd, (struct sockaddr*)&client_addr, &client_len);
        if (clientfd < 0) {
            perror("accept");
            continue;
        }

        // Lese Benutzernamen vom Client
        char username[MAX_USERNAME_LEN];
        ssize_t bytes = recv(clientfd, username, sizeof(username) - 1, 0);
        if (bytes <= 0) {
            close(clientfd);
            continue;
        }
        username[bytes] = '\0';
        username[strcspn(username, "\n")] = '\0';  // Zeilenumbruch entfernen

        // Client-Argumente vorbereiten
        client_args* cargs = malloc(sizeof(client_args));
        cargs->clientfd = clientfd;
        strncpy(cargs->username, username, MAX_USERNAME_LEN);
        for (int i = 0; i < args->num_admins; i++) {
            cargs->admins[i] = args->admins[i];
        }
        cargs->num_admins = args->num_admins;
        cargs->listener_tid = args->listener_tid;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, cargs);
        pthread_join(tid, NULL);  // Warten auf den Client-Thread
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <admin1> [admin2 ... admin5]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int num_admins = argc - 2;
    if (num_admins > MAX_ADMINS) {
        fprintf(stderr, "Max %d admins allowed.\n", MAX_ADMINS);
        exit(EXIT_FAILURE);
    }

    // Socket erstellen
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    // Admin-Namen speichern
    static char admin_storage[MAX_ADMINS][MAX_USERNAME_LEN];
    char* admin_ptrs[MAX_ADMINS];
    for (int i = 0; i < num_admins; i++) {
        strncpy(admin_storage[i], argv[i + 2], MAX_USERNAME_LEN - 1);
        admin_storage[i][MAX_USERNAME_LEN - 1] = '\0';
        admin_ptrs[i] = admin_storage[i];
    }

    // Listener-Thread starten
    listener_args args = {
        .socketfd = serverfd,
        .num_admins = num_admins
    };
    memcpy(args.admins, admin_ptrs, sizeof(admin_ptrs));

    pthread_t listener_tid;
    args.listener_tid = listener_tid;
    pthread_create(&listener_tid, NULL, listener, &args);
    args.listener_tid = listener_tid; // setze nochmal, da pthread_create den Wert nicht setzt

    pthread_join(listener_tid, NULL);

    close(serverfd);
    printf("Server shut down.\n");
    return 0;
}
