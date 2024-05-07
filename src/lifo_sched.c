/*  LIFO scheduler

    The work is implemented as a stack.
    A mutex is used to protect the stack and the integer that counts the number
    of threads sleeping.
    A condition variable allows the scheduler to sleep when the stack is empty
    and to be woken when a thread adds new work */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "../include/stack.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
    pthread_t *threads;

    int nth_sleeping_threads;

    pthread_mutex_t mutex;
    pthread_cond_t cond_var;
    struct stack *tasks;
};

int get_nbthreads(struct scheduler *s) {
    return s->nthreads;
}

void cleanup_sched(struct scheduler *sched) {
    free(sched->threads);
    pthread_mutex_destroy(&sched->mutex);
    pthread_cond_destroy(&sched->cond_var);
    free(sched->tasks);
}

void *slippy_time(void *args) {
    struct scheduler *sched = (struct scheduler*) args;

    while (1) {
        pthread_mutex_lock(&sched->mutex); // Lock taken to check if there is work to do

        while (is_empty(sched->tasks)) {
            sched->nth_sleeping_threads++;
            if (sched->nth_sleeping_threads >= sched->nthreads) {
                // We inform every thread waiting that no more work is available
                pthread_cond_broadcast(&sched->cond_var);
                pthread_mutex_unlock(&sched->mutex);
                return NULL; // No threads are working and there are no tasks left = end of threads/scheduler
            }
            pthread_cond_wait(&sched->cond_var, &sched->mutex);
            sched->nth_sleeping_threads--;
        }
        struct work w = pop(sched->tasks);
        pthread_mutex_unlock(&sched->mutex); // Lock is given back with nb_threads_working incremented & work taken from stack

        taskfunc f = w.f;
        void *closure = w.closure;
        f(closure, sched); // Going to work
    }
}

// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    pthread_mutex_init(&sched.mutex, NULL);
    pthread_cond_init(&sched.cond_var, NULL);

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
    pthread_mutex_lock(&s->mutex);
    if (size(s->tasks) >= s->qlen) {
        cleanup_sched(s);
        errno = EAGAIN;
        perror("The task amount is superior than what the scheduler can handle");
        return -1;
    }
    struct work w = {closure, f};
    push(w, s->tasks);

    pthread_cond_signal(&s->cond_var);
    pthread_mutex_unlock(&s->mutex);

    return 1;
}
