CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -pthread

.PHONY: all clean

all: use_pthread use_my_mutex

use_pthread: task_1.c
	$(CC) $(CFLAGS) -DUSE_MY_MUTEX=0 -o use_pthread task_1.c

use_my_mutex: task_1.c
	$(CC) $(CFLAGS) -DUSE_MY_MUTEX=1 -o use_my_mutex task_1.c

clean:
	rm -f use_pthread use_my_mutex