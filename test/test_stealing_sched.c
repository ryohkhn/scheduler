#include <stdio.h>
#include <stdlib.h>

#include "../include/sched.h"

void last_work(void *, struct scheduler *) {
    printf("Travail terminéééé\n");
}

struct testArg {
    int n;
};

void stacking_up(void *closure, struct scheduler *s) {
    struct testArg *ar = (struct testArg *) closure;
    int n = ar->n;
    printf("Stacking up, n: %d!\n", n);
    if (n < 150000) {
        struct testArg *new_ar = malloc(sizeof (struct testArg));
        new_ar->n = n + 1;
        sched_spawn(stacking_up, new_ar, s);
    } else {
        sched_spawn(last_work, NULL, s);
    }
}

int main(int argc, char *argv[]) {
    int nthreads = 16, qlen = 10;

    if (argc == 3) {
        nthreads = atoi(argv[1]);
        qlen = atoi(argv[2]);
    }
    struct testArg *ar = malloc(sizeof (struct testArg));
    ar->n = 0;
    if (sched_init(nthreads, qlen, stacking_up, ar) == -1) {
        perror("Failed to init sched");
        exit(1);
    } else {
        printf("Finished with single task\n");
    }

    return EXIT_SUCCESS;
}
