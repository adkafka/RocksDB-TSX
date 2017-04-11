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
        //store_id(thread);
    //    backtrace();
    }
    return rc;
}
}
}

