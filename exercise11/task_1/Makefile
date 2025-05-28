CFLAGS = -fPIC -Wall -Wextra -O2
LDFLAGS = -shared

all: some_library.so

some_library.so: some_library.o
	$(CC) $(LDFLAGS) -o $@ $^

some_library.o: some_library.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so
