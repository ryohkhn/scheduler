#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <unistd.h>

struct scheduler;

typedef void (*taskfunc)(void*, struct scheduler *);

struct work {
    void *closure;
    taskfunc f;
};

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);

#endif //SCHEDULER_H