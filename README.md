# Notes for CSE403 project, RocksDB+TSX
Adam Kafka and Ryan Santos

## Next steps
- Interposition on pthread library (at least the funcions that rocksDB uses, see 'Gathering the functions')
    - Verify we are doing this on all functions we identified as 'interesting'
    - Log on the actual benchmarks, get preliminary data (how many locks, which ops are most common)
    - Make pretty graphs :)
- Replace usage of locks with naïve spin locks (what about condvars?) to get a useful worst-case data point (what if all transactions fail)
- Use TSX to replace Locks (pthread\_mutex and pthread\_rw\_lock)
- Read spear's paper and replace Cond Vars...
- Run benchmark to test it and get performance results
- Tweak params (how many aborts until fall-back) to get more results

## Design of Lock Elision
### Locks
We will use TSX RTM to perform lock elision, using a spin lock as a fallback mechanism. I think the best option (to get a spin lock), is to use the pthread\_mutex\_t pointer itself. Reinterpert cast it to what we need (an atomic bool) so that we can use it as a spin lock. This will work as long the size of our spin lock is less than the size of pthread\_mutex\_t. Another option would be to store an in memory map of pthread\_mutex\_t to spin lock... but this has many downsides, though may be unavoidable in CondVars...

References for this solution include:
- [tsx-tools](https://github.com/andikleen/tsx-tools/blob/master/locks/spin-rtm.c)
- [glibc](https://lwn.net/Articles/534758/)
- [acm](https://queue.acm.org/detail.cfm?id=2579227)
- [gcc](https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/X86-transactional-memory-intrinsics.html#X86-transactional-memory-intrinsics)
- [intel ppt](http://www.halobates.de/adding-lock-elision-to-linux.pdf)
- [utoronto ppt](http://individual.utoronto.ca/mikedaiwang/tm/Intel_TSX_Overview.pdf)
- [intel handbook](https://www-ssl.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf)
- [Spin lock in c++](http://en.cppreference.com/w/cpp/atomic/atomic_flag)

### RWLocks
I believe we can treat RW locks the same as normal locks, because TM will catch any writes and cause reads (and other writes) to abort. RW locks are designed for mutliple readers, one writer. TM will place the memory in the corresponding read/write set and detect any issues via cache coherence.

### CondVars
The best option will be to base our implementation off of [Wang and Spear's work](http://transact2014.cse.lehigh.edu/wang2.pdf). The corresponding [github link](https://github.com/mfs409/transmem/tree/master/libs/libtmcondvar) & [another repo](https://github.com/mfs409/libcondvar) will be very valuable as well. We will need a per thread link list of semaphores (construct it during library construction). We will have to be careful about when we 'commit' our transaction. I think we would need a thread-safe map to map the condition variable ptr to our linked list (pthread\_cond\_t will be too small)... 

Other option would be based off [Richard Yoo paper](http://pages.cs.wisc.edu/~rajwar/papers/SC13_TSX.pdf). Uses Linux futex to atomically put self on wait list. Signal thread registers a callback if it signals. Thread will execute callback to update futex. BusyWait did the best in the paper, so we should impliment that if we can. Ideally, we would implement both and compare...

### Reusing pthread types
We should use the parameters to the pthread functions to store the data we need to add on. For example, pthread acquire should reinterpert cast the pthread\_mutex\_t pointer to a spin\_lock pointer that we can use as a fall-back lock. Reusing condvar will be a lot harder, cause we need to store more information. As long as we cast it to a smaller or equal size, we should be OK. It will be necessary to interpose on ALL functions used to prohibit any other functions writing to this data.

The file 'pthreadtypes.h' (full path: /usr/include/x86\_64-linux-gnu/bits/pthreadtypes.h) has the sizes of pthread types (in bytes, verified with sizeof). Reusing this for condition variables might not be useful...

## Gathering the functions
We used ``nm -D -C db_bench`` to get a list of all dynamicly resolved functions that RocksDB uses in its benchmark. We then manually went through and removed the functions that had no obvious relation to concurency. We than also ran ``nm -C db_bench | grep KEYWORD``, where KEYWORD was a concurrency programming keyword, such as mutex, condition, lock, etc. The list of these functions can be found in 'methods\_full.txt'.

Note, db\_bench uses unique\_lock (and other funcs) that ARE NOT dynamically linked to the binary. Thus, we cannot interposition on these funcs.

We may want to look into the libraries that are linked in (with ``ldd``)

## BenchMarks
RocksDB provides us with multiple benchmarks to use. Such as filling, reading, writing, deleting. (See help.txt -benchmarks). For simple tests we have created a target to fill 100k keys in random order with and without interposition. We have also created a target to randomly read and write using N threads. I believe the weight is 90% Reads - 10% Writes. 

With Interposition BenchMark
```
$ make fill
$ make readrandomwriterandom
```

## Notes
### Compiling
- Git clone the repository
- Resolve the submodules ``git submodule update --init --recursive``
- Compile libjunction dependency:

```
$ cd junction
$ mkdir build
$ cd build
$ mkdir install # Where the headers and libs will be sent
$ cmake -DCMAKE_INSTALL_PREFIX=install -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true ..
$ make install
$ cd ../../
```

- Run ``make all`` from the rocksdb repository


If missing dependencies and don't have root access:
- Follow instructions from 'https://gist.github.com/achalddave/7f7323a36f85b6c6dd64'
- Compile RocksDB ``LDFLAGS="-L${HOME}/local/lib" CFLAGS="-I${HOME}/local/include" make``

### Using gdb
1. Run ``gdb target``.
1. In gdb, set up the environmnet variables:
    - ``set env LD_PRELOAD ${HOME}/RocksDB-TSX/libmypthread.so``

### Design considerations (Meeting with spear)
- Lemming problem -> good to talk about during presentation
- Capture all pthread mutex ops
    - CondVars? - Richard Yoo paper 2013, Supercomputing
        - One option, come back to this issue
        - Use semaphores as commiting, then wait on them, start a new transaction on wait
    - Lock elision
        - Deal with all locks as one, or each lock independently. Library for each
    - Identify the critical sections
        - Program traces, performance logging
    - Conflict reduction strats
        - Try again or grab lock
        - Hourglass... PhD student. Stop new transactions, dont kill current ones
- Challenge: performance predicability
- Difficult to log, causes aborts. May be hard to debug why a transaction is aborting
