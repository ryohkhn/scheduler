#include <pthread.h>
#include "sched.h"
#include "stack.h"

struct scheduler {
    struct stack tasks;
    int qlen; //nombre max de tâches dans la pile
    int nthreads;
    pthread_t threads[];
};

int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    //TODO initialise a struct scheduler and make it consume tasks
    //Return -1 if failed to initialize or 1 if all the work is done
    return 0;
}
int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    //TODO add a task f on top of the stack
    return 0;
}