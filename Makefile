CC = gcc

CFLAGS = -Wall -Wextra -I$(INCLUDE) $(FAST)

FAST = -O2

INCLUDE = include

SRCS_DIR = src

BUILD_DIR = build

TESTS_DIR = test

DEMOS_DIR = demo

OUT_DIR = out

RAYLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

BENCHS = bench_lifo_quicksort bench_lifo_quicksort_spin bench_lifo_quicksort_sem bench_stealing_quicksort bench_stealing_quicksort_cond bench_stealing_quicksort_opt bench_stealing_quicksort_opt_multiple

DEMOS =  demo_mandelbrot_lifo demo_mandelbrot_stealing demo_voronoi_lifo demo_voronoi_stealing demo_mandelbrot_lifo_spin demo_static_voronoi_stealing demo_voronoi_viewer

BENCHMARK_FILE = quicksort.c

all: bench demo

# General build rules

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Specific rules for files with dependencies

$(BUILD_DIR)/lifo_sched.o: $(SRCS_DIR)/lifo_sched.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lifo_sched_sem.o: $(SRCS_DIR)/lifo_sched_sem.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lifo_sched_spin.o: $(SRCS_DIR)/lifo_sched_spin.c $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched.o: $(SRCS_DIR)/stealing_sched.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched_cond.o: $(SRCS_DIR)/stealing_sched_cond.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched_opt.o: $(SRCS_DIR)/stealing_sched_opt.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/stealing_sched_opt_multiple.o: $(SRCS_DIR)/stealing_sched_opt_multiple.c $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) -c $< -o $@

# Benchmarks

bench_lifo_quicksort: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_lifo_quicksort_sem: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/lifo_sched_sem.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_lifo_quicksort_spin: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/lifo_sched_spin.o $(BUILD_DIR)/stack.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_stealing_quicksort: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_stealing_quicksort_cond: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched_cond.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_stealing_quicksort_opt: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched_opt.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

bench_stealing_quicksort_opt_multiple: $(TESTS_DIR)/$(BENCHMARK_FILE) $(BUILD_DIR)/stealing_sched_opt_multiple.o $(BUILD_DIR)/deque.o
	$(CC) $(CFLAGS) $^ -o $(OUT_DIR)/$@

# Demos and dependencies

$(BUILD_DIR)/mandelbrot.o: $(DEMOS_DIR)/mandelbrot.c
	$(CC) -Wall $(FAST) -ffast-math `pkg-config --cflags gtk4` -c -o $(BUILD_DIR)/mandelbrot.o $(DEMOS_DIR)/mandelbrot.c

$(BUILD_DIR)/voronoi.o: $(DEMOS_DIR)/voronoi.c
	$(CC) -Wall $(FAST) $(RAYLIBS) -c -o $(BUILD_DIR)/voronoi.o $(DEMOS_DIR)/voronoi.c

$(BUILD_DIR)/static_voronoi.o: $(DEMOS_DIR)/static_voronoi.c
	$(CC) -Wall $(FAST) -c -o $(BUILD_DIR)/static_voronoi.o $(DEMOS_DIR)/static_voronoi.c

demo_mandelbrot_lifo: $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o `pkg-config --libs gtk4` -o $(OUT_DIR)/demo_mandelbrot_lifo

demo_mandelbrot_lifo_spin: $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/lifo_sched_spin.o $(BUILD_DIR)/stack.o
	$(CC) $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/lifo_sched_spin.o $(BUILD_DIR)/stack.o `pkg-config --libs gtk4` -o $(OUT_DIR)/demo_mandelbrot_lifo_spin

demo_mandelbrot_stealing: $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $(BUILD_DIR)/mandelbrot.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o `pkg-config --libs gtk4` -o $(OUT_DIR)/demo_mandelbrot_stealing

#demo_voronoi_lifo: $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
#	$(CC) $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o $(RAYLIBS) -o $(OUT_DIR)/demo_voronoi_lifo

#demo_voronoi_stealing: $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
#	$(CC) $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o $(RAYLIBS) -o $(OUT_DIR)/demo_voronoi_stealing

demo_voronoi_lifo: $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/lifo_sched.o $(BUILD_DIR)/stack.o
	$(CC) $^ $(RAYLIBS) -o $(OUT_DIR)/$@

demo_voronoi_stealing: $(BUILD_DIR)/voronoi.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $^ $(RAYLIBS) -o $(OUT_DIR)/$@

demo_static_voronoi_stealing: $(BUILD_DIR)/static_voronoi.o $(BUILD_DIR)/stealing_sched.o $(BUILD_DIR)/deque.o
	$(CC) $^ -lm -o $(OUT_DIR)/$@

demo_voronoi_viewer: VoronoiViewer.java
	javac VoronoiViewer.java

bench: $(BENCHS)

demo: $(DEMOS)

#rm -f $(addprefix $(TESTS_DIR)/, $(TESTS))
clean:
ifneq ("$(wildcard $(BUILD_DIR)/*.o)", "")
	rm $(BUILD_DIR)/*.o
endif
ifneq ("$(wildcard $(OUT_DIR)/*)", "")
	rm $(OUT_DIR)/*
endif

.PHONY:	all test clean bench demo
