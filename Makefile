all: backtrace

backtrace: backtrace.cpp
	g++ -g backtrace.cpp -std=c++14 -lunwind -L/lib/x86_64-linux-gnu/liblzma.so.5 -o backtrace
