// ringbuffer.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define SHM_NAME "/myringbuffer"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL  "/sem_full"
#define SEM_MUTEX "/sem_mutex"
#define BUFFER_SIZE 10
#define MSG_SIZE 256
#define MSG_COUNT 100

typedef struct {
    char messages[BUFFER_SIZE][MSG_SIZE];
    int write_pos;
    int read_pos;
} ringbuffer_t;

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    (void)sig;
    stop = 1;
}

int main() {
    signal(SIGINT, handle_sigint);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd < 0) { perror("shm_open"); exit(1); }
    if (ftruncate(shm_fd, sizeof(ringbuffer_t)) == -1) { perror("ftruncate"); exit(1); }

    ringbuffer_t *rb = mmap(NULL, sizeof(ringbuffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (rb == MAP_FAILED) { perror("mmap"); exit(1); }

    // Initialisieren, falls neu
    rb->write_pos = 0;
    rb->read_pos = 0;

    sem_t *sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0600, BUFFER_SIZE);
    sem_t *sem_full = sem_open(SEM_FULL, O_CREAT, 0600, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0600, 1);
    if (!sem_empty || !sem_full || !sem_mutex) { perror("sem_open"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork"); exit(1);
    } else if (pid == 0) {
        // Consumer
        for (int i = 0; i < MSG_COUNT && !stop; i++) {
            sem_wait(sem_full);
            sem_wait(sem_mutex);

            char msg[MSG_SIZE];
            strncpy(msg, rb->messages[rb->read_pos], MSG_SIZE);
            rb->read_pos = (rb->read_pos + 1) % BUFFER_SIZE;

            sem_post(sem_mutex);
            sem_post(sem_empty);

            printf("Consumed: %s\n", msg);
        }
        return 0;
    } else {
        // Producer
        for (int i = 0; i < MSG_COUNT && !stop; i++) {
            char msg[MSG_SIZE];
            snprintf(msg, MSG_SIZE, "Message %d", i+1);

            sem_wait(sem_empty);
            sem_wait(sem_mutex);

            strncpy(rb->messages[rb->write_pos], msg, MSG_SIZE);
            rb->write_pos = (rb->write_pos + 1) % BUFFER_SIZE;

            sem_post(sem_mutex);
            sem_post(sem_full);
            usleep(50000); // 50ms delay (optional)
        }

        // Warten auf Kindprozess
        wait(NULL);

        // Aufräumen
        sem_close(sem_empty);
        sem_close(sem_full);
        sem_close(sem_mutex);
        sem_unlink(SEM_EMPTY);
        sem_unlink(SEM_FULL);
        sem_unlink(SEM_MUTEX);
        munmap(rb, sizeof(ringbuffer_t));
        shm_unlink(SHM_NAME);
    }
    return 0;
}
