CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all clean

all: arg_counter

arg_counter: arg_counter.o
	$(CC) $(CFLAGS) -o $@ $^	

arg_counter.o: arg_counter.c
	$(CC) $(CFLAGS) -c $<



clean:
	@rm -f *.o task_1