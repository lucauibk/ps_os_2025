// zombiesurvival.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_WIDTH 100
#define MAX_HEIGHT 100

typedef enum { EMPTY, SURVIVOR, ZOMBIE } CellType;

typedef struct {
    int x, y;
    bool alive;
    int kills;
    int id;
} Survivor;

int width, height, num_survivors, num_zombies;
CellType city[MAX_WIDTH][MAX_HEIGHT];
pthread_mutex_t city_lock[MAX_WIDTH][MAX_HEIGHT];
Survivor *survivors;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};

void print_city() {
    pthread_mutex_lock(&print_lock);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (city[x][y]) {
                case EMPTY: printf("."); break;
                case SURVIVOR: printf("S"); break;
                case ZOMBIE: printf("Z"); break;
            }
        }
        printf("\n");
    }
    printf("----------------------\n");
    pthread_mutex_unlock(&print_lock);
}

void *survivor_thread(void *arg) {
    Survivor *s = (Survivor *)arg;
    while (s->alive) {
        usleep((rand() % 200 + 100) * 1000); // move delay
        int dx = 0, dy = 0;
        int dir = rand() % 4;
        dx = directions[dir][0];
        dy = directions[dir][1];

        int new_x = s->x + dx;
        int new_y = s->y + dy;
        if (new_x < 0 || new_y < 0 || new_x >= width || new_y >= height) continue;

        pthread_mutex_lock(&city_lock[new_x][new_y]);
        if (city[new_x][new_y] == ZOMBIE) {
            if ((rand() % 100) < 60) { // kills zombie
                city[new_x][new_y] = SURVIVOR;
                city[s->x][s->y] = EMPTY;
                pthread_mutex_unlock(&city_lock[new_x][new_y]);
                s->x = new_x;
                s->y = new_y;
                s->kills++;
                continue;
            } else {
                s->alive = false;
                city[s->x][s->y] = EMPTY;
                pthread_mutex_unlock(&city_lock[new_x][new_y]);
                return NULL;
            }
        } else if (city[new_x][new_y] == EMPTY) {
            city[new_x][new_y] = SURVIVOR;
            city[s->x][s->y] = EMPTY;
            pthread_mutex_unlock(&city_lock[new_x][new_y]);
            s->x = new_x;
            s->y = new_y;
        } else {
            pthread_mutex_unlock(&city_lock[new_x][new_y]);
        }
    }
    return NULL;
}

void *zombie_spawner(void *arg) {
    int spawned = 0;
    while (1) {
        sleep(2);
        int zx = rand() % width;
        int zy = rand() % height;
        pthread_mutex_lock(&city_lock[zx][zy]);
        if (city[zx][zy] == EMPTY) {
            city[zx][zy] = ZOMBIE;
            spawned++;
        }
        pthread_mutex_unlock(&city_lock[zx][zy]);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <width> <height> <survivors> <zombies>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));

    width = atoi(argv[1]);
    height = atoi(argv[2]);
    num_survivors = atoi(argv[3]);
    num_zombies = atoi(argv[4]);

    if (width > MAX_WIDTH || height > MAX_HEIGHT) {
        printf("Grid too large (max %dx%d)\n", MAX_WIDTH, MAX_HEIGHT);
        exit(1);
    }

    // Init city locks and grid
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y) {
            city[x][y] = EMPTY;
            pthread_mutex_init(&city_lock[x][y], NULL);
        }

    survivors = calloc(num_survivors, sizeof(Survivor));
    pthread_t *threads = malloc(num_survivors * sizeof(pthread_t));

    for (int i = 0; i < num_survivors; ++i) {
        int sx, sy;
        do {
            sx = rand() % width;
            sy = rand() % height;
        } while (city[sx][sy] != EMPTY);
        survivors[i].x = sx;
        survivors[i].y = sy;
        survivors[i].alive = true;
        survivors[i].kills = 0;
        survivors[i].id = i;
        city[sx][sy] = SURVIVOR;
        pthread_create(&threads[i], NULL, survivor_thread, &survivors[i]);
    }

    // Initial zombies
    for (int i = 0; i < num_zombies; ++i) {
        int zx, zy;
        do {
            zx = rand() % width;
            zy = rand() % height;
        } while (city[zx][zy] != EMPTY);
        city[zx][zy] = ZOMBIE;
    }

    pthread_t zspawn;
    pthread_create(&zspawn, NULL, zombie_spawner, NULL);

    for (int i = 0; i < 30; ++i) {
        sleep(2);
        print_city();
    }

    for (int i = 0; i < num_survivors; ++i) {
        survivors[i].alive = false;
        pthread_join(threads[i], NULL);
    }

    free(threads);
    pthread_cancel(zspawn);

    printf("Final report:\n");
    for (int i = 0; i < num_survivors; ++i) {
        printf("Survivor %d: %s, kills: %d\n", i, survivors[i].alive ? "alive" : "dead", survivors[i].kills);
    }

    free(survivors);
    return 0;
}
