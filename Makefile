CFLAGS = -Wall
FAST = -O2

all: test

build/stack.o: stack.c stack.h
	gcc $(CFLAGS) -c stack.c -o build/stack.o

test_stack: build/stack.o
	gcc $(CFLAGS) build/stack.o -o test/test_stack

test: test_stack

clean:
ifneq ("$(wildcard build/*.o)", "")
	rm build/*.o
endif
ifneq ("$(wildcard test/*)", "")
	rm test/*
endif