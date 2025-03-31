CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
ME := $(shell whoami)

.PHONY: all
all: compile

run:
	./main

compile: main.o key.o
	@echo "Linking main..."
	$(CC) $(CFLAGS) -o main main.o key.o

main.o: main.c key.h
	@echo "Compiling main.c..."
	$(CC) $(CFLAGS) -c main.c -o main.o

key.o: key.c key.h
	@echo "Compiling key.c..."
	$(CC) $(CFLAGS) -c key.c -o key.o

create_key: key.c

key.c:
	@echo "Creating key.c..."
	@echo '#include "key.h"\nconst char* get_key() {\n\treturn "$(ME)";\n}' > key.c

.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -f key.c main *.o