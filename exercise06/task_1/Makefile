CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O3

.PHONY: all

all: task_1_atomic task_1_mutex

task_1_atomic: task_1_atomic.c
	$(CC) $(CFLAGS) -o task_1_atomic task_1_atomic.c -lpthread
task_1_mutex: task_1_mutex.c
	$(CC) $(CFLAGS) -o task_1_mutex task_1_mutex.c -lpthread
clean:
	@rm -f *.o task_1_atomic task_1_mutex