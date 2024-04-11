#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void test(void *closure, struct scheduler *s) {
    printf("IN TEST !\n");
}
void last_test(void *closure, struct scheduler *s) {
    printf("IN LAST TEST !\n");
}

// For testing with make file call 'make test'
int main() {
    struct stack *st = stack_init();
    if (st == NULL) {
        perror("Failed stack initialization");
        exit(1);
    }

    push(last_test, st);
    push(test, st);
    push(test, st);
    printf("Size of stack after 3 push: %d\n", size(st));
    while (!is_empty(st)) {
        void *method = pop(st);
        ((void(*)())method)();
    }
    printf("Size of stack after 3 pop: %d\n", size(st));
    return EXIT_SUCCESS;
}
