#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <pthread.h>

#define RING_SIZE 16  // anpassbare Größe

typedef struct {
    int buffer[RING_SIZE];
    int head; // schreibt hier
    int tail; // liest hier
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t full, empty;

} RingBuffer;

// Initialisierung
void ring_init(RingBuffer* rb);

// Einfügen eines Elements
bool ring_put(RingBuffer* rb, int value);

// Lesen eines Elements
bool ring_get(RingBuffer* rb, int* out);

// Status
bool ring_is_full(RingBuffer* rb);
bool ring_is_empty(RingBuffer* rb);

#endif
