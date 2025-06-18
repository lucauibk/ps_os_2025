CC = gcc
CFLAGS = -Wall -Wextra -O0 -pthread -g
SRC = test.c
TARGET = test2

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
.PHONY: all clean