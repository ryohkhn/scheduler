#ifndef SCHEDULER_DEQUE_H
#define SCHEDULER_DEQUE_H

#include "sched.h"

struct deque {
    struct node *top;
    struct node *bottom;
    int size;
};

struct node {
    struct work w;
    struct node *next;
    struct node *prev;
};

struct deque *deque_init();

int is_empty(struct deque *);
int size(struct deque *);

void push_top(struct work w, struct deque *);
void push_bottom(struct  work w, struct deque *);
struct work pop_top(struct deque *);
struct work pop_bottom(struct deque *);

void free_up(struct deque *);

#endif //SCHEDULER_DEQUE_H
