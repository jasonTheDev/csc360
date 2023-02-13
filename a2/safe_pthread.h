#ifndef _SAFEPTHREAD_H_
#define _SAFEPTHREAD_H_

/* mutex */
void safe_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
void safe_mutex_destroy(pthread_mutex_t *mutex);
void safe_mutex_lock(pthread_mutex_t *mutex);
void safe_mutex_unlock(pthread_mutex_t *mutex);

/* cond */
void safe_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
void safe_cond_destroy(pthread_cond_t *cond);
void safe_cond_broadcast(pthread_cond_t *cond);
void safe_cond_signal(pthread_cond_t *cond);
void safe_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

/* threads */
void safe_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
void safe_pthread_join(pthread_t thread, void **value_ptr);

#endif
