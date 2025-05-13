#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MSG_SIZE 128

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char* username = argv[2];

    // Socket erstellen
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Serveradresse vorbereiten
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK), // oder INADDR_ANY, falls extern
    };

    // Verbindung aufbauen
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Benutzername senden
    send(sockfd, username, strlen(username), 0);

    char buffer[MSG_SIZE];
    while (1) {
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;

        // Newline entfernen
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';

        // Nachricht senden
        send(sockfd, buffer, strlen(buffer), 0);

        // Bei /quit oder /shutdown beenden
        if (strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/shutdown") == 0) {
            break;
        }
    }

    close(sockfd);
    return 0;
}
