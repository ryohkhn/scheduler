CC = gcc

CFLAGS = -Wall -Wextra

FAST = -O2

INCLUDE = include

all: test

build/stack.o: src/stack.c $(INCLUDE)/stack.h
	$(CC) $(CFLAGS) -c src/stack.c -o build/stack.o

test_stack: test/test_stack.c build/stack.o $(INCLUDE)/stack.h src/stack.c
	$(CC) $(cflags) test/test_stack.c build/stack.o -o test/test_stack

build/dumb_sched.o: src/dumb_sched.c $(INCLUDE)/sched.h $(INCLUDE)/stack.h
	$(CC) $(CFLAGS) -c src/dumb_sched.c -o build/dumb_sched.o

build/stealing_sched.o: src/stealing_sched.c $(INCLUDE)/sched.h $(INCLUDE)/deque.h
	$(CC) $(CFLAGS) -c src/stealing_sched.c -o build/stealing_sched.o

build/deque.o: src/deque.c $(INCLUDE)/deque.h
	$(CC) $(CFLAGS) -c src/deque.c -o build/deque.o

test_deque: test/test_deque.c build/deque.o $(INCLUDE)/deque.h src/deque.c
	$(CC) $(CFLAGS) test/test_deque.c build/deque.o -o test/test_deque

test_dumb_sched: test/test_dumb_sched.c src/dumb_sched.c build/dumb_sched.o build/stack.o
	$(CC) $(CFLAGS) test/test_dumb_sched.c build/dumb_sched.o build/stack.o -o test/test_dumb_sched

test_dumb_quicksort: src/quicksort.c src/dumb_sched.c build/dumb_sched.o build/stack.o
	$(CC) $(CFLAGS) src/quicksort.c build/dumb_sched.o build/stack.o -o test/test_dumb_quicksort

concurrent_test: src/concurrent_tester.c src/dumb_sched.c build/dumb_sched.o build/stack.o
	$(CC) $(CFLAGS) src/concurrent_tester.c build/dumb_sched.o build/stack.o -o test/concurrent_test

test_stealing_sched: test/test_stealing_sched.c src/stealing_sched.c build/stealing_sched.o build/deque.o
	$(CC) $(CFLAGS) test/test_stealing_sched.c build/stealing_sched.o build/deque.o -o test/test_stealing_sched

test: test_stack test_deque test_dumb_sched concurrent_test test_dumb_quicksort

clean:
	rm -f test/test_stack test/test_deque test/test_dumb_sched test/concurrent_test test/test_dumb_quicksort test/test_stealing_sched

ifneq ("$(wildcard build/*.o)", "")
	rm build/*.o
endif
ifneq ("$(wildcard test/*.o)", "")
	rm test/*.o
endif

.PHONY:	all test clean