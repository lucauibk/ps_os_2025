#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

double DR_p(const int t, const int n, const unsigned long long s) {
    unsigned long long h = 0;
    for (unsigned long long i = s; i--;) {
        h += (rand() % n + rand() % n + 2 == t);
    }
    return (double)h / s;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <n> <s>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    unsigned long long s = strtoull(argv[2], NULL, 10);
    clock_t start = clock();
    
    for (int i = 2; i <= 2 * n; i++) {
        //srand(getpid());
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork fehlgeschlagen");
            exit(1);
        }

        if (pid == 0) { // Kindprozess
            srand(getpid()); // Seed für Zufallszahlen
            double probability = DR_p(i, n, s);
            clock_t end = clock();
            double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

            printf("Child %d PID = %d. DR_p(%d, %d, %llu) = %.6f. Elapsed time = %.6f s\n",
                   i - 2, getpid(), i, n, s, probability, elapsed_time);

            exit(0); // Kindprozess beendet sich
        }
    }

    // Elternprozess wartet auf alle Kindprozesse
    while(wait(NULL) > 0);
    printf("Done.\n");

    return 0;
}
