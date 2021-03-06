CXX = g++
CFLAGS = -std=c++14 -Wall -g -O2 -mrtm
LDFLAGS = -lunwind -L/lib/x86_64-linux-gnu/liblzma.so.5 

PTHREAD_LIBNAME = libbt.so
TSX_LIBNAME = libtsx.so
SPIN_LIBNAME = libspin.so
LOG_FILE = mutex_usage.log

JUNCTION_DEPS = -I junction/build/install/include/
JUNCTION_LIBS = junction/build/install/lib/
JUNCTION_TURF_LD = -pthread -L $(JUNCTION_LIBS) -l junction -l turf

LD_STR = LD_PRELOAD="$(HOME)/RocksDB-TSX/$(PTHREAD_LIBNAME)"
TSX_LD = LD_PRELOAD="$(HOME)/RocksDB-TSX/$(TSX_LIBNAME)"
SPIN_LD = LD_PRELOAD="$(HOME)/RocksDB-TSX/$(SPIN_LIBNAME)"
DB_BENCH = ./rocksdb/db_bench

all: tsx_lib spin_lib pthread_lib 


pthread_test: pthread_test.cpp
	$(CXX) $(CFLAGS) $< -o pthread_test -lpthread

$(TSX_LIBNAME): spin_lock.hpp libtsx.cpp 
	$(CXX) -shared -fPIC $(CFLAGS) $^ -o $(TSX_LIBNAME) -pthread

$(SPIN_LIBNAME): spin_lock.hpp libspin.cpp 
	$(CXX) -shared -fPIC $(CFLAGS) $^ -o $(SPIN_LIBNAME) -pthread

$(PTHREAD_LIBNAME): spin_lock.hpp backtrace.hpp pthread_interpose.cpp 
	$(CXX) -shared -fPIC $(CFLAGS) -Wno-unused-value $^ -o $(PTHREAD_LIBNAME) -ldl $(LDFLAGS) $(JUNCTION_DEPS) $(JUNCTION_TURF_LD) 

junction_test: junction_test.cpp
	$(CXX) $(CFLAGS) -Wno-unused-value -Wno-unused-variable $< -o junction_test $(JUNCTION_DEPS) $(JUNCTION_TURF_LD) 


pthread_lib: $(PTHREAD_LIBNAME)
tsx_lib: $(TSX_LIBNAME)
spin_lib: $(SPIN_LIBNAME)

test: $(TSX_LIBNAME) pthread_test 
	$(TSX_LD) ./pthread_test

test_log: $(PTHREAD_LIBNAME) pthread_test 
	cat /dev/null > $(LOG_FILE)
	$(LD_STR) ./pthread_test
	cat $(LOG_FILE)

clean:
	-rm -f $(TSX_LIBNAME) $(PTHREAD_LIBNAME) pthread_test junction_test 2> /dev/null
