NESTS_PER_BUCKET=4
PREFIX=/usr

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99 -g -Ofast -I include -fPIC

SOURCE := $(wildcard src/*.c)
OBJECTS := $(SOURCE:src/%.c=build/%.o)

TSOURCE := $(wildcard tests/*.c)
TESTS := $(TSOURCE:tests/%.c=tests/build/%)

ESOURCE := $(wildcard example/*.c)
EXSAMPLES := $(ESOURCE:example/%.c=example/build/%)

all: build/libbloomfilter.so build/libbloomfilter.a tests examples

tests: $(TESTS)

examples: $(EXSAMPLES)

run_tests: tests/build/tests
	tests/build/tests

benchmark: tests/build/benchmark
	tests/build/benchmark

examples/build/%: $(OBJECTS) example/%.c examples/build
	$(CC) $(CFLAGS) -lcheck -lrt $^ -o $@

tests/build/%: $(OBJECTS) tests/%.c
	$(CC) $(CFLAGS) -lcheck -lrt $^ -o $@

build/libbloomfilter.so: $(OBJECTS)
	$(CC) $(CFLAGS) -lrt -shared $^ -o $@

build/libbloomfilter.a: $(OBJECTS)
	ar rs $@ $^

build:
	mkdir build

tests/build:
	mkdir tests/build

examples/build:
	mkdir examples/build

clean:
	rm -rf build tests/build examples/build

build/%.o: src/%.c build
	$(CC) $(CFLAGS) -c $< -o $@

install: build/libbloomfilter.so build/libbloomfilter.a
	install build/libbloomfilter.so $(PREFIX)/lib
	install build/libbloomfilter.a $(PREFIX)/lib
	install src/bloomfilter.h $(PREFIX)/include
