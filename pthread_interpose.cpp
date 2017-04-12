#include <stdio.h>
#include <stdint.h>
#include <bits/pthreadtypes.h> //get param types
#include <dlfcn.h> //dlsym

#include <cstring> //memcpy
#include <fstream> //file writing

#include "backtrace.hpp"

#define LOG_FILE "mutex_usage.log"


extern "C" {

static std::ofstream* ofs;

__attribute__((constructor)) void init(void) { 
    ofs = new std::ofstream();
    if(ofs==NULL){
        perror("constructor");
        exit(1);
    }
    ofs->open(LOG_FILE, std::ofstream::out | std::ofstream::app);
    printf("ofs: %p\n",ofs);
}
__attribute__((destructor))  void fini(void) { 
    ofs->close();
    delete ofs;
}

// Prevent backtrace from calling backtrace again
thread_local bool use_real_func = false;


void log_func(const char* func_name){
    // If we should use real func, no op
    if(use_real_func==false){
        if(!ofs) init();
        use_real_func = true;
        (*ofs) << "(0) " << func_name << "\n";
        backtrace(ofs);
        (*ofs) <<"\n";
        use_real_func = false;
    }
}


#undef pthread_mutex_init 
int pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex, const pthread_mutexattr_t *mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex, mutexattr);
    log_func("pthread_mutex_init");
    return rc;
}

#undef pthread_mutex_lock
int pthread_mutex_lock(pthread_mutex_t * mutex){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_lock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    log_func("pthread_mutex_lock");
    return rc;
}

#undef pthread_mutex_trylock
int pthread_mutex_trylock(pthread_mutex_t * mutex){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_trylock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    log_func("pthread_mutex_trylock");
    return rc;
}

#undef pthread_mutex_timedlock
int pthread_mutex_timedlock(pthread_mutex_t * mutex, const struct timespec *abs_timeout){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex, const struct timespec *abs_timeout);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_timedlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex,abs_timeout);
    log_func("pthread_mutex_timedlock");
    return rc;
}

#undef pthread_mutex_unlock
int pthread_mutex_unlock(pthread_mutex_t * mutex){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_unlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    log_func("pthread_mutex_unlock");
    return rc;
}

#undef pthread_mutex_consistent
int pthread_mutex_consistent(pthread_mutex_t * mutex){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_consistent")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    log_func("pthread_mutex_consistent");
    return rc;
}

#undef pthread_mutex_destroy
int pthread_mutex_destroy(pthread_mutex_t * mutex){
    int rc;
    static int (*real_create)(pthread_mutex_t * mutex);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutex_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutex);
    log_func("pthread_mutex_destroy");
    return rc;
}

#undef pthread_mutexattr_destroy
int pthread_mutexattr_destroy(pthread_mutexattr_t * mutexattr){
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    log_func("pthread_mutexattr_destroy");
    return rc;
}

#undef pthread_mutexattr_init
int pthread_mutexattr_init(pthread_mutexattr_t * mutexattr){
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    log_func("pthread_mutexattr_init");
    return rc;
}

#undef pthread_mutexattr_settype
int pthread_mutex_settype(pthread_mutexattr_t * mutexattr){
    int rc;
    static int (*real_create)(pthread_mutexattr_t * mutexattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_mutexattr_settype")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(mutexattr);
    log_func("pthread_mutexattr_settype");
    return rc;
}

#undef pthread_cond_broadcast
int pthread_cond_braodcast(pthread_cond_t * cond){
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_broadcast")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    log_func("pthread_cond_broadcast");
    return rc;
}

#undef pthread_cond_destroy 
int pthread_cond_destroy(pthread_cond_t * cond){
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    log_func("pthread_cond_destroy ");
    return rc;
}

#undef pthread_cond_init 
int pthread_cond_init(pthread_cond_t * __restrict cond, const pthread_condattr_t * __restrict attr){
    int rc;
    static int (*real_create)(pthread_cond_t * __restrict cond, const pthread_condattr_t *__restrict attr );
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond,attr);
    log_func("pthread_cond_init ");
    return rc;
}

#undef pthread_cond_signal
int pthread_cond_signal(pthread_cond_t * cond){
    int rc;
    static int (*real_create)(pthread_cond_t * cond);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_signal")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond);
    log_func("pthread_cond_signal");
    return rc;
}

#undef pthread_cond_timedwait 
int pthread_cond_timedwait(pthread_cond_t * __restrict cond, pthread_mutex_t * __restrict mutex, 
        const struct timespec * __restrict time){
    int rc;
    static int (*real_create)(pthread_cond_t *__restrict cond, pthread_mutex_t *__restrict mutex, 
            const struct timespec *__restrict time);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_cond_timedwait")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(cond,mutex,time);
    log_func("pthread_cond_timedwait ");
    return rc;
}

#undef pthread_rwlock_destroy 
int pthread_rwlock_destroy(pthread_rwlock_t * rwlock){
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_destroy")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    log_func("pthread_rwlock_destroy ");
    return rc;
}

#undef pthread_rwlock_init 
int pthread_rwlock_init(pthread_rwlock_t * __restrict rwlock, const pthread_rwlockattr_t * __restrict rwattr){
    int rc;
    static int (*real_create)(pthread_rwlock_t * __restrict rwlock, const pthread_rwlockattr_t * __restrict rwattr);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_init")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock, rwattr);
    log_func("pthread_rwlock_init ");
    return rc;
}

#undef pthread_rwlock_rdlock 
int pthread_rwlock_rdlock(pthread_rwlock_t * rwlock){
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_rdlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    log_func("pthread_rwlock_rdlock ");
    return rc;
}

#undef pthread_rwlock_unlock 
int pthread_rwlock_unlock(pthread_rwlock_t * rwlock){
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_unlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    log_func("pthread_rwlock_unlock ");
    return rc;
}

#undef pthread_rwlock_wrlock 
int pthread_rwlock_wrlock(pthread_rwlock_t * rwlock){
    int rc;
    static int (*real_create)(pthread_rwlock_t * rwlock);
    if (!real_create){
        const void* addr{dlsym(RTLD_NEXT, "pthread_rwlock_wrlock")};
        std::memcpy(&real_create,&addr, sizeof(addr));
    }

    rc = real_create(rwlock);
    log_func("pthread_rwlock_wrlock");
    return rc;
}

}// End external C
