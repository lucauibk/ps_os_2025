#include <stdio.h>  
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // für usleep

typedef struct {
    int id;
    int materials;
    int workers;
    pthread_mutex_t lock;
} construction_site;

typedef struct {
    int worker_id;
    construction_site* site;
} thread_arg;

void* worker(void* arg) {
    thread_arg* t_arg = (thread_arg*)arg;
    int worker_id = t_arg->worker_id;
    construction_site* site = t_arg->site;

    while (1) {
        pthread_mutex_lock(&site->lock);
        if (site->materials > 0) {
            site->materials--;
            printf("Worker %d took a material. Remaining: %d\n", worker_id, site->materials);
            pthread_mutex_unlock(&site->lock);
        } else {
            pthread_mutex_unlock(&site->lock);
            break;
        }
        usleep(100000); // 100 ms, simuliert Arbeit
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <materials> <workers>\n", argv[0]);
        return 1;
    }

    int materials = atoi(argv[1]);
    int workers = atoi(argv[2]);

    if (materials <= 0 || workers <= 0) {
        printf("Both materials and workers must be positive integers.\n");
        return 1;
    }

    construction_site site;
    site.materials = materials;
    site.workers = workers;
    site.id = 0;
    pthread_mutex_init(&site.lock, NULL);

    pthread_t* threads = malloc(workers * sizeof(pthread_t));
    thread_arg* args = malloc(workers * sizeof(thread_arg));
    if (threads == NULL || args == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    for (int i = 0; i < workers; i++) {
        args[i].worker_id = i + 1;
        args[i].site = &site;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < workers; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All workers finished. Total materials used: %d\n", materials - site.materials);

    free(threads);
    free(args);
    pthread_mutex_destroy(&site.lock);

    return 0;
}
