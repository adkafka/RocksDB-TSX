#!/bin/bash

FILE="mutex_usage.log"

if [[ -n $1 ]]; then
    FILE=$1
fi

cat $FILE | \
    egrep -o "pthread_cond_[a-z]*|pthread_mutex_[a-z]*|pthread_rwlock_[a-z]*|std::condition_variable::[~a-z]*" |\
    sort | uniq -c

ALL_LOCKS=`cat $FILE | grep "_lock addr: 0x[0-9a-f]*" | sort | uniq -c`

NUM_LOCKS=`echo "$ALL_LOCKS" | wc -l`

TOP_LOCKS=`echo "$ALL_LOCKS" | sort -nr | head -20`

echo "Number of locks: $NUM_LOCKS"
echo "Top contended locks:"
echo "$TOP_LOCKS"
