#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>

//#include <pthread.h>
//#include "spin_lock.hpp"

bool ready(false);
std::condition_variable cv;
std::mutex mtx;

void print_id (int id) {
    std::unique_lock<std::mutex> lck(mtx);
    while (!ready) cv.wait(lck);
    std::cout << "thread " << id << '\n';
}

void go() {
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all();
}


int main(){
    //std::cout << "spin_lock size: " << sizeof(spin_lock) << std::endl;
    //std::cout << "pthread_mutex_t: " << sizeof(pthread_mutex_t) << std::endl;
    std::cout << "Doing with simple condvar test\n";
    std::thread threads[10];
    // spawn 10 threads:
    for (int i=0; i<10; ++i)
        threads[i] = std::thread(print_id,i);
    std::cout << "10 threads ready to race...\n";
    go();                       // go!
    // Join the threads
    for (int i=0; i<10; ++i)
        threads[i].join();
    
    return 0;
}
