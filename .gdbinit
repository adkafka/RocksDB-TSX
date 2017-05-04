set env LD_PRELOAD ./libspin.so
file ./rocksdb/db_bench 

set breakpoint pending on
#b futex_signal
#b futex_broadcast
#b FuncArg::run

r --benchmarks=readwhilewriting --num=1000 --threads=1 --disable_wal=0 --use_existing_db=1  -db ./test/
