CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O2
LDFLAGS = -shared
TARGET = malloc_spy.so

all: $(TARGET)

$(TARGET): malloc_spy.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
