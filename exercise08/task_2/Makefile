CC=gcc
CFLAGS= -pthread -Wall -Wextra -pedantic -std=c11

all: server client
server: server.o
	$(CC) $(CFLAGS) -o server server.o
server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client: client.o
	$(CC) $(CFLAGS) -o client client.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c	

clean:
	rm -f server.o server client.o client
.PHONY: all clean