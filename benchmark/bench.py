#!/usr/bin/env python3

import subprocess
import os

program_name = "test_dumb_quicksort"
nth_execution = 10

os.chdir("..")

cmd = ["/usr/bin/make", program_name]
subprocess.run(cmd, shell = True)

with open("results.txt", "w") as f:
    for _ in range(nth_execution):
        cmd = "./test/" + program_name
        result = subprocess.run([cmd], stdout=subprocess.PIPE)
        f.write(result.stdout.decode())
