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
$ cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_SHARED_LIBS=ON ..
$ make install
$ cd ../../
```

- Run ``make all`` from the rocksdb repository


- If missing dependencies:
    Follow instructions from 'https://gist.github.com/achalddave/7f7323a36f85b6c6dd64'
    - Compile RocksDB ``LDFLAGS="-L/home/adk216/local/lib" CFLAGS="-I/home/adk216/local/include" make``


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
