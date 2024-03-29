#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "headers/lib/lib.h"
#include "headers/delay.h"

int c = 0;  // JMC
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int task(int level) {
    usleep(delay*1000);

    fprintf(stderr, "[lib] a %d task is starting (with %d delay)\n", level, delay);
    usleep(level*10000);
    fprintf(stderr, "[lib] a %d task has finished\n", level);

    pthread_mutex_lock(&lock);
    c += 10;
    int n = c;
    pthread_mutex_unlock(&lock);
    return n;
}
