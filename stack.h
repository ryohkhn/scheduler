#ifndef SCHEDULER_STACK_H
#define SCHEDULER_STACK_H

struct stack {
    struct node *top;
    int size;
};
struct node {
    void *f;
    struct node *next;
};

struct stack *stack_init();
int is_empty(struct stack *);
int size(struct stack *);
void push(void *f, struct stack *);
void *pop(struct stack *);

#endif //SCHEDULER_STACK_H
