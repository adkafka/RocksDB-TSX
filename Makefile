CXX = g++
CFLAGS = -std=c++14 -Wall -g -O0
LDFLAGS = -lunwind -L/lib/x86_64-linux-gnu/liblzma.so.5 


all: backtrace

backtrace: backtrace.cpp
	g++ $< $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm backtrace
