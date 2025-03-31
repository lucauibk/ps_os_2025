CC = gcc
CFLAGS = -std=c11 -Wall -Wextra

.PHONY: all clean

all: task3
task3: task3.o
	@echo "Linking task3..."
	$(CC) $(CFLAGS) -o task3 task3.o
task3.o: task3.c
	@echo "Compiling task3.c..."
	$(CC) $(CFLAGS) -c task3.c -o task3.o
clean:
	@rm -f *.o task3