#include <bits/pthreadtypes.h> //get param types
#include <string.h> //memset
#include <stdio.h> //printf

#include "rtm.h" //tsx stuff
#include "spin_lock.hpp"

/* How many aborts until we use fall-back lock */
#define NUM_RETRIES_CAPACITY 1 /* Fail due to capacity (_XABORT_CAPACITY) */
#define NUM_RETRIES_CONFLICT 3 /* Fail due to conflict (_XABORT_CONFLICT) */
#define NUM_RETRIES_OTHER 4    /* Failed for any other reason... */

extern "C" {


__attribute__((constructor))
void init(void) { 
    //printf("IN INITIALIZER\n");
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

    return 1;
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

    return 1;
}

#undef pthread_mutex_init 
int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr){ 
    spin_lock* lock = reinterpret_cast<spin_lock*>(mutex);
    memset(lock,0,sizeof(pthread_mutex_t));
    return 1;
}


#undef pthread_mutex_trylock
int pthread_mutex_trylock(pthread_mutex_t * mutex){
    return 0;
}

#undef pthread_mutex_timedlock
int pthread_mutex_timedlock(pthread_mutex_t * mutex, const struct timespec *abs_timeout){
    return 0;
}

#undef pthread_mutex_consistent
int pthread_mutex_consistent(pthread_mutex_t * mutex){
    return 0;
}


}// end extern c