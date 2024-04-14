#include "sched.h"
#include <stdlib.h>
#include <stdio.h>

void wake_up(void *closure, struct scheduler *s) {
    printf("HELLO WORLD\n");
}

void last_one(void *closure, struct scheduler *s) {
    printf("~~BYE WORLD~~\n");
}

// For testing with make file call 'make test'
int main(int argc, char *argv[]) {
    int nthreads = 1, qlen = 10;
    // For now, first argv is nthreads and second is qlen
    if (argc == 3) {
        nthreads = atoi(argv[1]);
        qlen = atoi(argv[2]);
    }
    if (sched_init(nthreads, qlen, wake_up, NULL) == -1) {
        perror("Failed to init scheduler");
        exit(1);
    }
    else
        printf("Properly finished with the work\n");

    return EXIT_SUCCESS;
}
