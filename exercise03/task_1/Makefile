CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

.PHONY: all clean

all: signal_handler

signal_handler: signal_handler.o
	@echo "Linking signal_handler..."
	$(CC) $(CFLAGS) -o signal_handler signal_handler.o

signal_handler.o: signal_handler.c
	@echo "Compiling signal_handler.c..."
	$(CC) $(CFLAGS) -c signal_handler.c -o signal_handler.o

clean:
	@rm -f *.o signal_handler	