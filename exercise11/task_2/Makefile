# Compiler and flags
CC = gcc
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS += -std=c11 -Wall -Wextra -pedantic -pthread -g -O3 -fPIC
LDFLAGS += -pthread
DLFLAGS = -ldl

# Binaries and shared libs
TARGET = map_string
PLUGINS = caesar_cipher.so xor_string.so reverse_string.so rotate_string.so library_one.so library_two.so

# Source files
PLUGIN_SRCS = $(PLUGINS:.so=.c)

.PHONY: all clean

all: $(TARGET) $(PLUGINS)

# Main program
$(TARGET): map_string.o
	$(CC) $(CFLAGS) -o $@ $^ $(DLFLAGS)

# Plugin rule
%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -o $@ $<

# Cleanup
clean:
	$(RM) $(TARGET) *.o *.so
