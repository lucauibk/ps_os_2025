#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "myqueue.h"
typedef struct {
    int id;
    int width;
    int height;
    int* count_visited;
    pthread_mutex_t* mutex;
    myqueue* queue;
    int** flowerfield;
} bee_data;


void* bee_thread(void* arg) {
    bee_data* data = (bee_data*)arg;
    unsigned int seed = (unsigned int)pthread_self();

    while (1) {
        pthread_mutex_lock(data->mutex);
        if (myqueue_is_empty(data->queue)) {
            pthread_mutex_unlock(data->mutex);
            printf("Bee %d is working in beehive.\n", data->id);
            usleep((rand_r(&seed) % 401 + 100) * 1000);
            continue;
        }

        coordinate flower = myqueue_pop(data->queue);
        printf("Bee %d is flying to food source at position (%d,%d).\n", data->id, flower.x, flower.y);

        if (data->flowerfield[flower.x][flower.y] == 0) {
            usleep((rand_r(&seed) % 401 + 100) * 1000);
            data->flowerfield[flower.x][flower.y] = 1;
            (*data->count_visited)++;

            printf("Bee %d collected nectar at position (%d,%d) and reports potential food sources:", data->id, flower.x, flower.y);

            int directions[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
            for (int i = 0; i < 4; i++) {
                int nx = flower.x + directions[i][0];
                int ny = flower.y + directions[i][1];
                if (nx >= 0 && nx < data->width && ny >= 0 && ny < data->height &&
                    data->flowerfield[nx][ny] == 0) {
                    myqueue_push(data->queue, (coordinate){nx, ny});
                    printf(" (%d,%d)", nx, ny);
                }
            }
            printf(".\n");
        }

        if (*data->count_visited >= data->width * data->height) {
            pthread_mutex_unlock(data->mutex);
            break;
        }
        pthread_mutex_unlock(data->mutex);
        usleep((rand_r(&seed) % 401 + 100) * 1000);
    }
    return NULL;
}


int main(int argc, char* argv[]){
    if(argc != 4){
        fprintf(stderr, "Usage: %s <flowerfield_width> <flowerfield_height> <number of bees>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int num_bees = atoi(argv[3]);
    int** flowerfield = malloc(width * sizeof(int*));
    for (int i = 0; i < width; ++i) {
        flowerfield[i] = malloc(height * sizeof(int));
        for (int j = 0; j < height; ++j) {
            flowerfield[i][j] = 0;
        }
    }

    myqueue* queue = malloc(sizeof(myqueue));
    myqueue_init(queue);
    bee_data data = {
        .width = width,
        .height = height,
        .mutex = malloc(sizeof(pthread_mutex_t)),
        .queue = queue,
        .flowerfield = flowerfield,
        .count_visited = calloc(1, sizeof(int))
    };
    pthread_mutex_init(data.mutex, NULL);
    myqueue_push(queue, (coordinate){0, 0});
    pthread_t* bees = malloc(num_bees * sizeof(pthread_t));
    if(bees == NULL){
        perror("Failed to allocate memory for bees");
        return EXIT_FAILURE;
    }
    for(int i = 0; i < num_bees; i++){
        if(pthread_create(&bees[i], NULL, bee_thread, &data) != 0){
            perror("Failed to create bee thread");
            free(bees);
            return EXIT_FAILURE;
        }
    }
    for(int i = 0; i < num_bees; i++){
        if(pthread_join(&bees[i], NULL) != 0){
            perror("Failed to join bee thread");
            free(bees);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < width; ++i)
    free(flowerfield[i]);
    free(flowerfield);
    free(bees);
    pthread_mutex_destroy(data.mutex);
    free(data.mutex);
    free(data.count_visited);
    free(queue);

}
