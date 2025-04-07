CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all

all: sheduler service

sheduler: sheduler.c
	$(CC) $(CFLAGS) -o sheduler sheduler.c

service: service.c
	$(CC) $(CFLAGS) -o service service.c

clean:
	@rm -f *.o sheduler service