CC=gcc
CFLAGS= -pthread -Wall -Wextra -pedantic -std=c11

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
