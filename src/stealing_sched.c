#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

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

int steal_work(struct scheduler *s, struct deque *dq) {
    seed_rand();
    int random_thread = rand() % s->nthreads;
    printf("Stealer chose thread_t %d\n", random_thread);
    int next_thread = (random_thread + 1) % s->nthreads;
    struct deque *rand_dq = s->deques[random_thread];

    while (next_thread != random_thread) {
        printf("Stealer checks thread_t %d\n", next_thread);
        if (!is_empty(rand_dq)) {
            printf("Stealer found work in thread_t %d !!\n", next_thread);
            struct work w = pop_top(rand_dq);
            push_bottom(w, dq);
            return 1;
        }
        next_thread = (next_thread + 1) % s->nthreads;
    }

    return 0;
}

void gaming_time(void* args) {
    struct scheduler *s = ((struct args_pack *)args)->s;
    struct deque *dq = ((struct args_pack *)args)->dq;
    int id = ((struct args_pack *)args)->thread_id;

    while (1) {
        pthread_mutex_lock(&s->deques_mutexes[id]);

        while (is_empty(dq)) {
            // TODO Mutex is kept locked during stealing, maybe faster if unlocked but risk of getting stolen before executing ?
            if (steal_work(s, dq)) {
                break;
            }
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
