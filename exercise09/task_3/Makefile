CC=gcc
CFLAGS=-std=c11 -Wpedantic -D_DEFAULT_SOURCE -pthread -Wall -Wextra -O2

.PHONY: all clean

all: task_3

task_3: task_3.o my_pthread_barrier.o
	$(CC) $(CFLAGS) -o task_3 task_3.o my_pthread_barrier.o

task_3.o: task_3.c my_pthread_barrier.h
	$(CC) $(CFLAGS) -c task_3.c

my_pthread_barrier.o: my_pthread_barrier.c my_pthread_barrier.h
	$(CC) $(CFLAGS) -c my_pthread_barrier.c

clean:
	rm -f *.o task_3
