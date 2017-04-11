
CXX = g++
CC = gcc
CFLAGS = -std=c++14 -Wall -g -O0
LDFLAGS = -lunwind -L/lib/x86_64-linux-gnu/liblzma.so.5 
PTHREAD_LIBNAME = libmypthread.so


all: backtrace pthread_test

backtrace: backtrace.o 
	$(CXX) backtrace.o -o $@

backtrace.o: backtrace.cpp backtrace.h 
	$(CXX) $< $(CFLAGS) $(LDFLAGS) backtrace.cpp

pthread_test: pthread_test.o
	$(CC) -o pthread_test -lpthread

pthread_test.o: pthread_test.c
	$(CC) $< $(LDFLAGS) pthread_test.c

pthread_lib: pthread_interpose.c
	$(CC) -shared -fPIC $< -o $(PTHREAD_LIBNAME) -ldl

$(PTHREAD_LIBNAME): pthread_lib

log: $(PTHREAD_LIBNAME)
	LD_PRELOAD="$(HOME)/RocksDB-TSX/$(PTHREAD_LIBNAME)" ./rocksdb/db_bench \
			   --num_levels=6 --key_size=20 --prefix_size=20 \
			   --keys_per_prefix=0 --value_size=100 --cache_size=17179869184 \
			   --cache_numshardbits=6 --compression_type=none --compression_ratio=1 \
			   --min_level_to_compress=-1 --disable_seek_compaction=1 --hard_rate_limit=2 \
			   --write_buffer_size=134217728 --max_write_buffer_number=2 \
			   --level0_file_num_compaction_trigger=8 --target_file_size_base=134217728 \
			   --max_bytes_for_level_base=1073741824 --disable_wal=0 \
			   --sync=0 \
			   --verify_checksum=1 --delete_obsolete_files_period_micros=314572800 \
			   --max_background_compactions=4 --max_background_flushes=0 \
			   --level0_slowdown_writes_trigger=16 --level0_stop_writes_trigger=24 \
			   --statistics=0 --stats_per_interval=0 --stats_interval=1048576 --histogram=0 \
			   --use_plain_table=1 --open_files=-1 --mmap_read=1 --mmap_write=0 \
			   --memtablerep=prefix_hash --bloom_bits=10 --bloom_locality=1 \
			   --benchmarks=filluniquerandom --use_existing_db=0 --num=524288000 --threads=2

clean:
	rm backtrace $(PTHREAD_LIBNAME)
