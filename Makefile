CC = gcc

CFLAGS = -Wall -Wextra

FAST = -O2

INCLUDE = include

SRCS_DIR = src

BUILD_DIR = build

TESTS_DIR = test

OUT_DIR = out

LDLIBS = `pkg-config --libs gtk4`

TESTS = test_stack test_deque test_dumb_sched test_dumb_sched_sem concurrent_test test_dumb_quicksort test_dumb_quicksort_sem test_stealing_sched test_mandelbrot_dumb

all: test

# General build rules
$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Specific rules for files with dependencies
$(BUILD_DIR)/dumb_sched.o: $(SRCS_DIR)/dumb_sched.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/dumb_sched_sem.o: $(SRCS_DIR)/dumb_sched_sem.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched.o: $(SRCS_DIR)/stealing_sched.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

# Tests and dependencies

test_stack: $(TESTS_DIR)/test_stack.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_deque: $(TESTS_DIR)/test_deque.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_dumb_sched: $(TESTS_DIR)/test_dumb_sched.c $(BUILD_DIR)/dumb_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_dumb_sched_sem: $(TESTS_DIR)/test_dumb_sched.c $(BUILD_DIR)/dumb_sched_sem.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_dumb_quicksort: $(TESTS_DIR)/quicksort.c $(BUILD_DIR)/dumb_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_dumb_quicksort_sem: $(TESTS_DIR)/quicksort.c $(BUILD_DIR)/dumb_sched_sem.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

concurrent_test: $(TESTS_DIR)/concurrent_tester.c $(BUILD_DIR)/dumb_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_stealing_sched: $(TESTS_DIR)/test_stealing_sched.c $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

$(BUILD_DIR)/mandelbrot_dumb.o: $(TESTS_DIR)/mandelbrot.c
	$(CC) $(CFLAGS) $(FAST) -ffast-math `pkg-config --cflags gtk4` -c -o $(BUILD_DIR)/mandelbrot_dumb.o $(TESTS_DIR)/mandelbrot.c

test_mandelbrot_dumb: $(BUILD_DIR)/mandelbrot_dumb.o $(BUILD_DIR)/dumb_sched.o
	$(CC) $(BUILD_DIR)/mandelbrot_dumb.o $(BUILD_DIR)/dumb_sched.o $(BUILD_DIR)/stack.o `pkg-config --libs gtk4` -o $(OUT_DIR)/test_mandelbrot_dumb

test: $(TESTS)

#rm -f $(addprefix $(TESTS_DIR)/, $(TESTS))
clean:
ifneq ("$(wildcard $(BUILD_DIR)/*.o)", "")
	rm $(BUILD_DIR)/*.o
endif
ifneq ("$(wildcard $(OUT_DIR)/*)", "")
	rm $(OUT_DIR)/*
endif

.PHONY:	all test clean
