CC=gcc
CFLAGS= -pthread -Wall -Wextra -pedantic -std=c11

all: server
server: server.o
	$(CC) $(CFLAGS) -o server server.o
server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f server.o server
.PHONY: all clean