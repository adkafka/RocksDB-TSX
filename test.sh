#!/bin/bash

# Run a simple test with rockdb db_bench
# Taken from their wiki: https://github.com/facebook/rocksdb/wiki/Performance-Benchmarks
# and scaled down to run in seconds

# Run normally:
# ./test.sh
# Run with tsx lib (libtsx.so)
# ./test.sh tsx
# Perform backtrace logging (libbt.so)
# ./test.sh bt

echo "Cleaning test directory (test/)"
rm -rf test/
#mkdir test

LOG_FILE="mutex_usage.log"


SHARED_PARAMS='\
    --disable_auto_compactions=true \
    --key_size=8 \
    --value_size=256 \
    --block_size=262144 \
    --db=./test \
    --compression_type=none \
    --compression_ratio=1 \
    --num_levels=4'

num=250000
echo "Load $num keys sequentially into database....."
./rocksdb/db_bench \
    --benchmarks=fillrandom \
    --num=$num \
    --threads=1 \
    --disable_wal=1 \
    --use_existing_db=0 \
    ${SHARED_PARAMS}
#echo "Copying database to test dir"
#cp -r test.2000000 test
num=500000
threads=${2:-1}
echo "Reading and writing randomly $num keys in database using $threads threads in random order...."
if [[ $1 == "tsx" ]]; then
    export "LD_PRELOAD=./libtsx.so"
elif [[ $1 == "bt" ]]; then
    cat /dev/null > ${LOG_FILE}
    export "LD_PRELOAD=./libbt.so"
elif [[ $1 == "spin" ]]; then
    export "LD_PRELOAD=./libspin.so"
elif [[ $1 == "perf" ]]; then
    # Record all transaction events for later analysis with 'report'
    PRE="perf record -g --transaction --weight -e cpu/tx-abort/pp "
    export "LD_PRELOAD=./libtsx.so"
elif [[ $1 == "report" ]]; then
    perf report --sort symbol,transaction,weight
elif [[ $1 == "rate" ]]; then
    # Give abort/start stats
    PRE="perf stat -e tx-abort,tx-start,tx-commit,tx-conflict,tx-capacity "
    export "LD_PRELOAD=./libtsx.so"
fi
${PRE} ./rocksdb/db_bench \
    --benchmarks=readwhilewriting \
    --num=$num \
    --threads=$threads \
    --disable_wal=1 \
    --use_existing_db=1 \
    ${SHARED_PARAMS}
