#!/usr/bin/env python3

import subprocess
import os
import psutil
import time

nth_iterations = 5
max_threads = 16
og_dir = os.getcwd()
output_index_time = 2

num_processors = os.cpu_count()
print("Number of online processors (CPU cores):", num_processors)

programs_names = [
    ("LIFO scheduler","test_dumb_quicksort"),
    ("LIFO scheduler with semaphore and synchronisation variable", "test_dumb_quicksort_sem"),
    ("Work-stealing scheduler", "test_stealing_quicksort"),
    ("Work-stealing scheduler with semaphore and synchronisation variable", "test_stealing_quicksort_sem")
]

os.chdir("..")

print("Compiling files...")

compile_progs_args = ' '.join(program[1] for program in programs_names)

cmd = ["/usr/bin/make", compile_progs_args]
subprocess.run(cmd, shell = True, stdout=subprocess.DEVNULL)

os.chdir(og_dir)

results = [[None] * (max_threads + 1) for _ in range(len(programs_names))]

def launch_prog(pair, args):
    cmd = "../out/" + pair[1]
    result = subprocess.run([cmd, args], stdout=subprocess.PIPE)
    s = result.stdout.decode().split()
    res = float(s[output_index_time])
    f.write(result.stdout.decode())
    return res


with open("results.txt", "w") as f:
    print(f"Executing benchmark with {nth_iterations} iterations, from 1 to {max_threads} threads")
    for count, pair in enumerate(programs_names):
        f.write(pair[0] + "\n")

        # Launch serial reference
        f.write("Serial time reference:\n")
        args = "-s"
        serial_res = launch_prog(pair, args)
        results[count][0] = serial_res

        for i in range(1, max_threads + 1):
            program_bench_sum = 0.
            for _ in range(nth_iterations):
                # Launch program with each threads from 1 to max_threads
                args = "-t " + str(i)
                res = launch_prog(pair, args)
                program_bench_sum += res
                time.sleep(0.3)

            average_time = round(program_bench_sum / nth_iterations, 6)
            results[count][i] = average_time
            f.write("Average time with " + str(i) + " threads: " + str(average_time) + "\n")
        f.write("\n")


import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl

min_x_axis = 0
max_x_axis = max_threads + 1
min_y_axis = 0
max_y_axis = 1.5

print(results)

for i, values in enumerate(results):
    program_name = programs_names[i][1].replace(" ", "_")

    plt.plot(values)
    plt.xlabel('Threads')
    plt.ylabel('Time in seconds')
    plt.title(programs_names[i][0] + "\n(average at " + str(nth_iterations) + " iterations)")

    plt.ylim(min_y_axis, max_y_axis)
    plt.xlim(min_x_axis, max_x_axis)

    # Replace the 0 tick label with "(serial)"
    x_values = np.arange(min_x_axis, max_x_axis)  # Adjusted to start from 0
    tick_labels = ['(serial)' if x == 0 else str(x) for x in x_values]
    plt.xticks(x_values, tick_labels)

    # Save the plot to a file (e.g., PNG, PDF, SVG)
    plt.savefig(f'{program_name}_plot.png')
    plt.close()
