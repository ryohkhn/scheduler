#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>

#include "../include/stack.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
    pthread_t *threads;
    pthread_cond_t cond_var;

    sem_t sem;

    pthread_mutex_t mutex;
    struct stack *tasks;
};

int get_nbthreads(struct scheduler *s) {
    return s->nthreads;
}

void cleanup_sched(struct scheduler *sched) {
    free(sched->threads);
    free_up(sched->tasks);
    pthread_mutex_destroy(&sched->mutex);
    pthread_cond_destroy(&sched->cond_var);
    sem_destroy(&sched->sem);
}

void *slippy_time(void *args) {
    struct scheduler *s = (struct scheduler*) args;
    int val = -1;

    while (1) {
        pthread_mutex_lock(&s->mutex);

        while (is_empty(s->tasks)) {
            pthread_mutex_unlock(&s->mutex);

            sem_getvalue(&s->sem, &val);
            if (val == 0) {
                sem_post(&s->sem);
                return NULL;
            }
            sem_wait(&s->sem);
            pthread_mutex_lock(&s->mutex);
        }
        struct work w = pop(s->tasks);
        pthread_mutex_unlock(&s->mutex);

        taskfunc f = w.f;
        void *closure = w.closure;
        f(closure, s); // Going to work
    }
}

// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    if (nthreads <= 0 || nthreads > sched_default_threads())
        sched.nthreads = sched_default_threads();
    else
        sched.nthreads = nthreads;

    pthread_mutex_init(&sched.mutex, NULL);
    pthread_cond_init(&sched.cond_var, NULL);
    sem_init(&sched.sem, 1, sched.nthreads - 1);

    sched.tasks = stack_init();
    if (!sched.tasks) {
        fprintf(stderr, "Failed to initialize the stack\n");
        return -1;
    }

    sched.threads = malloc(sizeof(pthread_t) * sched.nthreads);
    if (!sched.threads) {
        fprintf(stderr, "Failed to malloc threads array\n");
        return -1;
    }
    sched.qlen = qlen;

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

    pthread_mutex_unlock(&s->mutex);
    sem_post(&s->sem);

    return 1;
}
