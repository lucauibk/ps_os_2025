// TODO: include necessary header files
#define _DEFAULT_SOURCE
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//priintf is not async so safe write function is needed
void safe_write(const char *msg){
    write(STDOUT_FILENO, msg, strlen(msg));
}

// TODO: define signal handler
void signal_handler(int signal){
    switch(signal){
        case SIGINT:
            safe_write("Caught SIGINT\n");
            break;
        case SIGTERM:
            safe_write("Caught SIGTERM\n");
            break;
        case SIGUSR1:
            safe_write("Caught SIGUSR1\n");
            break;
        case SIGUSR2:
            safe_write("Caught SIGUSR2\n");
            break;
        default:
            safe_write("Caught unknown signal\n");
            break;
    }
}

void register_signal(int signal){
    struct sigaction action;
    memset(&action, 0, sizeof(action)); // zero out the sigaction struct
    action.sa_handler = signal_handler;
    action.sa_flags = SA_RESTART; // restart interrupted system calls
    sigaction(signal, &action, NULL);
}

int main(void) {
    // TODO: for each signal, use sigaction to register signal handler
    register_signal(SIGINT);
    register_signal(SIGTERM);
    register_signal(SIGUSR1);
    register_signal(SIGUSR2);

    const time_t work_seconds = 1;

    struct timespec work_duration = {
        .tv_sec = work_seconds,
    };

    struct timespec remaining = {0};

    while (true) {
        // simulate real workload
        if (nanosleep(&work_duration, &remaining) == -1 && errno == EINTR) {
            work_duration = remaining;
            continue;
        }

        // restore work_duration
        work_duration.tv_sec = work_seconds;

        // TODO: more code (only if needed)
    }

    return EXIT_SUCCESS;
}