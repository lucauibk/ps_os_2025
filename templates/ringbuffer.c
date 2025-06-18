#include "ringbuffer.h"

void ring_init(RingBuffer* rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool ring_is_full(RingBuffer* rb) {
    return rb->count == RING_SIZE;
}

bool ring_is_empty(RingBuffer* rb) {
    return rb->count == 0;
}

bool ring_put(RingBuffer* rb, int value) {
    if (ring_is_full(rb)) return false;

    rb->buffer[rb->head] = value;
    rb->head = (rb->head + 1) % RING_SIZE;
    rb->count++;
    return true;
}

bool ring_get(RingBuffer* rb, int* out) {
    if (ring_is_empty(rb)) return false;
    
    *out = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_SIZE;
    rb->count--;
    return true;
}
