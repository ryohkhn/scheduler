#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/stack.h"

struct stack *stack_init() {
    struct stack *s = malloc(sizeof(struct stack));
    if (!s) {
        fprintf(stderr, "Failed to malloc stack\n");
        return NULL;
    }
    memset(s, 0, sizeof(struct stack));
    return s;
}

struct node *node_init(struct work w, struct node *next) {
    struct node *n = malloc(sizeof(struct node));
    if (!n) {
        fprintf(stderr, "Failed to malloc node in stack\n");
        return NULL;
    }
    n->w = w;
    n->next = next;
    return n;
}

int is_empty(struct stack *s) {
    return s->top == NULL;
}

int size(struct stack *s) {
    return s->size;
}

void push(struct work w, struct stack *s) {
    struct node *n = node_init(w, s->top);
    if (n == NULL) {
        return;
    }
    s->top = n;
    s->size++;
}

struct work pop(struct stack *s) {
    if (is_empty(s)) {
        struct work w = {0};
        return w;
    }

    struct node *n = s->top;
    struct work w = n->w;
    s->top = s->top->next;
    s->size--;
    free(n);
    return w;
}

void free_up(struct stack *s) {
    while (!is_empty(s)) {
        pop(s);
    }
    free(s);
}
