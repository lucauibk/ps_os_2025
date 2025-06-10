CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -pthread -O3

.PHONY : all clean
all: beehive

beehive: beehive.o
	@echo "Linking beehive..."
	$(CC) $(CFLAGS) beehive.o -o beehive

beehive.o: beehive.c
	@echo "Compiling beehive.c..."
	$(CC) $(CFLAGS) -c beehive.c -o beehive.o
clean:
	@rm -f *.o beehive
	@echo "Cleanup complete."