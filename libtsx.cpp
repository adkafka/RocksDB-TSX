#include <pthread.h> //Get paramteter types
#include <string.h> //memset
#include <stdio.h> //printf
#include <time.h> //Timespec param
#include <unistd.h> //syscall
#include <sys/syscall.h> //syscall names
#include <linux/futex.h> //futex
#include <limits.h> //INT_MAX
#include <condition_variable> //condvar class

#include "rtm.h" //tsx stuff
#include "spin_lock.hpp"

/* How many aborts until we use fall-back lock */
#define NUM_RETRIES_CAPACITY 1 /* Fail due to capacity (_XABORT_CAPACITY) */
#define NUM_RETRIES_CONFLICT 3 /* Fail due to conflict (_XABORT_CONFLICT) */
#define NUM_RETRIES_OTHER 4    /* Failed for any other reason... */

extern "C" {


__attribute__((constructor))
void init(void) { 
}

__attribute__((destructor))
void fini(void) { 
}


/** PTHREAD_MUTEX METHODS **/

#undef pthread_mutex_lock
int pthread_mutex_lock(pthread_mutex_t * mutex){
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);

    unsigned i;
    unsigned status;
    unsigned retry = NUM_RETRIES_OTHER;

    for (i = 0; i < retry; i++) {
        if ((status = _xbegin()) == _XBEGIN_STARTED) {
            /* If lock is not held, we succesfully started a transaction */
            if (!lock->held())
                return 1;
            /* Otherwise, abort and deal with below. Note that an abort returns
             * back to the _xbegin() call */
            _xabort(0xff);
        }
        /* If we explicitly aborted, someone has the lock */
        if ((status & _XABORT_EXPLICIT) && _XABORT_CODE(status) == 0xff) {
            lock->spin_until_free();
        /* If RETRY and CAPACITY are NOT set */
        } else if (!(status & _XABORT_RETRY) && !(status & _XABORT_CAPACITY))
            break;

        /* Conflict */
        if (status & _XABORT_CONFLICT) {
            retry = NUM_RETRIES_CONFLICT;
            /* Could do various kinds of backoff here. */
            lock->spin_until_free();
        /* Capacity */
        } else if (status & _XABORT_CAPACITY) {
            retry = NUM_RETRIES_CAPACITY;
        /* Not a conflict or capacity, but RETRY must have been set */
        } else {
            retry = NUM_RETRIES_OTHER;
        }
    }

    /* All else failed, use fall-back lock */
    lock->acquire();

    return 0;
}

#undef pthread_mutex_unlock
int pthread_mutex_unlock(pthread_mutex_t * mutex){
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);
    /* If no one has the fall-back lock, we can end the transaction */
    if(!lock->held())
        _xend();
    /* If someone has the lock, it must be us, so we release */
    else
        lock->release();

    return 0;
}

#undef pthread_mutex_init 
int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr){ 
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);
    memset(lock,0,sizeof(pthread_mutex_t));
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
    pthread_mutex_init(reinterpret_cast<pthread_mutex_t*>(rwlock),0);
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
int futex_signal(std::atomic<int> *addr) {
    return (futex_wake(addr, 1) >= 0) ? 0 : -1;
}
int futex_broadcast(std::atomic<int> *addr) {
    return (futex_wake(addr, INT_MAX) >= 0) ? 0 : -1;
}
int futex_wait(std::atomic<int> *addr, int val, const struct timespec *to) {
    return sys_futex(addr, FUTEX_WAIT_PRIVATE, val, to, NULL, 0);
}

/* CONDVARS */
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


}// end extern c
