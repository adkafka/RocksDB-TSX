#!/bin/bash

cat mutex_usage.log | \
    egrep -o "pthread_cond_[a-z]*|pthread_mutex_[a-z]*|pthread_rwlock_[a-z]*|std::condition_variable::[~a-z]*" |\
    sort | uniq -c
