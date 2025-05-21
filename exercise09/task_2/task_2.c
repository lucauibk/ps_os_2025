#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int id;
    int num_players;
    pthread_barrier_t* barrier;
    int* rolls;
    bool* eliminated;
    int* active_players;
    pthread_mutex_t* print_mutex;
} thread_data_t;

void* player_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int id = data->id;
    pthread_barrier_t* barrier = data->barrier;
    int* rolls = data->rolls;
    bool* eliminated = data->eliminated;
    int* active_players = data->active_players;
    pthread_mutex_t* print_mutex = data->print_mutex;

    while (1) {
        int die1 = 0, die2 = 0, sum = 0;
        if (!eliminated[id]) {
            // Würfeln
            die1 = rand() % 6 + 1;
            die2 = rand() % 6 + 1;
            sum = die1 + die2;
            rolls[id] = sum;
    
            pthread_mutex_lock(print_mutex);
            printf("Player %d rolled a %d and a %d, summing up to %d\n", id, die1, die2, sum);
            pthread_mutex_unlock(print_mutex);
        } else {
            rolls[id] = 0; // oder ein Wert, der niemals der Maximalwert sein kann
        }
    
        int rc = pthread_barrier_wait(barrier);
    
        if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Nur dieser Thread entscheidet über Eliminierung
    
            int max = -1;
            int same = 1;
            int prev = -1;
            int count = 0;
    
            for (int i = 0; i < data->num_players; ++i) {
                if (!eliminated[i]) {
                    count++;
                    if (prev == -1) {
                        prev = rolls[i];
                    } else if (prev != rolls[i]) {
                        same = 0;
                    }
                    if (rolls[i] > max) {
                        max = rolls[i];
                    }
                }
            }
    
            if (same) {
                pthread_mutex_lock(print_mutex);
                printf("Repeating round\n");
                pthread_mutex_unlock(print_mutex);
            } else {
                pthread_mutex_lock(print_mutex);
                for (int i = 0; i < data->num_players; ++i) {
                    if (!eliminated[i] && rolls[i] == max) {
                        eliminated[i] = true;
                        (*active_players)--;
                        printf("Eliminating player %d\n", i);
                    }
                }
                pthread_mutex_unlock(print_mutex);
            }
    
            pthread_mutex_lock(print_mutex);
            printf("---------------------\n");
            pthread_mutex_unlock(print_mutex);
        }
    
        // Alle warten hier nochmal auf Synchronisation
        pthread_barrier_wait(barrier);
    
        // Erst *nach* der Barriere aussteigen
        if (*active_players <= 1) {
            break;
        }
    }
    

    // Letzter Spieler verkündet den Sieg
    if (!eliminated[id]) {
        pthread_mutex_lock(print_mutex);
        printf("Player %d has won the game\n", id);
        pthread_mutex_unlock(print_mutex);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_players>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_players = atoi(argv[1]);
    if (num_players < 2) {
        fprintf(stderr, "At least 2 players are required\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    pthread_t threads[num_players];
    thread_data_t thread_data[num_players];
    int rolls[num_players];
    bool eliminated[num_players];
    for (int i = 0; i < num_players; ++i) {
        eliminated[i] = false;
    }

    int active_players = num_players;
    pthread_barrier_t barrier;
    pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_barrier_init(&barrier, NULL, num_players);

    for (int i = 0; i < num_players; ++i) {
        thread_data[i] = (thread_data_t){
            .id = i,
            .num_players = num_players,
            .barrier = &barrier,
            .rolls = rolls,
            .eliminated = eliminated,
            .active_players = &active_players,
            .print_mutex = &print_mutex
        };
        pthread_create(&threads[i], NULL, player_thread, &thread_data[i]);
    }
//cleanup
    for (int i = 0; i < num_players; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&print_mutex);
    return 0;
}
