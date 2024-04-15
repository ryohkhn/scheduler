CFLAGS = -Wall
FAST = -O2

all: test

build/stack.o: stack.c stack.h
	gcc $(CFLAGS) -c stack.c -o build/stack.o

test_stack: test_stack.c build/stack.o stack.h stack.c
	gcc $(CFLAGS) test_stack.c build/stack.o -o test/test_stack

build/dumb_sched.o: dumb_sched.c sched.h stack.h
	gcc $(CFLAGS) -c dumb_sched.c -o build/dumb_sched.o

build/deque.o: deque.c deque.h
	gcc $(CFLAGS) -c deque.c -o build/deque.o

test_deque: test_deque.c build/deque.o deque.h deque.c
	gcc $(CFLAGS) test_deque.c build/deque.o -o test/test_deque

test_dumb_sched: test_dumb_sched.c dumb_sched.c build/dumb_sched.o build/stack.o
	gcc $(CFLAGS) test_dumb_sched.c build/dumb_sched.o build/stack.o -o test/test_dumb_sched

test_dumb_quicksort: quicksort.c dumb_sched.c build/dumb_sched.o build/stack.o
	gcc $(CFLAGS) quicksort.c build/dumb_sched.o build/stack.o -o test/test_dumb_quicksort

concurrent_test: concurrent_tester.c dumb_sched.c build/dumb_sched.o build/stack.o
	gcc $(CFLAGS) concurrent_tester.c build/dumb_sched.o build/stack.o -o test/concurrent_test

test: test_stack test_deque test_dumb_sched concurrent_test

clean:
ifneq ("$(wildcard build/*.o)", "")
	rm build/*.o
endif
ifneq ("$(wildcard test/*)", "")
	rm test/*
endif
