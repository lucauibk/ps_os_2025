#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int *shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *shared = 0;

    if (fork() == 0) {
        for (int i = 0; i < 100000; i++) (*shared)++;
        exit(0);
    } else {
        for (int i = 0; i < 100000; i++) (*shared)++;
        wait(NULL);
        printf("Result: %d\n", *shared);
    }

    munmap(shared, sizeof(int));
    return 0;
}
