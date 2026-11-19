
CC = gcc

CPPFLAGS = -Iincludes
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O0 -pthread
LDFLAGS = -pthread

SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))

.PHONY: all clean check-toolchain

all: hw1

hw1: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

check-toolchain:
	$(CC) --version
	$(MAKE) --version

clean:
	rm -rf build hw1
