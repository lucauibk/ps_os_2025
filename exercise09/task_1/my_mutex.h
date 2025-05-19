#ifndef MY_MUTEX_H_
#define MY_MUTEX_H_

#include <stdatomic.h>
#include <sched.h>

typedef struct {
    atomic_flag flag;
} my_mutex_t;

static inline void my_mutex_init(my_mutex_t *m) {
    atomic_flag_clear(&m->flag);
}

static inline void my_mutex_lock(my_mutex_t *m) {
    while (atomic_flag_test_and_set(&m->flag)) {
        sched_yield(); // freiwilliges Yielding
    }
}

static inline void my_mutex_unlock(my_mutex_t *m) {
    atomic_flag_clear(&m->flag);
}

#endif
