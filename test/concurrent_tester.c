#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "../include/sched.h"

struct test_args {
    int n;
};

struct test_args *new_args(int n) {
    struct test_args *args = malloc(sizeof(struct test_args));
    if(args == NULL)
        return NULL;

    args->n = n + 1;
    return args;
}

void test(void *closure, struct scheduler *s) {
    struct test_args *args = (struct test_args *) closure;
    if (args->n < 10) {
        printf("Spawning another because n = %d\n", args->n);
        int rc = sched_spawn(test, new_args(args->n), s);
        assert(rc >= 0);
    }
    return;
}

int main() {
    int rc = sched_init(2, 100, test, new_args(0));
    assert(rc >= 0);

    return EXIT_SUCCESS;
}
