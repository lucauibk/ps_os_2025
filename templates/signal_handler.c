#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sig) {
    printf("Signal %d received. Ignoring Ctrl+C.\n", sig);
}

int main() {
    signal(SIGINT, sigint_handler);
    while (1) {
        printf("Running... press Ctrl+C\n");
        sleep(1);
    }
}
