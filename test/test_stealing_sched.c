#include <stdio.h>
#include <stdlib.h>

#include "../include/sched.h"

void wake_up(void *closure, struct scheduler *s) {
    printf("HELLO WORLD\n");
}

int main(int argc, char *argv[]) {
    int nthreads = 16, qlen = 10;

    if (argc == 3) {
        nthreads = atoi(argv[1]);
        qlen = atoi(argv[2]);
    }
    if (sched_init(nthreads, qlen, wake_up, NULL) == -1) {
        perror("Failed to init sched");
        exit(1);
    }
    else
        printf("Finished with single task");

    return EXIT_SUCCESS;
}
