CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -pthread -O3

.PHONY : all clean
all: post_office

post_office: post_office.o
	@echo "Linking post_office..."
	$(CC) $(CFLAGS) post_office.o -o post_office

post_office.o: post_office.c
	@echo "Compiling post_office.c..."
	$(CC) $(CFLAGS) -c post_office.c -o post_office.o
clean:
	@rm -f *.o post_office
	@echo "Cleanup complete."