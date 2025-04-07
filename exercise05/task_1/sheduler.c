#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAX_MSG_SIZE 256
#define MAX_MESSAGES 10

volatile sig_atomic_t running = 1;
mqd_t mq;
const char *queue_name;

void handle_sigint(int sig) {
    (void)sig;
    running = 0;
    printf("\nShutting down.\n");
    mq_close(mq);
    mq_unlink(queue_name);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    queue_name = argv[1];

    if (queue_name[0] != '/') {
        fprintf(stderr, "Error: queue name must start with '/'. Example: /csXXXX\n");
        exit(EXIT_FAILURE);
    }

    // Handle SIGINT for graceful shutdown
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // Remove previous queue if it exists
    mq_unlink(queue_name);

    mq = mq_open(queue_name, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    printf("Scheduler started on queue: %s\n", queue_name);

    char buffer[MAX_MSG_SIZE];
    unsigned int prio;

    while (running) {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_MSG_SIZE, &prio);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("Scheduling task with priority %u:\n", prio);

            // Parse integers from buffer
            int numbers[128];
            int n = 0;
            char *token = strtok(buffer, " ");
            while (token != NULL && n < 128) {
                numbers[n++] = atoi(token);
                token = strtok(NULL, " ");
            }

            if (n == 0) {
                printf("  No valid numbers received.\n");
                continue;
            }

            int min = numbers[0], max = numbers[0];
            long sum = numbers[0];

            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    if (numbers[i] < min) min = numbers[i];
                    if (numbers[i] > max) max = numbers[i];
                    sum += numbers[i];
                }

                printf("\rStatistics progress: %d%%", (100 * (i + 1) / n));
                fflush(stdout);
                usleep(5000000); // 500 ms
            }

            double mean = (double)sum / n;
            printf("\n  min: %d\n  max: %d\n  mean: %.2f\n", min, max, mean);
        } else if (errno != EINTR) {
            perror("mq_receive");
        }
    }

    // Cleanup (should normally not reach here, as signal handler exits)
    mq_close(mq);
    mq_unlink(queue_name);
    return 0;
}
