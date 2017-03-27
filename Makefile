all: backtrace

backtrace: backtrace.cpp
	g++ -g backtrace.cpp -std=c++14 `pkg-config --static --libs libunwind` -o backtrace
