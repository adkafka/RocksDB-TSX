# Notes for CSE403 project, RocksDB+TSX
Adam Kafka and Ryan Santos

## Next steps
- Make graphs from backtrace info
- Replace usage of locks with naïve spin locks (what about condvars?) to get a useful worst-case data point (what if all transactions fail)
- Fix TSX high abort rate
- Fix condvars so they use onCommit to wake
- Run benchmark to test it and get performance results
- Tweak params (how many aborts until fall-back) to get more results

## Design of Lock Elision
### Locks
We will use TSX RTM to perform lock elision, using a spin lock as a fallback mechanism. The best option (to get a spin lock), is to use the pthread\_mutex\_t pointer itself. Reinterpert cast it to what we need (an atomic bool) so that we can use it as a spin lock. This will work as long the size of our spin lock is less than the size of pthread\_mutex\_t. 

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
We can treat RW locks the same as normal locks, because TM will catch any writes and cause reads (and other writes) to abort. RW locks are designed for mutliple readers, one writer. TM will place the memory in the corresponding read/write set and detect any issues via cache coherence.

### CondVars
The simplest (and still efficient) option for implementing condvars will be to use futexes to signal/broadcast/wait. The guide [here](https://www.remlab.net/op/futex-condvar.shtml) is sufficient. I think we will have to settle for the simple solution which technically has a bug, but it depends on exactly 2^32 signals with no wait to happen, to result in a signal is lost. We will not be experiencing 2^32 signals in the lifetime of out program, so I am happy with this solution. The author provides a fix, but it would involvemodified a shared state in the condition variable... On second thought this might be fine. I will come back to this later. We will need to implement some kind of onCommit handler, so that wakes can happen after the commit.

References:
- [Semaphores](http://www.csc.villanova.edu/~mdamian/threads/posixsem.html)
- [Transaction-Friendly CondVars](http://transact2014.cse.lehigh.edu/wang2.pdf)
- [libtmcondvar](https://github.com/mfs409/transmem/blob/master/libs/libtmcondvar/tmcondvar.cc)
- [libcondvar](https://github.com/mfs409/libcondvar)

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
$ cmake -DCMAKE_INSTALL_PREFIX=install ..
$ make install
$ cd ../../
```

- Rocks DB needs the following dependencies:
    - libgflags-dev
    - libsnappy-dev
- Run ``make all`` or ``make release`` from the rocksdb repository

- To compile our program:
    - We need the dependency libunwind-dev
    - ``make`` will compile both libraries (``make tsx_lib`` and ``make pthread_lib``)
    - ``make readrandomwriterandom_tsx`` Will run the random reads and writes benchmark with our tsx library.
        - This should be done after the database is loaded with ``make fill_true``

### Suggestion
- Use backtrace on other aborts for insight
- There is a bug in TSX... maybe it is tickling it
- TLB misses -> context switch -> context swtich
- Try sequential


### Design considerations (Meeting with spear)
- Lemming problem -> good to talk about during presentation
- Use of pthread\_mutex/rwlock/cond types are not great for TSX. They all have sizes < 64 bytes, which is the cache line size. So any modifications of memory within the cache line, may cause spurious aborts. We shoud look into the alignment constraints of these types to see if it is only varaibles after, or if variables before can also affect cache line. 
    - What if we make small modifications to rocksDB to increase their size and fit our alignemnt constraingts? I'm not sure how we could do this though.
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
