#!/usr/bin/env python3

import subprocess
import os
import time
import argparse

DEBUG = 0

nth_iterations = 2
max_threads = os.cpu_count()
output_index_time = 2
output_stats_steal_succeeded_id = 3
output_stats_steal_failed_id = 7
output_stats_tasks_done_id = 10


programs_names = [
    ("LIFO scheduler", "bench_lifo_quicksort"),
    ("LIFO scheduler with spinlock", "bench_lifo_quicksort_spin"),
    ("Work-stealing scheduler", "bench_stealing_quicksort"),
    ("Work-stealing scheduler with cond_var", "bench_stealing_quicksort_cond"),
    ("Work-stealing scheduler with 1 wait time var", "bench_stealing_quicksort_opt"),
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
    return result.stdout.decode().split()


def launch_bench(results, results_avg, stats_avg):
    if max_threads == 0:
        print(f"Executing benchmark in serial")
    elif max_threads == 1:
        print(f"Executing benchmark with {nth_iterations} iterations, with 1 thead")
    else:
        print(f"Executing benchmark with {nth_iterations} iterations, from 1 to {max_threads} threads")

    for count, pair in enumerate(programs_names):
        for i in range(max_threads + 1):
            program_bench_sum = 0.
            stats_steal_succeeded = 0
            stats_steal_failed = 0
            stats_tasks_done = 0
            for j in range(nth_iterations):
                # Launch program with each threads from 0 to max_threads
                if i == 0:
                    prog_args = "-s"
                else:
                    prog_args = "-t " + str(i)
                prog_out = launch_prog(pair, prog_args)
                # Handle parsing for timed results
                if (prog_out[0] == "Done"):
                    res = float(prog_out[output_index_time])
                    program_bench_sum += res
                    results[count][i][j] = res
                # Handle parsing when stats are available
                elif (prog_out[0] == "Steal"):
                    stats_steal_succeeded += int(prog_out[output_stats_steal_succeeded_id])
                    stats_steal_failed += int(prog_out[output_stats_steal_failed_id])
                    stats_tasks_done += int(prog_out[output_stats_tasks_done_id])
                    res = float(prog_out[1 + output_stats_tasks_done_id + output_index_time])
                    program_bench_sum += res
                    results[count][i][j] = res
                if DEBUG:
                    print(f"Done '{pair[0]}' with {i} threads at the {j}th iteration")
            average_time = round(program_bench_sum / nth_iterations, 6)
            stats_avg[count][i] = (int(stats_steal_succeeded / nth_iterations),
                                   int(stats_steal_failed / nth_iterations),
                                   int(stats_tasks_done / nth_iterations))
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

    plt.xlabel('Threads')
    plt.ylabel('Time in seconds')
    plt.title("Combined schedulers\n(average at " + str(nth_iterations + 1) + " iterations)")
    plt.legend()
    plt.savefig('combined_schedulers.png')
    plt.close()


def format_stats_string(stats):
    failed, succeed, total = stats
    return (f"Steal attempts succeeded: {failed}\n"
            f"Steal attempts failed:    {succeed}\n"
            f"Tasks completed:          {total}\0")

def generate_csv_data(results, results_avg, stats):
    import csv

    for i, sub_array in enumerate(results):
        name = programs_names[i][0].replace(' ', '_').replace('-', '_')
        with open(f'bench_{name}.csv', 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([programs_names[i][0] + " (average at " + str(nth_iterations) + " iteration(s))", ""])
            writer.writerow("")
            writer.writerow([""] * (nth_iterations + 2) + ["Average", "", "Statistics"] )
            writer.writerow("")
            for j, arr in enumerate(sub_array):
                if j == 0:
                    s = ["Serial"]
                elif j == 1:
                    s = ["1 Thread"]
                else:
                    s = [str(j) + " Threads"]
                stats_string = ""
                if (stats[i][j] != (0., 0., 0.)):
                    stats_string = format_stats_string(stats[i][j])
                writer.writerow(s + arr + [""] + [results_avg[i][j]] + ["", stats_string])
            writer.writerow("")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script to launch the schedulars benchmarks')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-t', type=int, help="Run with t threads")
    group.add_argument('-s', action="store_true", help="Run in serial (wont work with graphs)")
    parser.add_argument('-g', action="store_true", help="Generate image graphs")
    parser.add_argument('-i', type=int, help="Number of iterations for each benchmark")
    parser.add_argument('-D', action="store_true", help="Debug mode")
    args = parser.parse_args()

    if args.t != None:
        if args.t < max_threads and args.t >= 0:
            max_threads = args.t
    elif args.s:
        max_threads = 0
    if args.i:
        nth_iterations = args.i
    if args.D:
        DEBUG = 1
    compile_files()
    res = [[[None for _ in range(nth_iterations)] for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
    res_avg = [[None for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
    stats = [[[None for _ in range(nth_iterations)] for _ in range(max_threads + 1)] for _ in range(len(programs_names))]
    launch_bench(res, res_avg, stats)
    print("Results : " + str(res))
    print("Averages results : " + str(res_avg))
    print("Statistics: " + str(stats))
    if args.g:
        generate_images(res, res_avg)
    generate_csv_data(res, res_avg, stats)
