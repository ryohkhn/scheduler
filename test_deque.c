#include "deque.h"
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
    struct deque *dq = deque_init();
    if (dq == NULL) {
        perror("Failed stack initialization");
        exit(1);
    }

    push_top(new_work(NULL, last_test), dq);
    push_top(new_work(NULL, test), dq);
    push_top(new_work(NULL, test), dq);
    printf("Size of stack after 3 push_top: %d\n", size(dq));
    printf("Popping from bottom:\n");
    while (!is_empty(dq)) {
        struct work w = pop_bottom(dq);
        ((void(*)())w.f)(w.closure);
    }
    printf("Size of stack after 3 pop_botom: %d\n", size(dq));
    push_top(new_work(NULL, last_test), dq);
    push_top(new_work(NULL, test), dq);
    push_top(new_work(NULL, test), dq);
    printf("Pushed 3 new struct work from top, size: %d\n", size(dq));
    while (!is_empty(dq)) {
        struct work w = pop_top(dq);
        ((void(*)())w.f)(w.closure);
    }
    printf("Just finished popping them from top this time\n");



    return EXIT_SUCCESS;
}
