# Compiler und Flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2
DEBUGFLAGS = -g -DDEBUG
RELEASEFLAGS = -O3

# Standard: Debug-Modus aus
DEBUG ?= 0

ifeq ($(DEBUG),1)
  CFLAGS += $(DEBUGFLAGS)
else
  CFLAGS += $(RELEASEFLAGS)
endif

LDFLAGS =
LDLIBS =

# Quell- und Ziel-Dateien (Beispiel, bitte anpassen)
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

# Executable Name
TARGET = main

# Für Shared Library
SHARED_LIB_NAME = libmylib.so
SHARED_LIB_SOURCES = $(wildcard lib/*.c)
SHARED_LIB_OBJS = $(SHARED_LIB_SOURCES:.c=.o)
SHARED_LIB_FLAGS = -fPIC -shared

.PHONY: all clean install debug release

all: $(TARGET)

# Regel für Executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Objektdateien mit automatischen Abhängigkeiten
-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Shared Library bauen
sharedlib: $(SHARED_LIB_OBJS)
	$(CC) $(SHARED_LIB_FLAGS) -o $(SHARED_LIB_NAME) $^

# Clean
clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d) $(SHARED_LIB_OBJS) $(SHARED_LIB_NAME)

# Debug aktivieren
debug:
	$(MAKE) DEBUG=1

# Release Build
release:
	$(MAKE) DEBUG=0

# Install Beispiel (Pfad anpassen)
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# Deinstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
