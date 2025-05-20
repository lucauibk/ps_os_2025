#include "my_pthread_barrier.h"
#include <errno.h>

int my_pthread_barrier_init(my_pthread_barrier_t *barrier, UNUSED_PARAM void* attr, int count) {
    if (count <= 0) return EINVAL;

    int ret;

    if ((ret = pthread_mutex_init(&barrier->mutex, NULL)) != 0)
        return ret;
    if ((ret = pthread_cond_init(&barrier->cond, NULL)) != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return ret;
    }

    barrier->count = count;
    barrier->waiting = 0;
    barrier->generation = 0;

    return 0;
}

int my_pthread_barrier_wait(my_pthread_barrier_t *barrier) {
    int ret = 0;

    pthread_mutex_lock(&barrier->mutex);

    int gen = barrier->generation;

    barrier->waiting++;

    if (barrier->waiting == barrier->count) {
        // Letzter Thread an der Barrier: reset warten, erhöhe Generation
        barrier->generation++;
        barrier->waiting = 0;

        // Alle Threads wecken
        pthread_cond_broadcast(&barrier->cond);

        // Speziellen Wert zurückgeben (Serial Thread)
        ret = PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
        // Warten bis alle Threads ankommen (Generation ändert sich)
        while (gen == barrier->generation) {
            pthread_cond_wait(&barrier->cond, &barrier->mutex);
        }
        ret = 0;
    }

    pthread_mutex_unlock(&barrier->mutex);

    return ret;
}

int my_pthread_barrier_destroy(my_pthread_barrier_t *barrier) {
    int ret1 = pthread_mutex_destroy(&barrier->mutex);
    int ret2 = pthread_cond_destroy(&barrier->cond);

    return ret1 != 0 ? ret1 : ret2;
}