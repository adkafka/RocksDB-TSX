#include <stdio.h>
#include <stdint.h>
#include <bits/pthreadtypes.h>
#include <dlfcn.h>

#include <cstring>

#include "backtrace.hpp"

extern "C" {

__attribute__((constructor)) void init(void) { 
    fprintf(stderr, "Loaded pthread interpositioning library\n");
}
__attribute__((destructor))  void fini(void) { 
    fprintf(stderr, "Unloaded pthread interpositioning library\n");
}


void store_id(pthread_t  * id) {
    fprintf(stderr, "new thread created with id  0x%lx\n", (*id));
}

#undef pthread_mutex_init 
int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutexx, const pthread_mutexattr_t *mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex, mutexattr);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_init\n");
        //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_lock
int pthread_mutex_lock(pthread_mutex_t * mutex){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_lock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_lock\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_trylock
int pthread_mutex_trylock(pthread_mutex_t * mutex){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_trylock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_trylock\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_timedlock
int pthread_mutex_timedlock(pthread_mutex_t * mutex, const struct timespec *abs_timeout){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex, const struct timespec *abs_timeout);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_timedlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex,abs_timeout);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_timedlock\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_unlock
int pthread_mutex_unlock(pthread_mutex_t * mutex){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_unlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_unlock\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_consistent
int pthread_mutex_consistent(pthread_mutex_t * mutex){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_consistent")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_consistent\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutex_destroy
int pthread_mutex_destroy(pthread_mutex_t * mutex){
{
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutex_destroy\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}



#undef pthread_mutexattr_destroy
int pthread_mutexattr_destroy(pthread_mutexattr_t * mutexattr){
{
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutexattr_destroy\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutexattr_init
int pthread_mutexattr_init(pthread_mutexattr_t * mutexattr){
{
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutexattr_init\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_mutexattr_settype
int pthread_mutex_settype(pthread_mutexattr_t * mutexattr){
{
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_settype")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_mutexattr_settypey\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_cond_broadcast
int pthread_cond_braodcast(pthread_cond_t * cond){
{
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_broadcast")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    if(!rc) {
	fprintf(stderr, "Interposing on pthread_cond_broadcasty\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_cond_destroy 
int pthread_cond_destroy(pthread_cond_t * cond){
{
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_cond_destroy\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_cond_init 
int pthread_cond_init(pthread_cond_t * __restrict cond, const pthread_condattr_t * __restrict attr){
{
    int rc;
    static int (*real_create)(pthread_cond_t * __restrict cond, const pthread_condattr_t *__restrict attr );
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond,attr);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_cond_init\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_cond_signal
int pthread_cond_signal(pthread_cond_t * cond){
{
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_signal")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_cond_signal\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}
#undef pthread_cond_timedwait 
int pthread_cond_timedwait(pthread_cond_t * __restrict cond, pthread_mutex_t * __restrict mutex, const struct timespec * __restrict time){
{
    int rc;
    static int (*real_create)(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex, const struct timespec *__restrict time);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_timedwait")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond,mutex,time);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_cond_timedwait\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_rwlock_destroy 
int pthread_rwlock_destroy(pthread_rwlock_t * rwlock){
{
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_rwlock_destroy\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_rwlock_init 
int pthread_rwlock_init(pthread_rwlock_t * __restrict rwlock, const pthread_rwlockattr_t * __restrict rwattr){
{
    int rc;
    static int (*real_create)(pthread_rwlock_t * __restrict rwlock, const pthread_rwlockattr_t * __restrict rwattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock, rwattr);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_rwlock_init\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_rwlock_rdlock 
int pthread_rwlock_rdlock(pthread_rwlock_t * rwlock){
{
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_rdlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_rwlock_rdlock\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_rwlock_unlock 
int pthread_rwlock_unlock(pthread_rwlock_t * rwlock){
{
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_unlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_rwlock_unlock\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}

#undef pthread_rwlock_wrlock 
int pthread_rwlock_wrlock(pthread_rwlock_t * rwlock){
{
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_wrlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    if(!rc) {
        fprintf(stderr, "Interposing on pthread_rwlock_wrlock\n");
         //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}
}
