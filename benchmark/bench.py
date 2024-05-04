#!/usr/bin/env python3

import subprocess
import os
import time
import argparse

nth_iterations = 2
max_threads = 8
output_index_time = 2
num_processors = os.cpu_count()
output_csv_filename = "benchmark_results.csv"


programs_names = [
    ("LIFO scheduler", "bench_lifo_quicksort"),
    ("Work-stealing scheduler", "bench_stealing_quicksort"),
    ("Work-stealing scheduler with cond_var", "bench_stealing_quicksort_cond"),
    # ("Work-stealing scheduler with 1 wait time var", "bench_stealing_quicksort_opt"),
    ("Work-stealing scheduler with multiple time var", "bench_stealing_quicksort_opt_multiple")
]


def compile_files():
    og_dir = os.getcwd()
    os.chdir("..")
    print("Compiling files...")
    subprocess.run(["/usr/bin/make", "clean"], stdout=subprocess.DEVNULL)
    cmd = ["/usr/bin/make"] + [program[1] for program in programs_names]
    subprocess.run(cmd, stdout=subprocess.DEVNULL)
    os.chdir(og_dir)


def launch_prog(pair, prog_args):
    cmd = "../out/" + pair[1]
    result = subprocess.run([cmd, prog_args], stdout=subprocess.PIPE)
    s = result.stdout.decode().split()
    return float(s[output_index_time])


def launch_bench(results, results_avg):
    if max_threads == 0:
        print(f"Executing benchmark in serial")
    elif max_threads == 1:
        print(f"Executing benchmark with {nth_iterations} iterations, with 1 thead")
    else:
        print(f"Executing benchmark with {nth_iterations} iterations, from 1 to {max_threads} threads")

    for count, pair in enumerate(programs_names):
        # Launch serial reference
        for i in range(max_threads + 1):
            program_bench_sum = 0.
            for j in range(nth_iterations):
                # Launch program with each threads from 0 to max_threads
                if i == 0:
                    prog_args = "-s"
                else:
                    prog_args = "-t " + str(i)
                done_time = launch_prog(pair, prog_args)
                program_bench_sum += done_time
                results[count][i][j] = done_time
                time.sleep(0.2)
            average_time = round(program_bench_sum / nth_iterations, 6)
            results_avg[count][i] = average_time


def generate_images(results, results_avg):
    import matplotlib.pyplot as plt
    import numpy as np
    min_x_axis = 0
    max_x_axis = max_threads + 1
    min_y_axis = 0
    max_value = max(max(sublist) for sublist in results)
    max_y_axis = max_value[0] + 0.5

    for i, values in enumerate(results_avg):
        program_name = programs_names[i][1].replace(" ", "_")

        plt.plot(values)
        plt.xlabel('Threads')
        plt.ylabel('Time in seconds')
        plt.title(programs_names[i][0] + "\n(average at " + str(nth_iterations) + " iterations)")

        plt.ylim(min_y_axis, max_y_axis)
        plt.xlim(min_x_axis, max_x_axis)

        x_values = np.arange(min_x_axis, max_x_axis)
        tick_labels = ['(serial)' if x == 0 else str(x) for x in x_values]
        plt.xticks(x_values, tick_labels)

        plt.savefig(f'{program_name}_plot.png')
        plt.close()

    for i, avg in enumerate(results_avg):
        plt.plot(avg, label=programs_names[i][0], linewidth=2)

    plt.legend()
    plt.savefig('combined_schedulers.png')
    plt.close()


def generate_csv_data(results, results_avg):
    import csv

    for i, sub_array in enumerate(results):
        name = programs_names[i][0].replace(' ', '_').replace('-', '_')
        with open(f'bench_{name}.csv', 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([programs_names[i][0] + " (average at " + str(nth_iterations) + " iteration(s))", ""])
            writer.writerow("")
            writer.writerow([""] * (nth_iterations + 2) + ["Average"])
            writer.writerow("")
            for j, arr in enumerate(sub_array):
                if j == 0:
                    s = ["Serial"]
                elif j == 1:
                    s = ["1 Thread"]
                else:
                    s = [str(j) + " Threads"]
                writer.writerow(s + arr + [""] + [results_avg[i][j]])
            writer.writerow("")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to launch the schedulars benchmarks')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-t', type=int, help="Run with t threads")
    group.add_argument('-s', action="store_true", help="Run in serial (wont work with graphs)")
    parser.add_argument('-g', action="store_true", help="Generate image graphs")
    parser.add_argument('-i', type=int, help="Number of iterations for each benchmark")
    args = parser.parse_args()

    if args.t:
        max_threads = args.t
        if max_threads > num_processors:
            max_threads = num_processors
    elif args.s:
        max_threads = 0
    else:
        max_threads = num_processors
    if args.i:
        nth_iterations = args.i
    compile_files()
    res = [[[None for _ in range(nth_iterations)] for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
    res_avg = [[None for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
    launch_bench(res, res_avg)
    print("Results : " + str(res))
    print("Averages results : " + str(res_avg))
    if args.g:
        generate_images(res, res_avg)
    generate_csv_data(res, res_avg)
