#include "sched.h"
#include <stdlib.h>
#include <stdio.h>

void wake_up(void *closure, struct scheduler *s) {
    printf("HELLO WORLD\n");
}

// For testing with make file call 'make test'
int main() {
    if (sched_init(1, 10, wake_up, NULL) == -1) {
        perror("Failed to init scheduler");
        exit(1);
    } else {
        printf("Properly finished with the work\n");
    }
    return EXIT_SUCCESS;
}