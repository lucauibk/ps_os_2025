#define _DEFAULT_SOURCE // nötig für PTHREAD_BARRIER_SERIAL_THREAD
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "my_pthread_barrier.h"

typedef struct {
    int id;
    int* rolls;
    int* active;
    int* round_done;
    int* remaining;
    my_pthread_barrier_t* barrier;
} thread_data_t;

void* player_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int id = data->id;

    while (1) {
        if (!data->active[id]) {
            my_pthread_barrier_wait(data->barrier);
            continue;
        }

        int d1 = (rand() % 6) + 1;
        int d2 = (rand() % 6) + 1;
        int sum = d1 + d2;
        data->rolls[id] = sum;

        printf("Player %d rolled a %d and a %d, summing up to %d\n", id, d1, d2, sum);
        fflush(stdout);

        int status = my_pthread_barrier_wait(data->barrier);

        if (status == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Analyse durch einen "Leader"
            int max = -1;
            int same = 1;
            int first = -1;
            for (int i = 0; i < *data->remaining; i++) {
                if (!data->active[i]) continue;
                int r = data->rolls[i];
                if (first == -1) first = r;
                else if (r != first) same = 0;
                if (r > max) max = r;
            }

            if (same) {
                printf("Repeating round\n");
            } else {
                for (int i = 0; i < *data->remaining; i++) {
                    if (data->active[i] && data->rolls[i] == max) {
                        data->active[i] = 0;
                        printf("Eliminating player %d\n", i);
                        (*data->round_done)++;
                    }
                }
            }
            printf("---------------------\n");
        }

        my_pthread_barrier_wait(data->barrier);

        // Abbruchbedingung
        int alive = 0, winner = -1;
        for (int i = 0; i < *data->remaining; i++) {
            if (data->active[i]) {
                alive++;
                winner = i;
            }
        }

        if (alive <= 1) {
            if (data->active[id])
                printf("Player %d has won the game\n", id);
            break;
        }

        my_pthread_barrier_wait(data->barrier);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_players>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n < 2) {
        fprintf(stderr, "At least 2 players required.\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    pthread_t threads[n];
    thread_data_t data[n];

    int rolls[n];
    int active[n];
    for (int i = 0; i < n; i++) active[i] = 1;
    int round_done = 0;
    int remaining = n;

    my_pthread_barrier_t barrier;
    my_pthread_barrier_init(&barrier, NULL, n);

    for (int i = 0; i < n; i++) {
        data[i].id = i;
        data[i].rolls = rolls;
        data[i].active = active;
        data[i].round_done = &round_done;
        data[i].remaining = &n;
        data[i].barrier = &barrier;
        pthread_create(&threads[i], NULL, player_thread, &data[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    my_pthread_barrier_destroy(&barrier);
    return 0;
}
