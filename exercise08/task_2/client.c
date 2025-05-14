#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MSG_SIZE 128

void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    const char *username = argv[2];
    int sockfd;

    struct sockaddr_in server_addr;

    // Erstellen des Sockets
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // localhost

    // Verbindung zum Server herstellen
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Benutzername an den Server senden
    if (send(sockfd, username, strlen(username), 0) < 0) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[MSG_SIZE];
    ssize_t n;

    // Eingabeaufforderung für Nachrichten
    while (1) {
        printf("> ");
        if (!fgets(buffer, MSG_SIZE, stdin)) {
            break;
        }

        trim_newline(buffer);

        // Wenn /quit eingegeben wird, Verbindung beenden
        if (strcmp(buffer, "/quit") == 0) {
            send(sockfd, "%s disconnected", username, 0);
            break;
        }

        // Wenn /shutdown eingegeben wird, Nachricht an den Server senden
        if (strcmp(buffer, "/shutdown") == 0) {
            if (send(sockfd, "/shutdown", 9, 0) < 0) {
                perror("send /shutdown");
                break;
            }
            printf("Sent /shutdown to the server.\n");
            break;
        }

        // Normale Nachricht senden
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
    }

    close(sockfd);
    return 0;
}
