#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_DIR "/tmp/"
#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <client_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char fifo_path[BUFFER_SIZE];
    snprintf(fifo_path, BUFFER_SIZE, "%s%s_fifo", FIFO_DIR, argv[1]);

    int fd = open(fifo_path, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    char message[BUFFER_SIZE];
    while (1) {
        printf("Enter message: ");
        if (fgets(message, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        size_t len = strlen(message);
        if (message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }

        if (write(fd, message, strlen(message)) == -1) {
            perror("write");
            break;
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
