#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "sched.h"
#include "stack.h"

struct scheduler {
    struct stack *tasks;
    int qlen; // Maximum number of tasks
    int nthreads;
    int nb_threads_working;
    pthread_mutex_t mutex;
    pthread_cond_t cond_var;
    pthread_t *threads;
};

struct args_pack {
    void *closure;
    struct scheduler *s;
};


void slippy_time(void *args) {
    struct scheduler *s = ((struct args_pack *) args)->s;

    pthread_mutex_lock(&s->mutex); // Lock taken to check if there is work to do
    while (is_empty(s->tasks)) {
        if (s->nb_threads_working > 0) {
            // Other threads are working so tasks might get added, we go to sleep until we get spawned
            // pthread_cond_wait will unlock the mutex until a signal is catched, then relock it for the thread that received the signal
            pthread_cond_wait(&s->cond_var, &s->mutex);
        }
        else {
            pthread_cond_broadcast(&s->cond_var);
            pthread_mutex_unlock(&s->mutex);
            return; // No threads are working and there are no tasks left = end of threads/scheduler
        }
        // We request the lock again to check if stack is empty and it was previously unlocked by pthread_cond_wait
        // pthread_mutex_lock(&s->mutex);
    }

    s->nb_threads_working++;
    struct work w = pop(s->tasks);
    pthread_mutex_unlock(&s->mutex); // Lock is given back with nb_threads_working incremented & work taken from stack

    taskfunc f = w.f;
    void *closure = w.closure;

    ((void(*)())f)(closure, s); // Going to work

    pthread_mutex_lock(&s->mutex);
    s->nb_threads_working--;
    pthread_mutex_unlock(&s->mutex);

    slippy_time(args); // When back from work go to sleep again
}

// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    pthread_mutex_init(&sched.mutex, NULL);
    pthread_mutex_lock(&sched.mutex);

    pthread_cond_init(&sched.cond_var, NULL);

    sched.tasks = stack_init();
    if (sched.tasks == NULL) {
        pthread_mutex_unlock(&sched.mutex);
        return -1;
    }
    else {
        struct work w = {closure, f};
        push(w, sched.tasks);
    }

    if (nthreads <= 0) {
        sched.nthreads = sched_default_threads();
    }
    else {
        sched.nthreads = nthreads;
    }
    sched.threads = malloc(sizeof(pthread_t) * sched.nthreads);
    sched.qlen = qlen;
    sched.nb_threads_working = 0;

    printf("Unlocking mutex, nb threads: %d, is_empty: %d\n", sched.nthreads, is_empty(sched.tasks));

    struct args_pack argsPack = {closure, &sched};

    pthread_mutex_unlock(&sched.mutex);

    for (int i = 0; i < sched.nthreads; i++) {
        if (pthread_create(&sched.threads[i], NULL, ((void * (*)(void *)) slippy_time), &argsPack) != 0) {
            perror("Failed to create thread");
            return -1;
        }
    }
    void *arg = NULL;
    for (int i = 0; i < sched.nthreads; i++) {
        if (pthread_join(sched.threads[i], arg) != 0) {
            perror("Failed to join thread");
            return -1;
        }
    }
    // TODO Free sched et pthread_t
    free(sched.threads);
    free(sched.tasks);
    return 1;
}

/*
// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler *sched = malloc(sizeof(struct scheduler));

    if (sched == NULL)
        return -1;

    pthread_mutex_init(&sched->mutex, NULL);
    pthread_mutex_lock(&sched->mutex);

    pthread_cond_init(&sched->cond_var, NULL);

    sched->tasks = stack_init();
    if (sched->tasks == NULL) {
        pthread_mutex_unlock(&sched->mutex);
        return -1;
    }
    else {
        struct work w = {closure, f};
        push(w, sched->tasks);
    }

    if (nthreads <= 0) {
        sched->nthreads = sched_default_threads();
    }
    else {
        sched->nthreads = nthreads;
    }
    sched->threads = malloc(sizeof(pthread_t) * sched->nthreads);
    sched->qlen = qlen;
    sched->nb_threads_working = 0;

    printf("Unlocking mutex, nb threads: %d, is_empty: %d\n", sched->nthreads, is_empty(sched->tasks));

    struct args_pack *argsPack = malloc(sizeof(struct args_pack));
    if (argsPack == NULL) {
        pthread_mutex_unlock(&sched->mutex);
        return -1;
    }
    argsPack->closure = closure;
    argsPack->s = sched;

    pthread_mutex_unlock(&sched->mutex);

    for (int i = 0; i < sched->nthreads; i++) {
        if (pthread_create(&sched->threads[i], NULL, ((void * (*)(void *)) slippy_time), argsPack) != 0) {
            perror("Failed to create thread");
            return -1;
        }
    }
    void *arg = NULL;
    for (int i = 0; i < sched->nthreads; i++) {
        if (pthread_join(sched->threads[i], arg) != 0) {
            perror("Failed to join thread");
            return -1;
        }
    }
    // TODO Free sched et pthread_t
    free(sched->threads);
    free(sched->tasks);
    free(argsPack);
    free(sched);
    return 1;
}
 */

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    pthread_mutex_lock(&s->mutex);
    if (size(s->tasks) >= s->qlen) {
        pthread_mutex_unlock(&s->mutex);
        errno = EAGAIN;
        return -1;
    }
    struct work w = {closure, f};
    push(w, s->tasks);

    // TODO only send signal if a thread is waiting ??
    pthread_cond_signal(&s->cond_var);
    pthread_mutex_unlock(&s->mutex);

    return 1;
}
