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

#undef pthread_create
int pthread_create(pthread_t * thread, const pthread_attr_t* attr, void * (*start)(void *), void * arg)
{
    int rc;
    static int (*real_create)(pthread_t * , const pthread_attr_t *, void * (*start)(void *), void *) = NULL;

    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_create")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(thread, attr, start, arg);
    if(!rc) {
        store_id(thread);
        backtrace();
    }
    return rc;
}


}

