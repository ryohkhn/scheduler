CFLAGS = -Wall
FAST = -O2

all: test

build/stack.o: stack.c stack.h
	gcc $(CFLAGS) -c stack.c -o build/stack.o

test_stack: test_stack.c build/stack.o
	gcc $(CFLAGS) test_stack.c build/stack.o -o test/test_stack

build/dumb_sched.o: dumb_sched.c sched.h stack.h
	gcc $(CFLAGS) -c dumb_sched.c -o build/dumb_sched.o

test_dumb_sched: test_dumb_sched.c dumb_sched.c build/dumb_sched.o build/stack.o
	gcc $(CFLAGS) test_dumb_sched.c build/dumb_sched.o build/stack.o -o test/test_dumb_sched

# test: test_stack test_dumb_sched
test: test_dumb_sched

clean:
ifneq ("$(wildcard build/*.o)", "")
	rm build/*.o
endif
ifneq ("$(wildcard test/*)", "")
	rm test/*
endif
