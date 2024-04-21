#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>
#include <string.h>

#include "../include/deque.h"

struct scheduler {
    int nthreads;
    int qlen; // Maximum number of tasks
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
       perror("Random seed generator failed, time used instead");
       srand(time(NULL));
    }
    srand(seed);
}

int next_thread_id(int currend_id, int n_threads) {
    return (currend_id + 1) % n_threads;
}

int steal_work(struct scheduler *s, struct deque *dq, int thread_id) {
    printf("Thread_t %d is a thief\n", thread_id);
    pthread_mutex_unlock(&s->deques_mutexes[thread_id]);
    seed_rand();
    int random_thread = rand() % s->nthreads;
    int next_thread = random_thread;
    printf("Stealer chose thread_t %d\n", random_thread);

    do {
        struct deque *rand_dq = s->deques[next_thread];
        pthread_mutex_lock(&s->deques_mutexes[next_thread]);
        printf("Stealer checks thread_t %d\n", next_thread);

        if (!is_empty(rand_dq)) {
            printf("Stealer found labor in thread_t %d !!\n", next_thread);
            struct work w = pop_top(rand_dq);
            pthread_mutex_unlock(&s->deques_mutexes[next_thread]);
            pthread_mutex_lock(&s->deques_mutexes[thread_id]);
            push_bottom(w, dq);
            return 1;
        }
        pthread_mutex_unlock(&s->deques_mutexes[next_thread]);
        next_thread = next_thread_id(next_thread, s->nthreads);
    }
    while (next_thread != random_thread);

    return 0;
}

void gaming_time(void* args) {
    struct scheduler *s = ((struct args_pack *)args)->s;
    struct deque *dq = ((struct args_pack *)args)->dq;
    int id = ((struct args_pack *)args)->thread_id;

    while (1) {
        pthread_mutex_lock(&s->deques_mutexes[id]);

        while (is_empty(dq)) {
            if (steal_work(s, dq, id))
                break;
            else {
                pthread_mutex_unlock(&s->deques_mutexes[id]);
                usleep(1000);
                pthread_mutex_lock(&s->deques_mutexes[id]);
            }
        }
        struct work w = pop_bottom(dq);
        taskfunc f = w.f;
        void *closure = w.closure;

        pthread_mutex_unlock(&s->deques_mutexes[id]);

        ((void(*)())f)(closure, s); // Going to work
    }
}

// TODO CHECK MALLOC & SYSTEM CALLS
// Return -1 if failed to initialize or 1 if all the work is done
int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    struct scheduler sched;

    if (nthreads <= 0)
        sched.nthreads = sched_default_threads();
    else
        sched.nthreads = nthreads;

    sched.threads = malloc(sizeof(pthread_t) * sched.nthreads);
    if (!sched.threads) {
        perror("Failed to malloc threads array");
        return -1;
    }
    sched.qlen = qlen;
    sched.deques = malloc(sizeof(struct deque*) * sched.nthreads);
    if (!sched.deques) {
        perror("Failed to malloc deques array");
        return -1;
    }
    sched.deques_mutexes = malloc(sizeof(pthread_mutex_t) * sched.nthreads);
    if (!sched.deques_mutexes) {
        perror("Failled to malloc mutexes array");
        return -1;
    }

    for (int id = 0; id < sched.nthreads; ++id) {
        struct deque *dq = deque_init();
        if (id == 0) {
            struct work w = {closure, f};
            push_top(w, dq);
        }
        sched.deques[id] = dq;
        struct args_pack argsPack = { &sched, dq, id};
        pthread_mutex_init(&sched.deques_mutexes[id], NULL);
        if(pthread_create(&sched.threads[id], NULL,(void *(*)(void *)) gaming_time, &argsPack) != 0) {
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
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {

}
