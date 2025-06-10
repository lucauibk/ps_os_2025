#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include "myqueue.h"

#define MIN_SLEEP 100
#define MAX_SLEEP 500
#define BEAR_CHANCE 10     // 10%
#define BEAR_SUCCESS 50    // 50%

typedef struct {
    int id;
    struct flower_field *field;
} bee_arg_t;

typedef struct flower_field {
    int width, height;
    int **flowers; // 0 = harvested, 1 = nectar present
    pthread_mutex_t **flower_locks;

    int nectar_left;

    myqueue queue;
    pthread_mutex_t queue_lock;

    pthread_mutex_t nectar_lock;
    pthread_barrier_t bear_barrier;
    int beehive_destroyed;
    pthread_mutex_t beehive_lock;

    int bees_total;
    int *nectar_collected_by;
} flower_field_t;

void ms_sleep(int min_ms, int max_ms) {
    int duration = min_ms + rand() % (max_ms - min_ms + 1);
    usleep(duration * 1000);
}

// Angepasste Funktion: push mit Schutz
void safe_myqueue_push(flower_field_t *field, int x, int y) {
    pthread_mutex_lock(&field->queue_lock);
    myqueue_push(&field->queue, (coordinate){x, y});
    pthread_mutex_unlock(&field->queue_lock);
}

// Angepasste Funktion: pop mit Schutz, gibt 1 bei Erfolg, 0 wenn leer
int safe_myqueue_pop(flower_field_t *field, int *x, int *y) {
    pthread_mutex_lock(&field->queue_lock);
    if (myqueue_is_empty(&field->queue)) {
        pthread_mutex_unlock(&field->queue_lock);
        return 0;
    }
    coordinate c = myqueue_pop(&field->queue);
    pthread_mutex_unlock(&field->queue_lock);
    *x = c.x;
    *y = c.y;
    return 1;
}

void report_neighbors(flower_field_t *field, int x, int y) {
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < field->width && ny >= 0 && ny < field->height) {
            pthread_mutex_lock(&field->flower_locks[ny][nx]);
            if (field->flowers[ny][nx] == 1) {
                safe_myqueue_push(field, nx, ny);
                fprintf(stdout, " (%d,%d)", nx, ny);  // Ausgabe hier!
            }
            pthread_mutex_unlock(&field->flower_locks[ny][nx]);
        }
    }
}


int check_and_harvest(flower_field_t *field, int x, int y, int bee_id) {
    pthread_mutex_lock(&field->flower_locks[y][x]);
    int harvested = 0;
    if (field->flowers[y][x] == 1) {
        field->flowers[y][x] = 0;
        pthread_mutex_lock(&field->nectar_lock);
        field->nectar_left--;
        field->nectar_collected_by[bee_id]++;
        pthread_mutex_unlock(&field->nectar_lock);
        harvested = 1;
    }
    pthread_mutex_unlock(&field->flower_locks[y][x]);
    return harvested;
}

int check_bear_attack(flower_field_t *field) {
    int outcome = rand() % 100;
    if (outcome < BEAR_CHANCE) {
        fprintf(stdout, "Bees encounter a bear and engage in a fight.\n");
        int fight = rand() % 100;
        if (fight < BEAR_SUCCESS) {
            fprintf(stdout, "The bees successfully repel the bear and resume their work.\n");
        } else {
            fprintf(stdout, "Bear destroys the beehive.\n");
            pthread_mutex_lock(&field->beehive_lock);
            field->beehive_destroyed = 1;
            pthread_mutex_unlock(&field->beehive_lock);
        }
        return 1;
    }
    return 0;
}

void *bee_thread(void *arg) {
    bee_arg_t *barg = (bee_arg_t *)arg;
    flower_field_t *field = barg->field;
    int id = barg->id;

    while (1) {
        pthread_mutex_lock(&field->beehive_lock);
        if (field->beehive_destroyed || field->nectar_left <= 0) {
            pthread_mutex_unlock(&field->beehive_lock);
            break;
        }
        pthread_mutex_unlock(&field->beehive_lock);

        int x, y;
        int found = safe_myqueue_pop(field, &x, &y);
        if (found) {
            fprintf(stdout, "Bee %d is flying to food source at position (%d,%d).\n", id, x, y);
            ms_sleep(MIN_SLEEP, MAX_SLEEP);
            if (check_and_harvest(field, x, y, id)) {
                fprintf(stdout, "Bee %d collected nectar at position (%d,%d) and reports potential food sources:", id, x, y);
                report_neighbors(field, x, y);
                fprintf(stdout, "\n");
            } else {
                fprintf(stdout, "Bee %d could not find nectar at position (%d,%d).\n", id, x, y);
            }
        } else {
            fprintf(stdout, "Bee %d is working in beehive.\n", id);
            ms_sleep(MIN_SLEEP, MAX_SLEEP);
        }

        pthread_barrier_wait(&field->bear_barrier);
        check_bear_attack(field);
        pthread_barrier_wait(&field->bear_barrier);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <width> <height> <number of bees>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int bees = atoi(argv[3]);

    if (width <= 0 || height <= 0 || bees <= 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return EXIT_FAILURE;
    }

    flower_field_t field = {
        .width = width,
        .height = height,
        .nectar_left = width * height,
        .beehive_destroyed = 0,
        .bees_total = bees
    };

    myqueue_init(&field.queue);
    pthread_mutex_init(&field.queue_lock, NULL);
    pthread_mutex_init(&field.nectar_lock, NULL);
    pthread_mutex_init(&field.beehive_lock, NULL);
    pthread_barrier_init(&field.bear_barrier, NULL, bees);

    field.flowers = malloc(height * sizeof(int *));
    field.flower_locks = malloc(height * sizeof(pthread_mutex_t *));
    field.nectar_collected_by = calloc(bees, sizeof(int));
    if (!field.flowers || !field.flower_locks || !field.nectar_collected_by) {
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < height; ++i) {
        field.flowers[i] = malloc(width * sizeof(int));
        field.flower_locks[i] = malloc(width * sizeof(pthread_mutex_t));
        if (!field.flowers[i] || !field.flower_locks[i]) {
            fprintf(stderr, "Memory allocation failed\n");
            return EXIT_FAILURE;
        }
        for (int j = 0; j < width; ++j) {
            field.flowers[i][j] = 1;
            pthread_mutex_init(&field.flower_locks[i][j], NULL);
        }
    }

    int seed_x = rand() % width;
    int seed_y = rand() % height;
    safe_myqueue_push(&field, seed_x, seed_y);

    pthread_t threads[bees];
    bee_arg_t args[bees];
    for (int i = 0; i < bees; ++i) {
        args[i].id = i;
        args[i].field = &field;
        if (pthread_create(&threads[i], NULL, bee_thread, &args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < bees; ++i) {
        pthread_join(threads[i], NULL);
    }

    int total_collected = 0;
    for (int i = 0; i < bees; ++i) {
        total_collected += field.nectar_collected_by[i];
    }

    printf("%d bees collected nectar from %d/%d flowers.\n", bees, total_collected, width * height);
    if (field.beehive_destroyed) {
        printf("Beehive was destroyed.\n");
    } else {
        printf("Beehive survived.\n");
    }

    // Cleanup
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            pthread_mutex_destroy(&field.flower_locks[i][j]);
        }
        free(field.flowers[i]);
        free(field.flower_locks[i]);
    }
    free(field.flowers);
    free(field.flower_locks);
    free(field.nectar_collected_by);

    pthread_mutex_destroy(&field.queue_lock);
    pthread_mutex_destroy(&field.nectar_lock);
    pthread_mutex_destroy(&field.beehive_lock);
    pthread_barrier_destroy(&field.bear_barrier);

    // myqueue cleanup if needed (nicht verändert, je nach Implementierung)
    // z.B. myqueue_destroy(&field.queue);

    return EXIT_SUCCESS;
}
