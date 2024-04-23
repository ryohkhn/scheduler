#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>
#include <string.h>
#include <semaphore.h>

#include "../include/deque.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
    sem_t sem;
    pthread_cond_t cond_var;
    pthread_t *threads;
    struct deque **deques;
    pthread_mutex_t *deques_mutexes;
};

struct args_pack {
    struct scheduler *s;
    struct deque *dq;
    int thread_id;
};

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
    printf("Thread_t %d is a thief\n", thread_id);
    seed_rand();
    int random_thread = next_thread_id(rand(), s->nthreads, thread_id);
    int next_thread = random_thread;
    printf("Stealer chose thread_t %d\n", random_thread);

    do {
        struct deque *rand_dq = s->deques[next_thread];
        printf("Stealer checks thread_t %d\n", next_thread);
        pthread_mutex_lock(&s->deques_mutexes[next_thread]);

        if (!is_empty(rand_dq)) {
            printf("Stealer found labor in thread_t %d !!\n", next_thread);
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
                if (sem_trywait(&s->sem)) {
                    pthread_cond_wait(&s->cond_var, &s->deques_mutexes[id]);
                }
                else {
                    pthread_mutex_unlock(&s->deques_mutexes[id]);
                    return NULL;
                }
            }
        }

        sem_post(&s->sem);
        struct work w = pop_bottom(dq);
        pthread_mutex_unlock(&s->deques_mutexes[id]);

        taskfunc f = w.f;
        void *closure = w.closure;
        f(closure, s); // Going to work
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
    pthread_cond_init(&sched.cond_var, NULL);
    sem_init(&sched.sem, 1, sched.nthreads - 1);

    for (int id = 0; id < sched.nthreads; ++id) {
        struct deque *dq = deque_init();
        if (id == 0) {
            struct work w = {closure, f};
            push_top(w, dq);
        }
        sched.deques[id] = dq;
        struct args_pack argsPack = {&sched, dq, id};
        pthread_mutex_init(&sched.deques_mutexes[id], NULL);
        if(pthread_create(&sched.threads[id], NULL,gaming_time, &argsPack) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            return -1;
        }
    }

    // TODO SCHED_SPAWN LA FONCTION DE BASE DANS LE THREAD 0 (see dumb_sched) et retirer de la boucle

    void *arg = NULL;
    for (int i = 0; i < sched.nthreads; i++) {
        if (pthread_join(sched.threads[i], arg) != 0) {
            fprintf(stderr, "Failed to join thread\n");
            return -1;
        }
    }
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {

}
