#!/usr/bin/env python3

import subprocess
import os

nth_iterations = 10

program_name = [("LIFO scheduler","test_dumb_quicksort"),
                ("LIFO scheduler with semaphore", "test_dumb_quicksort_sem"),
                ("Work-stealing scheduler", "test_stealing_quicksort"),
                ("Work-stealing scheduler with semaphore", "test_stealing_quicksort_sem")]


os.chdir("..")

for s in program_name:
    cmd = ["/usr/bin/make", s[1]]
    subprocess.run(cmd, shell = True)

# TODO Get parent dir and not hardcode
os.chdir("benchmark")

with open("results.txt", "w") as f:
    for pair in program_name:
        f.write(pair[0] + "\n")
        for _ in range(nth_iterations):
            cmd = "../out/" + pair[1]
            result = subprocess.run([cmd], stdout=subprocess.PIPE)
            f.write(result.stdout.decode())
