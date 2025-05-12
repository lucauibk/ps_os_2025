#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char* username = argv[2];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Sende Username an Server
    send(sockfd, username, strlen(username), 0);

    while (1) {
        char buffer[MAX_MSG_LEN];
        printf("Enter message: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Zeilenumbruch entfernen

        if (strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/shutdown") == 0) {
            send(sockfd, buffer, strlen(buffer), 0);
            break;
        }

        send(sockfd, buffer, strlen(buffer), 0);
    }

    close(sockfd);
    printf("Connection closed.\n");
    return 0;
}
