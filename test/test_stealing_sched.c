#include <stdio.h>
#include <stdlib.h>

#include "../include/sched.h"

void wake_up(void *closure, struct scheduler *s) {
    printf("HELLO WORLD\n");
}

int main(void) {
    int nthreads = 1, qlen = 10;

    if (sched_init(nthreads, qlen, wake_up, NULL) == -1) {
        perror("Failed to init sched");
        exit(1);
    }
    else
        printf("Finished with single task");

    return EXIT_SUCCESS;
}