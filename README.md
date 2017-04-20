# Notes for CSE403 project, RocksDB+TSX
Adam Kafka and Ryan Santos

## Next steps
- Interposition on pthread library
    - At first to get an idea of where locks are being used (output a backtrace every time)
    - Use this information to determine how to procede... continue interposition, or fork code.
    - Can also use interpositioning library to test and verify our TSX with a simple program that uses locks
    - https://stackoverflow.com/questions/3707358/get-all-the-thread-id-created-with-pthread-created-within-an-process/3709027#3709027
    - Open up log file, and wrap pthread\_create such that we log every call with where it came from

- Run examples in rockDB and learn from lock usage. Maybe lock success/failures as well...

## Gathering the functions
We used ``nm -D -C db_bench`` to get a list of all dynamicly resolved functions that RocksDB uses in its benchmark. We then manually went through and removed the functions that had no obvious relation to concurency. We than also ran ``nm -C db_bench | grep KEYWORD``, where KEYWORD was a concurrency programming keyword, such as mutex, condition, lock, etc. The list of these functions can be found in 'methods\_full.txt'.

Note, db\_bench uses unique\_lock (and other funcs) that ARE NOT dynamically linked to the binary. Thus, we cannot interposition on these funcs.

## Design of Lock Elision
### Locks
We will use TSX RTM to perform lock elision, using a spin lock. I think the best option (to perform the spin lock), is to use the pthread\_mutex\_t struct itself... Reinterpert cast it to what we need (really just an int) so that we can use it as a spin lock. Another option would be to store an in memory map of pthread\_mutex\_t to spin lock... but this has many downsides...

References for this solution include:
- [tsx-tools](https://github.com/andikleen/tsx-tools/blob/master/locks/spin-rtm.c)
- [glibc](https://lwn.net/Articles/534758/)
- [acm](https://queue.acm.org/detail.cfm?id=2579227)
- [gcc](https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/X86-transactional-memory-intrinsics.html#X86-transactional-memory-intrinsics)
- [intel ppt](http://www.halobates.de/adding-lock-elision-to-linux.pdf)
- [utoronto ppt](http://individual.utoronto.ca/mikedaiwang/tm/Intel_TSX_Overview.pdf)
- [intel handbook](https://www-ssl.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf)

### RWLocks
I believe we can treat RW locks the same as normal locks, because TM will catch any issues. RW locks are designed for mutliple readers, one writer. TM will place the memory in the corresponding read/write set and detect any issues via cache coherence.

### CondVars
Based off [Richard Yoo paper](http://pages.cs.wisc.edu/~rajwar/papers/SC13_TSX.pdf). Uses Linux futex to atomically put self on wait list. Signal thread registers a callback if it signals. Thread will execute callback to update futex. BusyWait did the best in the paper, so we should impliment that if we can.


## Notes
### Compiling on 4pac1
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


- If missing dependencies:
    Follow instructions from 'https://gist.github.com/achalddave/7f7323a36f85b6c6dd64'
    - Compile RocksDB ``LDFLAGS="-L/home/adk216/local/lib" CFLAGS="-I/home/adk216/local/include" make``

### Using gdb
1. Run ``gdb target``.
1. In gdb, set up the environmnet variables:
    - ``set env LD_PRELOAD ${HOME}/RocksDB-TSX/libmypthread.so``

### Design considerations (Meeting with spear)
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
