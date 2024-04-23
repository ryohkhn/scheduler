#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>
#include <string.h>
#include <errno.h>

#include "../include/deque.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
    int nb_threads_working;
    pthread_mutex_t threads_working_mutex;
    pthread_t *threads;
    struct deque **deques;
    pthread_mutex_t *deques_mutexes;
};

struct args_pack {
    struct scheduler *s;
    struct deque *dq;
    int thread_id;
};

void cleanup_sched(struct scheduler *sched) {
    free(sched->threads);
    for (int i = 0; i < sched->nthreads; ++i) {
        free_up(sched->deques[i]);
    }
    for (int i = 0; i < sched->nthreads; ++i) {
        pthread_mutex_destroy(&sched->deques_mutexes[i]);
    }
}

void seed_rand() {
    unsigned int seed;

    if(getrandom(&seed, sizeof(seed), 0) != sizeof(seed)) {
        fprintf(stderr, "Random seed generator failed, time used as a seed instead");
        srand(time(NULL));
    }
    srand(seed);
}

int next_thread_id(int currend_id, int n_threads, int og_id) {
    int next = (currend_id + 1) % n_threads;
    // Skip the case where the thread tries to steal himself
    if (og_id == next)
        next_thread_id(next, n_threads, og_id);
    return next;
}

int steal_work(struct scheduler *s, struct deque *dq, int thread_id) {
    pthread_mutex_unlock(&s->deques_mutexes[thread_id]);
    // Prevent from infinite looping when the scheduler is serial
    if (s->nthreads == 1) return 0;
    seed_rand();
    int random_thread = next_thread_id(rand(), s->nthreads, thread_id);
    int next_thread = random_thread;

    do {
        struct deque *rand_dq = s->deques[next_thread];
        pthread_mutex_lock(&s->deques_mutexes[next_thread]);

        if (!is_empty(rand_dq)) {
            struct work w = pop_top(rand_dq);
            pthread_mutex_unlock(&s->deques_mutexes[next_thread]);
            pthread_mutex_lock(&s->deques_mutexes[thread_id]);
            push_bottom(w, dq);
            return 1;
        }
        pthread_mutex_unlock(&s->deques_mutexes[next_thread]);
        next_thread = next_thread_id(next_thread, s->nthreads, thread_id);
    }
    while (next_thread != random_thread);

    return 0;
}

void *gaming_time(void* args) {
    struct scheduler *s = ((struct args_pack *)args)->s;
    struct deque *dq = ((struct args_pack *)args)->dq;
    int id = ((struct args_pack *)args)->thread_id;

    while (1) {
        pthread_mutex_lock(&s->deques_mutexes[id]);

        while (is_empty(dq)) {
            if (steal_work(s, dq, id))
                break;
            else {
                pthread_mutex_lock(&s->threads_working_mutex);
                if (s->nb_threads_working == 0) {
                    pthread_mutex_unlock(&s->deques_mutexes[id]);
                    pthread_mutex_unlock(&s->threads_working_mutex);
                    return NULL;
                }
                pthread_mutex_unlock(&s->threads_working_mutex);

                pthread_mutex_unlock(&s->deques_mutexes[id]);
                usleep(1000);
                pthread_mutex_lock(&s->deques_mutexes[id]);
            }
        }
        struct work w = pop_bottom(dq);
        taskfunc f = w.f;
        void *closure = w.closure;

        pthread_mutex_lock(&s->threads_working_mutex);
        s->nb_threads_working++;
        pthread_mutex_unlock(&s->threads_working_mutex);

        pthread_mutex_unlock(&s->deques_mutexes[id]);

        f(closure, s); // Going to work

        pthread_mutex_lock(&s->threads_working_mutex);
        s->nb_threads_working--;
        pthread_mutex_unlock(&s->threads_working_mutex);
    }
}

// TODO CHECK MALLOC & SYSTEM CALLS
// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    if (nthreads <= 0 || nthreads > sched_default_threads())
        sched.nthreads = sched_default_threads();
    else
        sched.nthreads = nthreads;

    pthread_mutex_init(&sched.threads_working_mutex, NULL);

    sched.threads = malloc(sizeof(pthread_t) * sched.nthreads);
    if (!sched.threads) {
        fprintf(stderr, "Failed to malloc threads array\n");
        return -1;
    }
    sched.deques = malloc(sizeof(struct deque*) * sched.nthreads);
    if (!sched.deques) {
        fprintf(stderr, "Failed to malloc deques array\n");
        return -1;
    }
    sched.deques_mutexes = malloc(sizeof(pthread_mutex_t) * sched.nthreads);
    if (!sched.deques_mutexes) {
        fprintf(stderr, "Failed to malloc mutexes array\n");
        return -1;
    }
    sched.qlen = qlen;
    sched.nb_threads_working = 0;

    for (int id = 0; id < sched.nthreads; ++id) {
        struct deque *dq = deque_init();
        sched.deques[id] = dq;
        struct args_pack argsPack = {&sched, dq, id};
        pthread_mutex_init(&sched.deques_mutexes[id], NULL);
        if(pthread_create(&sched.threads[id], NULL,gaming_time, &argsPack) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            return -1;
        }
    }

    if (!sched_spawn(f, closure, &sched)) {
        fprintf(stderr, "Failed to create the inital task\n");
        // cleanup_sched(&sched);
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

int find_thread(struct scheduler *s) {
    pthread_t adr = pthread_self();

    for (int i = 0; i < s->nthreads; ++i) {
        pthread_t current_thread = s->threads[i];
        if (pthread_equal(adr, current_thread)) {
            return i;
        }
    }
    return -1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    int sum = 0;
    for (int i = 0; i < s->nthreads; i++) {
        pthread_mutex_lock(&s->deques_mutexes[i]);
        sum += size(s->deques[i]);
        pthread_mutex_unlock(&s->deques_mutexes[i]);
    }
    int id = find_thread(s);
    if (id == -1) {
        id = 0;
    }
    if (sum >= s->qlen) {
        cleanup_sched(s);
        errno = EAGAIN;
        perror("The task amount is superior than what the scheduler can handle");
        return -1;
    }

    struct work w = {closure, f};
    pthread_mutex_lock(&s->deques_mutexes[id]);
    push_bottom(w, s->deques[id]);
    pthread_mutex_unlock(&s->deques_mutexes[id]);

    return 1;
}
