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

class TLS_attributes{
    public:
        int locks, attempts, exp, ret, con, cap, deb, nest, fbl, other;
        int fut_w, fut_s, fut_b;
        TLS_attributes(){
            locks=0, attempts=0; exp=0; ret=0; con=0; cap=0; deb=0; nest=0; fbl=0; other=0;
            fut_w=0,fut_s=0;fut_b=0;
        }
        ~TLS_attributes(){
            printf("Thread_local aborts: \n\
                    Locks:\t%d\n\
                    Attempts:\t%d\n\
                    Explicit:\t%d\n\
                    Retry:\t%d\n\
                    Conflict:\t%d\n\
                    Capacity:\t%d\n\
                    Debug:\t%d\n\
                    Nested:\t%d\n\
                    Other:\t%d\n\
                    Fallback:\t%d\n",
                    locks,attempts,exp,ret,con,cap,deb,nest,other,fbl);
            printf("Cond_Var usage: \n\
                    Waits:\t%d\n\
                    Signals:\t%d\n\
                    Bcasts:\t%d\n",
                    fut_w,fut_s,fut_b);
        }
};

volatile thread_local TLS_attributes stats;

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

    stats.locks++;

    for (i = 0; i < retry; i++) {
        stats.attempts++;
        if ((status = _xbegin()) == _XBEGIN_STARTED) {
            /* If lock is not held, we succesfully started a transaction */
            if (!lock->held())
                return 1;
            /* Otherwise, abort and deal with below. Note that an abort returns
             * back to the _xbegin() call */
            _xabort(0xff);
        }
        stats.attempts++;
        /* If we explicitly aborted, someone has the lock */
        if ((status & _XABORT_EXPLICIT) && _XABORT_CODE(status) == 0xff) {
            lock->spin_until_free();
            stats.exp++;
        } 
        /* Retry */
        if (status & _XABORT_RETRY){
            stats.ret++;
        }
        /* Conflict */
        if (status & _XABORT_CONFLICT) {
            stats.con++;
        }
        /* Capacity */
        if (status & _XABORT_CAPACITY) {
            stats.cap++;
        }
        /* Nested fail*/
        if (status & _XABORT_NESTED) {
            stats.nest++;
        } 
        /* Debug */
        if (status & _XABORT_DEBUG){
            stats.deb++;
        }
        if (status == 0){
            stats.other++;
            break;
        }
    }

    /* All else failed, use fall-back lock */
    stats.fbl++;
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
int futex_signal(std::atomic<int> *addr) {
    stats.fut_s++;
    return (futex_wake(addr, 1) >= 0) ? 0 : -1;
}
int futex_broadcast(std::atomic<int> *addr) {
    stats.fut_b++;
    return (futex_wake(addr, INT_MAX) >= 0) ? 0 : -1;
}
int futex_wait(std::atomic<int> *addr, int val, const struct timespec *to) {
    stats.fut_w++;
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



}// end extern c
