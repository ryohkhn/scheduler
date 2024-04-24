CC = gcc

CFLAGS = -Wall -Wextra -I$(INCLUDE) $(FAST)

FAST = -O2

INCLUDE = include

SRCS_DIR = src

BUILD_DIR = build

TESTS_DIR = test

DEMOS_DIR = demo

OUT_DIR = out

LDLIBS = `pkg-config --cflags --libs gtk4`

# TESTS = test_stack test_deque test_lifo_sched test_lifo_sched_sem concurrent_test test_lifo_quicksort test_lifo_quicksort_sem test_stealing_sched test_stealing_quicksort test_stealing_quicksort_sem test_stealing_quicksort_cond
TESTS = test_lifo_quicksort test_lifo_quicksort_sem test_stealing_quicksort test_stealing_quicksort_sem test_stealing_quicksort_cond test_mandelbrot_lifo test_mandelbrot_stealing_cond

BENCHMARK_FILE = quicksort.c

all: test

# General build rules
$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Specific rules for files with dependencies
$(BUILD_DIR)/lifo_sched.o: $(SRCS_DIR)/lifo_sched.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lifo_sched_sem.o: $(SRCS_DIR)/lifo_sched_sem.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched.o: $(SRCS_DIR)/stealing_sched.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched_sem.o: $(SRCS_DIR)/stealing_sched_sem.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched_cond.o: $(SRCS_DIR)/stealing_sched_cond.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mandelbrot.o: $(DEMOS_DIR)/mandelbrot.c
	$(CC) $(CFLAGS) -ffast-math $(LDLIBS) -c $< -o $@

# Tests and dependencies

test_stack: $(TESTS_DIR)/test_stack.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_deque: $(TESTS_DIR)/test_deque.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_lifo_sched: $(TESTS_DIR)/test_lifo_sched.c $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_lifo_sched_sem: $(TESTS_DIR)/test_lifo_sched.c $(BUILD_DIR)/lifo_sched_sem.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

concurrent_test: $(TESTS_DIR)/concurrent_tester.c $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_stealing_sched: $(TESTS_DIR)/test_stealing_sched.c $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

# Benchmarks

test_lifo_quicksort: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_lifo_quicksort_sem: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/lifo_sched_sem.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_stealing_quicksort: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_stealing_quicksort_cond: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched_cond.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

test_stealing_quicksort_sem: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched_sem.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

# Demos and dependencies

test_mandelbrot_lifo: $(DEMOS_DIR)/mandelbrot.c $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $(OUT_DIR)/$@

test_mandelbrot_stealing_cond: $(DEMOS_DIR)/mandelbrot.c $(BUILD_DIR)/stealing_sched_cond.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $(OUT_DIR)/$@

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
