#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
        return 1;
    }
    int pipefd[2]; 
    if(pipe(pipefd) == -1){
        perror("pipe");
        return 2;
    }

    pid_t pid1 = fork();
    if(pid1 == -1){
        perror("fork");
        return 3;
    }
    
    if(pid1 == 0){
        close(pipefd[0]); // close read end
        dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
        close(pipefd[1]); // close write end
        execlp("cat", "cat", argv[1], argv[2], NULL); //execute cat on file1 and file2
        perror("execlp");
        return EXIT_FAILURE;
    }
    close(pipefd[1]); // close write end
    pid_t pid2 = fork();
    if(pid2 == -1){
        perror("fork");
        return 4;
    }
    if(pid2 == 0){
        close(pipefd[1]); // close write end
        dup2(pipefd[0], STDIN_FILENO); // redirect stdin to pipe
        close(pipefd[0]); // close read end
        execlp("cut", "cut", "-c", "22-", NULL); // execute cut on the output of cat
        perror("execlp cut");
        return EXIT_FAILURE;
    }
    close(pipefd[0]);
    wait(NULL);
    wait(NULL);
    return EXIT_SUCCESS;

}