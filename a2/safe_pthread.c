#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "safe_pthread.h"


// mutex
void safe_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    if (pthread_mutex_init(mutex, attr) != 0) {
        printf("ERROR: pthread_mutex_init() failed\n");
        exit(1);
    } 
}

void safe_mutex_destroy(pthread_mutex_t *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) {
        printf("ERROR: pthread_mutex_destroy() failed\n");
        exit(1);
    }
}

void safe_mutex_lock(pthread_mutex_t *mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        printf("ERROR: pthread_mutex_lock() failed\n");
        exit(1);
    }
}

void safe_mutex_unlock(pthread_mutex_t *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        printf("ERROR: pthread_mutex_unlock() failed\n");
        exit(1);
    }
}

// condv
void safe_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
    if (pthread_cond_init(cond, attr) != 0) {
        printf("ERROR: pthread_cond_init() failed\n");
        exit(1);
    }
}

void safe_cond_destroy(pthread_cond_t *cond) {
    if (pthread_cond_destroy(cond) != 0) {
        printf("ERROR: pthread_cond_destroy() failed\n");
        exit(1);
    }
}

void safe_cond_broadcast(pthread_cond_t *cond) {
    if (pthread_cond_broadcast(cond) != 0) {
        printf("ERROR: pthread_cond_broadcast() failed\n");
        exit(1);
    }
}

void safe_cond_signal(pthread_cond_t *cond) {
    if (pthread_cond_signal(cond) != 0) {
        printf("ERROR: pthread_cond_signal() failed\n");
        exit(1);
    }
}

void safe_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    if (pthread_cond_wait(cond, mutex) != 0) {
        printf("ERROR: pthread_cond_wait() failed\n");
        exit(1);
    }
}

// threads
void safe_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    if (pthread_create(thread, attr, start_routine, arg) != 0) {
        printf("ERROR: pthread_create() failed\n");
        exit(1);
    }
}

void safe_pthread_join(pthread_t thread, void **value_ptr) {
    if (pthread_join(thread, value_ptr) != 0) {
        printf("ERROR: pthread_join() failed\n");
        exit(1);
    }
}