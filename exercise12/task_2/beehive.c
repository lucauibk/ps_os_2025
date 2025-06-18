#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "myqueue.h"

typedef struct {
    int id;
    struct flower_field *field;
} bee_arg_t;

typedef struct flower_field {
    int width, height;
    int **flowers; // 0 = geerntet, 1 = Nektar vorhanden
    pthread_mutex_t **flower_locks;

    int nectar_left;

    myqueue queue;
    pthread_mutex_t queue_lock;

    pthread_mutex_t nectar_lock;
} flower_field_t;

static inline int random_between(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Funktion zum Ernten an einer Position
int harvest_flower(flower_field_t *field, coordinate pos) {
    int harvested = 0;

    pthread_mutex_lock(&field->flower_locks[pos.y][pos.x]);
    if (field->flowers[pos.y][pos.x] == 1) {
        field->flowers[pos.y][pos.x] = 0;
        harvested = 1;
        pthread_mutex_lock(&field->nectar_lock);
        field->nectar_left--;
        pthread_mutex_unlock(&field->nectar_lock);
    }
    pthread_mutex_unlock(&field->flower_locks[pos.y][pos.x]);

    return harvested;
}

// Funktion, die Nachbarn einer Position überprüft und sie der Queue hinzufügt, wenn Nektar da ist
void report_neighbours(flower_field_t *field, int id, coordinate pos) {
    coordinate neighbors[4] = {
        {pos.x - 1, pos.y},
        {pos.x + 1, pos.y},
        {pos.x, pos.y - 1},
        {pos.x, pos.y + 1}
    };

    char report[256];
    int offset = snprintf(report, sizeof(report),
        "Bee %d collected nectar at position (%d,%d) and reports potential food sources:",
        id, pos.x, pos.y);

    pthread_mutex_lock(&field->queue_lock);
    for (int i = 0; i < 4; i++) {
        int nx = neighbors[i].x;
        int ny = neighbors[i].y;

        if (nx >= 0 && nx < field->width && ny >= 0 && ny < field->height) {
            pthread_mutex_lock(&field->flower_locks[ny][nx]);
            int nectar = field->flowers[ny][nx];
            pthread_mutex_unlock(&field->flower_locks[ny][nx]);

            if (nectar == 1) {
                myqueue_push(&field->queue, neighbors[i]);
                offset += snprintf(report + offset, sizeof(report) - offset, " (%d,%d)", nx, ny);
            }
        }
    }
    pthread_mutex_unlock(&field->queue_lock);

    printf("%s\n", report);
}

void *bee_thread(void *arg) {
    bee_arg_t *barg = (bee_arg_t *)arg;
    flower_field_t *field = barg->field;
    int id = barg->id;

    while (1) {
        coordinate pos;

        pthread_mutex_lock(&field->queue_lock);
        while (myqueue_is_empty(&field->queue)) {
            // Queue leer -> arbeite im Beehive
            pthread_mutex_unlock(&field->queue_lock);

            printf("Bee %d is working in beehive.\n", id);
            usleep(random_between(100, 500) * 1000);

            pthread_mutex_lock(&field->queue_lock);
            if (field->nectar_left == 0) {
                pthread_mutex_unlock(&field->queue_lock);
                return NULL; // alles geerntet, fertig
            }
        }

        // Pop aus der Queue (Queue ist garantiert nicht leer)
        pos = myqueue_pop(&field->queue);
        pthread_mutex_unlock(&field->queue_lock);

        // 2. Fliege zur Blüte (random sleep 100-500ms)
        printf("Bee %d is flying to food source at position (%d,%d).\n", id, pos.x, pos.y);
        usleep(random_between(100, 500) * 1000);

        // 3. Versuche zu ernten (mit Lock)
        int harvested = harvest_flower(field, pos);

        if (harvested) {
            // 4. Füge Nachbarn zur Queue hinzu (wenn Nektar vorhanden)
            report_neighbours(field, id, pos);
        } else {
            printf("Bee %d could not find nectar at position (%d,%d).\n", id, pos.x, pos.y);
        }

        // 5. Abbruch, wenn alles geerntet
        pthread_mutex_lock(&field->nectar_lock);
        int done = (field->nectar_left == 0);
        pthread_mutex_unlock(&field->nectar_lock);

        if (done) break;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <width> <height> <bee_count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int bee_count = atoi(argv[3]);

    if (width <= 0 || height <= 0 || bee_count <= 0) {
        fprintf(stderr, "All arguments must be positive integers.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    flower_field_t field;
    field.width = width;
    field.height = height;
    field.nectar_left = width * height;

    // Allocate 2D arrays
    field.flowers = malloc(height * sizeof(int *));
    field.flower_locks = malloc(height * sizeof(pthread_mutex_t *));
    for (int i = 0; i < height; i++) {
        field.flowers[i] = malloc(width * sizeof(int));
        field.flower_locks[i] = malloc(width * sizeof(pthread_mutex_t));
        for (int j = 0; j < width; j++) {
            field.flowers[i][j] = 1;
            pthread_mutex_init(&field.flower_locks[i][j], NULL);
        }
    }

    myqueue_init(&field.queue);
    pthread_mutex_init(&field.queue_lock, NULL);
    pthread_mutex_init(&field.nectar_lock, NULL);

    // Startposition in Queue
    pthread_mutex_lock(&field.queue_lock);
    myqueue_push(&field.queue, (coordinate){0,0});
    pthread_mutex_unlock(&field.queue_lock);

    pthread_t *bees = malloc(bee_count * sizeof(pthread_t));
    bee_arg_t *bee_args = malloc(bee_count * sizeof(bee_arg_t));

    for (int i = 0; i < bee_count; i++) {
        bee_args[i].id = i;
        bee_args[i].field = &field;
        if (pthread_create(&bees[i], NULL, bee_thread, &bee_args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < bee_count; i++) {
        pthread_join(bees[i], NULL);
    }

    printf("All nectar harvested! Simulation finished.\n");

    // Cleanup
    for (int i = 0; i < height; i++) {
        free(field.flowers[i]);
        for (int j = 0; j < width; j++) {
            pthread_mutex_destroy(&field.flower_locks[i][j]);
        }
        free(field.flower_locks[i]);
    }
    free(field.flowers);
    free(field.flower_locks);

    pthread_mutex_destroy(&field.queue_lock);
    pthread_mutex_destroy(&field.nectar_lock);

    free(bees);
    free(bee_args);

    return EXIT_SUCCESS;
}
