#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void test(void *closure, struct scheduler *s) {
    printf("IN TEST !\n");
}
void last_test(void *closure, struct scheduler *s) {
    printf("IN LAST TEST !\n");
}

struct work new_work(void *closure, taskfunc f) {
    struct work w = {closure, f};
    return w;
}

// For testing with make file call 'make test'
int main() {
    struct stack *st = stack_init();
    if (st == NULL) {
        perror("Failed stack initialization");
        exit(1);
    }

    push(new_work(NULL, last_test), st);
    push(new_work(NULL, test), st);
    push(new_work(NULL, test), st);
    printf("Size of stack after 3 push: %d\n", size(st));
    while (!is_empty(st)) {
        struct work w = pop(st);
        ((void(*)())w.f)(w.closure);
    }
    printf("Size of stack after 3 pop: %d\n", size(st));
    return EXIT_SUCCESS;
}
