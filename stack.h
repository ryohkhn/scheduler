#ifndef SCHEDULER_STACK_H
#define SCHEDULER_STACK_H

#include "sched.h"

struct work {
    void *closure;
    taskfunc f;
};

struct stack {
    struct node *top;
    int size;
};

struct node {
    struct work w;
    struct node *next;
};

struct stack *stack_init();
int is_empty(struct stack *);
int size(struct stack *);
void push(struct work w, struct stack *);
struct work pop(struct stack *);

#endif //SCHEDULER_STACK_H
