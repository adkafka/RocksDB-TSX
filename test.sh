#!/bin/bash

# Run a simple test with rockdb db_bench
# Taken from their wiki: https://github.com/facebook/rocksdb/wiki/Performance-Benchmarks
# and scaled down to run in seconds

# Run normally:
# ./test.sh
# Run with tsx lib (libtsx.so)
# ./test.sh tsx
# Perform backtrace logging (libmypthread.so)
# ./test.sh bt

echo "Cleaning test directory (test/)"
rm -rf test/
mkdir test

LOG_FILE="mutex_usage.log"


SHARED_PARAMS='\
    --disable_seek_compaction=1 \
    --mmap_read=0 \
    --key_size=16 \
    --value_size=800 \
    --block_size=65536 \
    --cache_size=1048576 \
    --bloom_bits=10 \
    --cache_numshardbits=6 \
    --open_files=200 \
    --verify_checksum=1 \
    --db=./test \
    --sync=0 \
    --compression_type=none \
    --compression_ratio=1 \
    --write_buffer_size=100000 \
    --target_file_size_base=100000 \
    --max_write_buffer_number=10 \
    --max_background_compactions=3 \
    --level0_file_num_compaction_trigger=1000 \
    --level0_slowdown_writes_trigger=1000 \
    --level0_stop_writes_trigger=1000 \
    --num_levels=2 \
    --delete_obsolete_files_period_micros=300000 \
    --min_level_to_compress=2 \
    --max_bytes_for_level_base=10485760'

num=100000
echo "Load $num keys sequentially into database....."
./rocksdb/db_bench \
    --benchmarks=fillseq \
    --num=$num \
    --threads=1 \
    --disable_wal=1 \
    --use_existing_db=0 \
    ${SHARED_PARAMS}

num=100000
threads=4
echo "Reading while writing $num keys in database using $threads threads in random order...."
if [[ $1 == "tsx" ]]; then
    export "LD_PRELOAD=./libtsx.so"
elif [[ $1 == "bt" ]]; then
    cat /dev/null > ${LOG_FILE}
    export "LD_PRELOAD=./libmypthread.so"
elif [[ $1 == "perf" ]]; then
    #PRE="perf record -e cpu/tx-abort/pp "
    PRE="perf record -g --transaction --weight -e cpu/tx-abort/pp "
    export "LD_PRELOAD=./libtsx.so"
fi
${PRE} ./rocksdb/db_bench \
    --benchmarks=readwhilewriting \
    --num=$num \
    --threads=$threads \
    --disable_wal=0 \
    --use_existing_db=1 \
    ${SHARED_PARAMS}
