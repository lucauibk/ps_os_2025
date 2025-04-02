#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    
    if (pid1 == 0) { // `cat` process
        close(pipe1[0]);
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        execlp("cat", "cat", argv[1], argv[2], NULL);
        perror("execlp cat");
        return EXIT_FAILURE;
    }
    
    close(pipe1[1]);
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    
    if (pid2 == 0) { // `grep` process
        close(pipe2[0]);
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe1[0]);
        close(pipe2[1]);
        execlp("grep", "grep", "some_pattern", NULL);
        perror("execlp grep");
        return EXIT_FAILURE;
    }
    
    close(pipe1[0]);
    close(pipe2[1]);
    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    
    if (pid3 == 0) { // `cut` process
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);
        execlp("cut", "cut", "-c", "22-", NULL);
        perror("execlp cut");
        return EXIT_FAILURE;
    }
    
    close(pipe2[0]);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    
    return EXIT_SUCCESS;
}
