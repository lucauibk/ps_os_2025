CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all
all: task_2
task_2: task_2.c
	$(CC) $(CFLAGS) -o task_2 task_2.c -lpthread
clean:
	@rm -f *.o task_2