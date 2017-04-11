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
	fprintf(stderr, "Interposing on pthread_destroy\n");
	 //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}
}

