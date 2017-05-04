#!/bin/bash

thread_list="1 2 4 8 16"
iters="7"

for threads in `echo $thread_list`; do
    echo "# $threads threads"

    echo "## Pthread"
    for i in `seq 1 $iters`; do
        ./test.sh normal $threads 2> /dev/null | grep fillrandom 
    done

    echo "## TSX"
    for i in `seq 1 $iters`; do
        ./test.sh tsx $threads 2> /dev/null | grep fillrandom 
    done

    echo "## Spin"
    for i in `seq 1 $iters`; do
        ./test.sh spin $threads 2> /dev/null | grep fillrandom 
    done
done
