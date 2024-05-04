/*  LIFO scheduler

    The work is implemented as a stack.
    A spinlock is used to protect the stack and the integer that counts the
    number of threads sleeping. */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdatomic.h>
#include <sched.h>

#include "../include/stack.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
    pthread_t *threads;

    int nth_sleeping_threads;
    atomic_int spinlock;

    struct stack *tasks;
};

int get_nbthreads(struct scheduler *s) {
    return s->nthreads;
}

void cleanup_sched(struct scheduler *sched) {
    free(sched->threads);
    free(sched->tasks);
}

void lock(struct scheduler *sched) {
    while (1) {
        int rc, expected = 0;

        rc = atomic_compare_exchange_weak(&sched->spinlock, &expected, 1);
        if (rc)
            break;
        sched_yield();
    }
}

void unlock(struct scheduler *sched) {
    atomic_store(&sched->spinlock, 0);
}

void *slippy_time(void *args) {
    struct scheduler *sched = (struct scheduler*) args;

    while (1) {
        lock(sched); // Lock taken to check if there is work to do

        while (is_empty(sched->tasks)) {
            sched->nth_sleeping_threads++;
            if (sched->nth_sleeping_threads >= sched->nthreads) {
                unlock(sched);
                return NULL; // No threads are working and there are no tasks left = end of threads/scheduler
                // Other threads are working so tasks might get added, we go to sleep until we get spawned
            }
            unlock(sched);
            usleep(2000);
            lock(sched);
            sched->nth_sleeping_threads--;
        }

        struct work w = pop(sched->tasks);
        taskfunc f = w.f;
        void *closure = w.closure;

        unlock(sched);

        f(closure, sched); // Going to work
    }
}

// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    sched.tasks = stack_init();
    if (!sched.tasks) {
        fprintf(stderr, "Failed to initialize the stack\n");
        return -1;
    }

    if (nthreads <= 0 || nthreads > sched_default_threads())
        sched.nthreads = sched_default_threads();
    else
        sched.nthreads = nthreads;

    sched.threads = malloc(sizeof(pthread_t) * sched.nthreads);
    if (!sched.threads) {
        fprintf(stderr, "Failed to malloc threads array\n");
        return -1;
    }
    sched.qlen = qlen;
    sched.nth_sleeping_threads = 0;
    unlock(&sched);

    for (int i = 0; i < sched.nthreads; i++) {
        if (pthread_create(&sched.threads[i], NULL,slippy_time, &sched) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            return -1;
        }
    }

    if (!sched_spawn(f, closure, &sched)) {
        fprintf(stderr, "Failed to create the inital task\n");
        return -1;
    }

    void *arg = NULL;
    for (int i = 0; i < sched.nthreads; i++) {
        if (pthread_join(sched.threads[i], arg) != 0) {
            fprintf(stderr, "Failed to join thread\n");
            return -1;
        }
    }

    cleanup_sched(&sched);
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    lock(s);
    if (size(s->tasks) >= s->qlen) {
        cleanup_sched(s);
        errno = EAGAIN;
        perror("The task amount is superior than what the scheduler can handle");
        return -1;
    }
    struct work w = {closure, f};
    push(w, s->tasks);

    unlock(s);

    return 1;
}
