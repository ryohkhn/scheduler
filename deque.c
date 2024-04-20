#include "deque.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

struct deque *deque_init() {
    struct deque *d = malloc(sizeof(struct deque));

    if (d != NULL)
        bzero(d, sizeof(struct deque));
    else
        printf("Failed to malloc deque\n");
    return d;
}

struct node * node_init(struct work w, struct node *next, struct node *prev) {
    struct node *n = malloc(sizeof(struct node));

    if (n != NULL) {
        n->w = w;
        n->next = next;
        n->prev = prev;
    }
    else {
        printf("Failed to malloc node in deque\n");
    }
    return n;
}

int is_empty(struct deque *d) {
    return d->top == NULL || d->bottom == NULL;
}

int size(struct deque *d) {
    return d->size;
}

void push_top(struct work w, struct deque *d) {
    struct node *n = node_init(w, d->top, NULL);

    if (n == NULL)
        return;
    else {
        if (is_empty(d)) {
            d->top = n;
            d->bottom = n;
        }
        else {
            d->top->prev = n;
            d->top = n;
        }
        d->size++;
    }
}

void push_bottom(struct  work w, struct deque *d) {
    struct node *n = node_init(w, NULL, d->bottom);

    if (n == NULL)
        return;
    else {
        if (is_empty(d)) {
            d->top = n;
            d->bottom = n;
        }
        else {
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

    d->top = d->top->next;
    if (d->top != NULL) {
        d->top->prev = NULL;
    }
    d->size--;
    if (d->size == 0)
        d->bottom = NULL;
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

    d->bottom = d->bottom->prev;
    if (d->bottom != NULL) {
        d->bottom->next = NULL;
    }
    d->size--;
    if (d->size == 0)
        d->top = NULL;
    free(n);
    return w;
}

void free_up(struct deque *d) {
    while (!is_empty(d)) {
        pop_top(d);
    }
    free(d);
}

