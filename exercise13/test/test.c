#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>  // für usleep
#include <string.h>

#include "myqueue.h"

typedef struct {
    int height;
    int width;
    int num_bees;
    atomic_int nectar;
    int** field;
    myqueue* queue;
    pthread_mutex_t queue_lock;
    pthread_mutex_t** locks; // Hier korrigiert: Array von Zeigern auf Mutex
} flower_field_t;

typedef struct {
    int id;
    flower_field_t* flower_field;
} bee_args;

char* coordinates_to_string(coordinate* positions, int count) {
    char* buffer = malloc(100);
    if (!buffer) return NULL;
    buffer[0] = '\0';

    for (int i = 0; i < count; i++) {
        char part[32];
        snprintf(part, sizeof(part), "(%d, %d)", positions[i].x, positions[i].y);
        strcat(buffer, part);
        if (i < count - 1) {
            strcat(buffer, ", ");
        }
    }

    return buffer;
}

void report_neighbours(coordinate position, bee_args* bargs ) {
    int width = bargs->flower_field->width;
    int height = bargs->flower_field->height;
    coordinate* positions = malloc(4 * sizeof(coordinate));
    int count = 0;

    int directions[4][2] = {
        { 0, -1},
        { 0,  1},
        {-1,  0},
        { 1,  0}
    };

    for (int i = 0; i < 4; i++) {
        int nx = position.x + directions[i][0];
        int ny = position.y + directions[i][1];

        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
            pthread_mutex_lock(&bargs->flower_field->locks[ny][nx]);
            if (bargs->flower_field->field[ny][nx] == 0) {
                coordinate new_cord = { .x = nx, .y = ny };
                positions[count] = new_cord;
                count++;
                pthread_mutex_lock(&bargs->flower_field->queue_lock);
                myqueue_push(bargs->flower_field->queue, new_cord);
                pthread_mutex_unlock(&bargs->flower_field->queue_lock);
            }
            pthread_mutex_unlock(&bargs->flower_field->locks[ny][nx]);
        }
    }

    char* pos_str = coordinates_to_string(positions, count);
    printf("Bee %d found potential sources: %s\n", bargs->id, pos_str);
    free(pos_str);
    free(positions);
}

void* bee_thread(void* args){
    bee_args* bargs = (bee_args*)args;
    int id = bargs->id;
    flower_field_t* flower_field = bargs->flower_field;

    while(atomic_load(&flower_field->nectar) > 0){
        pthread_mutex_lock(&flower_field->queue_lock);
        if(myqueue_is_empty(flower_field->queue)){
            pthread_mutex_unlock(&flower_field->queue_lock);
            printf("Bee %d is working in the hive\n", id);
            usleep(50000);
        }else{
            coordinate position = myqueue_pop(flower_field->queue);
            pthread_mutex_unlock(&flower_field->queue_lock);

            pthread_mutex_lock(&flower_field->locks[position.y][position.x]);
            if(flower_field->field[position.y][position.x] == 0){
                flower_field->field[position.y][position.x] = 1;
                atomic_fetch_sub(&flower_field->nectar, 1);
                printf("Bee %d found nectar at position (%d, %d)\n", id, position.x, position.y);
                report_neighbours(position, bargs);
                usleep(50000);
            }
            pthread_mutex_unlock(&flower_field->locks[position.y][position.x]);
        }
        printf("nectar remaining: %d\n", atomic_load(&flower_field->nectar));
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if (argc < 4) {
        fprintf(stderr, "Usage: %s height width num_bees\n", argv[0]);
        return 1;
    }

    int height = atoi(argv[1]);
    int width = atoi(argv[2]);
    int num_bees = atoi(argv[3]);

    flower_field_t flower_field;
    flower_field.height = height;
    flower_field.width = width;
    flower_field.num_bees = num_bees;
    atomic_init(&flower_field.nectar, height * width - 1);
    pthread_mutex_init(&flower_field.queue_lock, NULL);

    flower_field.queue = malloc(sizeof(myqueue));
    myqueue_init(flower_field.queue);

    bee_args* bargs = malloc(num_bees * sizeof(bee_args));
    pthread_t* bees = malloc(num_bees * sizeof(pthread_t));

    flower_field.field = malloc(height * sizeof(int*));
    flower_field.locks = malloc(height * sizeof(pthread_mutex_t*));

    for(int i = 0; i < height; i++){
        flower_field.field[i] = malloc(width * sizeof(int)); // auf 0 initialisieren
        flower_field.locks[i] = malloc(width * sizeof(pthread_mutex_t));
        for(int j = 0; j < width; j++){
            pthread_mutex_init(&flower_field.locks[i][j], NULL);
            flower_field.field[i][j] = 0;
        }
    }
    coordinate start_coordinate = {.x = 0, .y = 0};
    myqueue_push(flower_field.queue, start_coordinate);

    for(int i = 0; i < num_bees; i++){
        bargs[i].id = i;
        bargs[i].flower_field = &flower_field;
        pthread_create(&bees[i], NULL, bee_thread, &bargs[i]);
    }

    for(int i = 0; i < num_bees; i++){
        pthread_join(bees[i], NULL);
    }

    printf("simulation ending\n");

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pthread_mutex_destroy(&flower_field.locks[i][j]);
        }
        free(flower_field.field[i]);
        free(flower_field.locks[i]);
    }
    free(flower_field.field);
    free(flower_field.locks);
    free(bargs);
    free(bees);
    free(flower_field.queue);

    pthread_mutex_destroy(&flower_field.queue_lock);

    return 0;
}
