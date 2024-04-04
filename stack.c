#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

struct stack *stack_init() {
    struct stack *s = malloc(sizeof(struct stack));
    if (s != NULL) {
        s->size = 0;
        return s;
    } else {
        printf("Failed to malloc stack\n");
        return NULL;
    }
}
struct node *node_init(void *f, struct node *next) {
    struct node *n = malloc(sizeof(struct node));
    if (n != NULL) {
        n->f = f;
        n->next = next;
        return n;
    } else {
        printf("Failed to malloc node\n");
        return NULL;
    }
}
int is_empty(struct stack *s) {
    return s->top == NULL;
}
int size(struct stack *s) {
    return s->size;
}
void push(void *f, struct stack *s) {
    s->top = node_init(f, s->top);
    s->size++;
}
void *pop(struct stack *s) {
    if (is_empty(s)) {
        return NULL;
    } else {
        void *res = s->top->f;
        s->top = s->top->next;
        s->size--;
        return res;
    }
}
void *test(void *arg) {
    printf("IN TEST !\n");
    return arg;
}
void *last_test(void *arg) {
    printf("IN LAST TEST !\n");
    return arg;
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