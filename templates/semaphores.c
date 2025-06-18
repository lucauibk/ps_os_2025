#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>

sem_t sem;

void* task(void* arg) {
    sem_wait(&sem); // Nur einer darf rein
    printf("Thread %ld running...\n", (long)arg);
    sleep(1);
    printf("Thread %ld done.\n", (long)arg);
    sem_post(&sem);
    return NULL;
}

int main() {
    sem_init(&sem, 0, 1); // Binärsema

    pthread_t t1, t2;
    pthread_create(&t1, NULL, task, (void*)1);
    pthread_create(&t2, NULL, task, (void*)2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem);
    return 0;
}
