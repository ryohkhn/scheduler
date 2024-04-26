#!/usr/bin/env python3

import subprocess
import os
import psutil
import time

nth_iterations = 4
max_threads = 6
output_index_time = 2
num_processors = os.cpu_count()


programs_names = [
    #("LIFO scheduler","test_lifo_quicksort"),
    #("LIFO scheduler with semaphore and synchronization variable", "test_lifo_quicksort_sem"),
    #("Work-stealing scheduler", "test_stealing_quicksort"),
    #("Work-stealing scheduler with synchronization variable", "test_stealing_quicksort_cond"),
    #("Work-stealing scheduler with semaphore and synchronization variable", "test_stealing_quicksort_sem")
    ("Work-stealing scheduler", "test_stealing_quicksort"),
    ("Work-stealing scheduler with synchronization variable", "test_stealing_quicksort_cond"),
    ("Work-stealing scheduler with 1 wait time var", "test_stealing_quicksort_opt"),
    ("Work-stealing scheduler with one time var for each thread", "test_stealing_quicksort_opt_multiple")
]

results = [[[None for _ in range(nth_iterations)] for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
results_avg = [[None] * (max_threads + 1) for _ in range(len(programs_names))]


def compile_files():
    og_dir = os.getcwd()
    os.chdir("..")
    print("Compiling files...")
    subprocess.run(["/usr/bin/make", "clean"], stdout=subprocess.DEVNULL)
    cmd = ["/usr/bin/make"] + [program[1] for program in programs_names]
    subprocess.run(cmd, stdout=subprocess.DEVNULL)
    os.chdir(og_dir)


def launch_prog(pair, args):
    cmd = "../out/" + pair[1]
    result = subprocess.run([cmd, args], stdout=subprocess.PIPE)
    s = result.stdout.decode().split()
    res = float(s[output_index_time])
    # f.write(result.stdout.decode())
    return res


def launch_bench():
    print(f"Executing benchmark with {nth_iterations} iterations, from 1 to {max_threads} threads")
    for count, pair in enumerate(programs_names):
        # f.write(pair[0] + "\n")

        # Launch serial reference
        # f.write("Serial time reference:\n")
        args = "-s"
        serial_res = launch_prog(pair, args)
        results_avg[count][0] = serial_res
        results[count][0][0] = serial_res

        for i in range(1, max_threads + 1):
            program_bench_sum = 0.
            for j in range(nth_iterations):
                # Launch program with each threads from 1 to max_threads
                args = "-t " + str(i)
                res = launch_prog(pair, args)
                program_bench_sum += res
                results[count][i][j] = res
                time.sleep(0.3)
            average_time = round(program_bench_sum / nth_iterations, 6)
            results_avg[count][i] = average_time
            # f.write("Average time with " + str(i) + " threads: " + str(average_time) + "\n")
        # f.write("\n")


min_x_axis = 0
max_x_axis = max_threads + 1
min_y_axis = 0
max_y_axis = 1.5


def generate_images():
    import matplotlib.pyplot as plt
    import numpy as np

    for i, values in enumerate(results_avg):
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


output_csv_filename = "benchmark_results.csv"


def generate_csv_data():
    import csv

    for i, sub_array in enumerate(results):
        name = programs_names[i][0].replace(' ', '_').replace('-', '_')
        with open(f'bench_{name}.csv', 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([programs_names[i][0], ""])
            writer.writerow(["Threads", "Time in seconds", ""])
            writer.writerow(["", ""])
            for j in range(1, max_threads + 1):
                if j-1 < len(sub_array):
                    writer.writerow([j] + sub_array)
                else:
                    writer.writerow([j, ""])


if __name__ == "__main__":
    if max_threads > num_processors:
        max_threads = num_processors
    compile_files()
    launch_bench()
    print(results)
    # generate_images()
    # generate_csv_data()
