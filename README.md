# Notes for CSE403 project, RocksDB+TSX
Adam Kafka and Ryan Santos

## Introduction
We have created a couple libraries for use with intel's TSX library, to replace pthread locks with RTM transactions.

This project is highly portable. Our 3 dynamic libraries can be used as is with any application that uses locks. To get ideal performance, some tweaking may be required in libtsx.so.

For more information, the presentation slides are available here: [Google Slides Presentation](https://docs.google.com/presentation/d/1g61QMVv1QOMUe-jrhXDU3pf8SQeA2HtgfAkeeDFnHXc/edit?usp=sharing)

## Components
### Libtsx
Library that will perform lock elision of pthread lock types. Additionally, it uses futexes to implement TSX safe condition variables (including defering the system call when in a transaction). See 'libtsx.cpp' for source.

### Libpsin
This is Libtsx, but instead of perfroming lock elision with TSX, it uses a spin lock every time. This is a good 'worst case' situation for libtsx. See 'libspin.cpp' for source.

### Libbt
This library performs a backtrace on interposed synchronization methods, and logs it to a file. This library imposes a signficant overhead, due to its dependency on addr2line. We cache these results using the Junction concurrent map, but there is still a noticable slowdown. Part of this is due to the logging. See 'pthread\_interpose.cpp and backtrace.hpp' for souce.


## Compilation
- Git clone the repository
- Resolve the submodules ``git submodule update --init --recursive``
### Dependencies
#### Compile libjunction dependency (this is only needed for the backtrace library):

```
$ cd junction
$ mkdir build
$ cd build
$ mkdir install # Where the headers and libs will be sent
$ cmake -DCMAKE_INSTALL_PREFIX=install ..
$ make install
$ cd ../../
```
#### Compile rocksDB:
- Rocks DB needs the following dependencies, installed with a package manager (before compilation):
    - libgflags-dev
    - libsnappy-dev (for compression)
```
$ cd rockdb
$ make release # Compile in release mode. Use 'all' if you want debug symbols.
```

### Our Libraries
- Compile all three libraries by simply executing ``make``. This will compile:
    - libtsx.so - Replace pthread locks, rwlock, condvars with TSX compliant code
    - libspin.so - Same as libtsx, but only use spin locks 
    - libbt.so - Produce a backtrace of pthread methods, rwlocks, and C/C++ cond vars
- These libraries can be compiled individually respectively with:
    - ``make tsx_lib``
    - ``make spin_lib``
    - ``make pthread_lib``

### Included Scripts
#### test.sh
This runs a benchmark on rocks db with the given parameters. The parameters are meant to be specified in the file for flexibility. This script is run with the following parameters:

``./test.sh [function] [num_threads=1]``

Possible functions are:
- tsx: Interpose with libtsx.so
- bt: Log the usage of synchronization methods with libbt.so
- spin: Interpose with libspin.so
- perf: Create a performance log using the unix command ``perf``
- report: View the report from 'perf'
- rate: Display the abort, start, commit, conflict, capacity results
- Any other string will result in normal execution of the benchmark

num\_threads is only used when a benchmark is run, and defaults to 1

#### all\_tests.sh
This is used to produce the graphs in the presentation. Useful if you would like to rerun our tests under different conditions.

#### analyze\_backtrace.sh
Uses bash tools to display simple statistics about a backtrace:
- Count of each pthread method called
- Number of unique lock addresses
- Top 20 contended locks

## Details
### Gathering the functions
We used ``nm -D -C db_bench`` to get a list of all dynamicly resolved functions that RocksDB uses in its benchmark. We then manually went through and removed the functions that had no obvious relation to concurency. We than also ran ``nm -C db_bench | grep KEYWORD``, where KEYWORD was a concurrency programming keyword, such as mutex, condition, lock, etc. The list of these functions can be found in 'methods\_full.txt'.

Note, db\_bench uses unique\_lock (and other funcs) that ARE NOT dynamically linked to the binary. Thus, we cannot interposition on these funcs. However, it seems that they are just wrappers around pthread methods.

We may want to look into the libraries that are linked in (with ``ldd``) for completeness.
