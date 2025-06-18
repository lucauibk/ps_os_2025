#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int space;
    int max_space;
    int* band;
    pthread_mutex_t lock;
} transportband_t;

typedef struct {
    transportband_t band;
    int workers;
    int tools;
    int stop; // Flag to stop the simulation
    pthread_mutex_t* tool_locks;
} construction_site;

typedef struct {
    int worker_id;
    construction_site* site;
} worker_arg;

int check_for_tool(construction_site* site, int worker_id) {
    for (int i = 0; i < site->tools; i++) {
        if (pthread_mutex_trylock(&site->tool_locks[i]) == 0) {
            // Werkzeug verfügbar
            printf("Worker %d took tool %d\n", worker_id, i);
            return i;
        }
    }
    return -1;
}

void release_tool(construction_site* site, int tool_id) {
    pthread_mutex_unlock(&site->tool_locks[tool_id]);
}

void take_material(transportband_t* band, int worker_id) {
    pthread_mutex_lock(&band->lock);
    for (int i = 0; i < band->max_space; i++) {
        if (band->band[i] == 1) {
            band->band[i] = 0;
            band->space++;
            printf("Worker %d took material from position %d\n", worker_id, i);
            pthread_mutex_unlock(&band->lock);
            return;
        }
    }
    pthread_mutex_unlock(&band->lock);
    printf("Worker %d: No material available to take.\n", worker_id);
}

void* worker(void* arg) {
    worker_arg* w_arg = (worker_arg*)arg;
    construction_site* site = w_arg->site;
    int worker_id = w_arg->worker_id;

    while (!site->stop) {
        int tool_id = check_for_tool(site, worker_id);
        if (tool_id >= 0) {
            pthread_mutex_lock(&site->band.lock);
            int has_material = (site->band.space < site->band.max_space);
            pthread_mutex_unlock(&site->band.lock);

            if (has_material) {
                take_material(&site->band, worker_id);
            } else {
                printf("Worker %d: No material on band, waiting...\n", worker_id);
                usleep(1000000); // simulate waiting
            }

            release_tool(site, tool_id);
            usleep(1000000); // simulate work
        } else {
            printf("Worker %d: No tools available, waiting...\n", worker_id);
            usleep(1000000);
        }
    }
    return NULL;
}

void add_material(transportband_t* band) {
    pthread_mutex_lock(&band->lock);
    for (int i = 0; i < band->max_space; i++) {
        if (band->band[i] == 0) {
            band->band[i] = 1;
            band->space--;
            printf("Producer added material at position %d\n", i);
            pthread_mutex_unlock(&band->lock);
            return;
        }
    }
    pthread_mutex_unlock(&band->lock);
}

void* producer(void* arg) {
    construction_site* site = (construction_site*)arg;
    while(!site->stop) {
        pthread_mutex_lock(&site->band.lock);
        int space_available = (site->band.space > 0);
        pthread_mutex_unlock(&site->band.lock);

        if (space_available) {
            add_material(&site->band);
        } else {
            printf("Transport band full, producer waiting...\n");
            usleep(500000); //
        }
        // simulate production time
    }
    return NULL;
}

void* timer_thread(void* arg) {
    construction_site* site = (construction_site*)arg;
    sleep(10); // Laufzeit in Sekunden
    site->stop = 1;
    printf("Timer: Stopping simulation.\n");
    return NULL;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <k> <workers> <tools>\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    int workers = atoi(argv[2]);
    int tools = atoi(argv[3]);

    if (k <= 0 || workers <= 0 || tools <= 0) {
        printf("All arguments must be positive integers.\n");
        return 1;
    }

    construction_site site;
    site.band.max_space = k;
    site.band.space = k;
    site.band.band = calloc(k, sizeof(int));
    pthread_mutex_init(&site.band.lock, NULL);

    site.workers = workers;
    site.tools = tools;
    site.tool_locks = malloc(tools * sizeof(pthread_mutex_t));
    for (int i = 0; i < tools; i++) {
        pthread_mutex_init(&site.tool_locks[i], NULL);
    }

    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer, &site);
    pthread_t timer;
    pthread_create(&timer, NULL, timer_thread, &site);

    pthread_t* worker_threads = malloc(workers * sizeof(pthread_t));
    worker_arg* args = malloc(workers * sizeof(worker_arg));

    for (int i = 0; i < workers; i++) {
        args[i].worker_id = i;
        args[i].site = &site;
        pthread_create(&worker_threads[i], NULL, worker, &args[i]);
    }

    pthread_join(producer_thread, NULL);
    pthread_join(timer, NULL);
    for (int i = 0; i < workers; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    // Cleanup (unreachable, aber korrektheitshalber)
    pthread_mutex_destroy(&site.band.lock);
    for (int i = 0; i < tools; i++) {
        pthread_mutex_destroy(&site.tool_locks[i]);
    }
    free(site.tool_locks);
    free(site.band.band);
    free(worker_threads);
    free(args);
    return 0;
}
