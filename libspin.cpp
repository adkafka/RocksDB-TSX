#include <pthread.h> //Get paramteter types
#include <string.h> //memset
#include <stdio.h> //printf
#include <time.h> //Timespec param
#include <unistd.h> //syscall
#include <sys/syscall.h> //syscall names
#include <linux/futex.h> //futex
#include <limits.h> //INT_MAX
#include <condition_variable> //condvar class

#include "spin_lock.hpp"


/** PTHREAD_MUTEX METHODS **/

#undef pthread_mutex_lock
int pthread_mutex_lock(pthread_mutex_t * mutex){
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);

    lock->acquire();

    return 0;
}

#undef pthread_mutex_unlock
int pthread_mutex_unlock(pthread_mutex_t * mutex){
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);

    lock->release();

    return 0;
}

#undef pthread_mutex_init 
int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr){ 
    memset(mutex,0,sizeof(pthread_mutex_t));
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);
    lock->release();
    return 0;
}


#undef pthread_mutex_trylock
int pthread_mutex_trylock(pthread_mutex_t * mutex){ return 0; }

#undef pthread_mutex_timedlock
int pthread_mutex_timedlock(pthread_mutex_t * mutex, const struct timespec *abs_timeout){ return 0; }

#undef pthread_mutex_consistent
int pthread_mutex_consistent(pthread_mutex_t * mutex){ return 0; }
#undef pthread_mutex_destroy
int pthread_mutex_destroy(pthread_mutex_t * mutex){ return 0; }
/* missing set/getprioceiling */

/* RWLOCKS
 * RW locks can be treated as a normal lock, because TSX will deal 
 * with write conflicts by aborting other transactions */

#undef pthread_rwlock_init 
int pthread_rwlock_init(pthread_rwlock_t* __restrict rwlock, const pthread_rwlockattr_t * __restrict rwattr){
    memset(rwlock,0,sizeof(pthread_rwlock_t));
    spin_lock* lock = reinterpret_cast<spin_lock*>(rwlock);
    lock->release();
    return 0;
}

#undef pthread_rwlock_rdlock 
int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock){
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(rwlock));
    return 0;
}
#undef pthread_rwlock_wrlock 
int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock){
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(rwlock));
    return 0;
}
#undef pthread_rwlock_unlock 
int pthread_rwlock_unlock(pthread_rwlock_t* rwlock){
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(rwlock));
    return 0;
}
#undef pthread_rwlock_destroy 
int pthread_rwlock_destroy(pthread_rwlock_t* rwlock){ return 0; }
/* Missing trylocks for rw */


/* Futex */
static int sys_futex(std::atomic<int> *uaddr, int futex_op, int val,
        const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, reinterpret_cast<int*>(uaddr), 
            futex_op, val, timeout, uaddr, val3);
}
int futex_wake(std::atomic<int> *addr, int nr) {
    return sys_futex(addr, FUTEX_WAKE_PRIVATE, nr, NULL, NULL, 0);
}
void futex_signal(void* arg) {
    auto addr = reinterpret_cast<std::atomic<int>*>(arg);
    futex_wake(addr, 1);
}
void futex_broadcast(void* arg) {
    auto addr = reinterpret_cast<std::atomic<int>*>(arg);
    futex_wake(addr, INT_MAX);
}
int futex_wait(std::atomic<int> *addr, int val, const struct timespec *to) {
    return sys_futex(addr, FUTEX_WAIT_PRIVATE, val, to, NULL, 0);
}

/* CONDVARS (C++)*/
std::condition_variable::condition_variable(){
    auto fut = reinterpret_cast<std::atomic<int>*>(this);
    (*fut) = 0;
}

std::condition_variable::~condition_variable(){
}

void std::condition_variable::notify_all(){
    auto fut = reinterpret_cast<std::atomic<int>*>(this);
    (*fut)++;
    futex_broadcast(fut);
}

void std::condition_variable::notify_one(){
    auto fut = reinterpret_cast<std::atomic<int>*>(this);
    (*fut)++;
    futex_signal(fut);
}

void std::condition_variable::wait(std::unique_lock<std::mutex>& cv_m){
    auto fut = reinterpret_cast<std::atomic<int>*>(this);
    int val = *fut;

    cv_m.unlock();
    futex_wait(fut,val,NULL);
    cv_m.lock();
}

/* CONDVARS (C) */

#undef pthread_cond_init 
int pthread_cond_init(pthread_cond_t * __restrict cond, const pthread_condattr_t * __restrict attr){
    auto fut = reinterpret_cast<std::atomic<int>*>(cond);
    (*fut) = 0;
    return 0;
}
#undef pthread_cond_destroy 
int pthread_cond_destroy(pthread_cond_t * cond){ return 0;}

#undef pthread_cond_broadcast
int pthread_cond_broadcast(pthread_cond_t * cond){
    auto fut = reinterpret_cast<std::atomic<int>*>(cond);
    (*fut)++;
    futex_broadcast(fut);
    return 0;
}

#undef pthread_cond_signal
int pthread_cond_signal(pthread_cond_t * cond){
    auto fut = reinterpret_cast<std::atomic<int>*>(cond);
    (*fut)++;
    futex_signal(fut);
    return 0;
}

#undef pthread_cond_wait
int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t *mutex){
    auto fut = reinterpret_cast<std::atomic<int>*>(cond);
    int val = *fut;

    pthread_mutex_unlock(mutex);
    futex_wait(fut,val,NULL);
    pthread_mutex_lock(mutex);

    return 0;
}

#undef pthread_cond_timedwait 
int pthread_cond_timedwait(pthread_cond_t * __restrict cond, pthread_mutex_t * __restrict mutex, 
        const struct timespec * __restrict time){
    fprintf(stderr,"pthread_cond_timedwait is unimplemented\n");
    exit(1);
}
