#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Kindprozess
        char *args[] = {"/bin/ls", "-l", NULL};
        execvp(args[0], args);
        perror("exec failed"); // Nur wenn exec fehlschlägt
        exit(1);
    } else if (pid > 0) {
        // Elternprozess
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child exited with code %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork failed");
        exit(1);
    }

    return 0;
}
