#include "deque.h"
#include <stdlib.h>
#include <stdio.h>

struct deque *deque_init() {
    struct deque *d = malloc(sizeof(struct deque));
    if (d != NULL) {
        d->size = 0;
        d->top = NULL;
        d->bottom = NULL;
        return d;
    } else {
        printf("Failed to malloc deque\n");
        return NULL;
    }
}

struct node * node_init(struct work w, struct node *next, struct node *prev) {
    struct node *n = malloc(sizeof(struct node));
    if (n != NULL) {
        n->w = w;
        n->next = next;
        n->prev = prev;
        return n;
    } else {
        printf("Failed to malloc node in deque\n");
        return NULL;
    }
}

int is_empty(struct deque *d) {
    return d->top == NULL || d->bottom == NULL;
}
int size(struct deque *d) {
    return d->size;
}

void push_top(struct work w, struct deque *d) {
    struct node *n = node_init(w, d->top, NULL);
    if (n == NULL) {
        return;
    } else {
        if (d->top == NULL || d->bottom == NULL) {
            d->top = n;
            d->bottom = n;
        } else {
            d->top->prev = n;
            d->top = n;
        }
        d->size++;
    }
}
void push_bottom(struct  work w, struct deque *d) {
    struct node *n = node_init(w, NULL, d->bottom);
    if (n == NULL) {
        return;
    } else {
        if (d->bottom == NULL || d->top == NULL) {
            d->bottom = n;
            d->top = n;
        } else {
            d->bottom->next = n;
            d->bottom = n;
        }
        d->size++;
    }
}
struct work pop_top(struct deque *d) {
    if (is_empty(d)) {
        struct work w = {0};
        return w;
    }

    struct node *n = d->top;
    struct work w = n->w;

    d->top = n->next;
    if (d->top != NULL) {
        d->top->prev = NULL;
    }
    d->size--;
    free(n);
    return w;
}
struct work pop_bottom(struct deque *d) {
    if (is_empty(d)) {
        struct work w = {0};
        return w;
    }

    struct node *n = d->bottom;
    struct work w = n->w;

    d->bottom = n->prev;
    if (d->bottom != NULL) {
        d->bottom->next = NULL;
    }
    d->size--;
    free(n);
    return w;
}

void free_up(struct deque *d) {
    while (!is_empty(d)) {
        pop_top(d);
    }
    free(d);
}

