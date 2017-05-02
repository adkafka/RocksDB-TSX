#include <iostream>
#include <pthread.h>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <chrono>//seconds
#include <thread>//sleep_for

//#include "spin_lock.hpp"

#define CACHE_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_SIZE)))

/* COND VAR */

struct data{
    volatile bool ready;
    char pad1[CACHE_SIZE-sizeof(bool)];
    std::condition_variable cv;
    char pad2[CACHE_SIZE-sizeof(std::condition_variable)];
    std::mutex mtx;
    char pad3[CACHE_SIZE-sizeof(std::mutex)];
} CACHE_ALIGNED;

struct data d;

void print_id () {
    std::unique_lock<std::mutex> lck(d.mtx);
    while (!d.ready) d.cv.wait(lck);
    lck.unlock();
    std::cout << "thread says hello\n";
}

void go() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::unique_lock<std::mutex> lck(d.mtx);
    d.ready = true;
    d.cv.notify_all();
}


void condvar_test(){
    std::cout << "Doing with simple condvar test\n";
    int num_threads = 1;
    std::thread threads[num_threads];
    d.ready = false;
    // spawn num_threads threads:
    for (int i=0; i<num_threads; ++i)
        threads[i] = std::thread(print_id);
    std::cout << num_threads <<" threads ready to race...\n";
    go();                       // go!
    // Join the threads
    for (int i=0; i<num_threads; ++i)
        threads[i].join();
}

/* LOCK */

long count_val = 0;
pthread_mutex_t mutex;
const long count_per_thread = 1000000;

void count(int id){
    for(int i=0; i<count_per_thread; i++){
        pthread_mutex_lock(&mutex);
        count_val++;
        pthread_mutex_unlock(&mutex);
    }
}

void lock_test(){
    int num_threads = 4;
    std::cout << "Counting to " << count_per_thread << " with "<<num_threads<<" threads\n";
    std::thread threads[num_threads];
    pthread_mutex_init(&mutex,0);
    // spawn num_threads threads:
    for (int i=0; i<num_threads; ++i)
        threads[i] = std::thread(count,i);
    for (int i=0; i<num_threads; ++i)
        threads[i].join();

    std::cout << "Final value: "<< count_val << "\n";
}

/* RWLOCK */

pthread_rwlock_t rwlock;
double checking_balance = 100.0;
double saving_balance = 100.0;

void get_bal(int id){
    for(int i=0;i<10;i++){
        pthread_rwlock_rdlock(&rwlock);
        double chk = checking_balance;
        double sav = saving_balance;
        double bal = chk+sav;
        pthread_rwlock_unlock(&rwlock);
        std::cout << "Bal: "<<bal<<"; chk: "<<chk<<"; sav: "<<sav<<"\n";
    }
}

void transfer(int id){
    for(int i=0;i<10;i++){
        pthread_rwlock_wrlock(&rwlock);
        checking_balance = checking_balance-7;
        saving_balance = saving_balance+7;
        pthread_rwlock_unlock(&rwlock);
        std::cout << "Deducted 7\n";
    }
}

void rwlock_test(){
    pthread_rwlock_init(&rwlock,0);
    std::cout << "Deducting 7$ from our 100$ account. As long as sum adds to 100, we are consistent\n";
    std::thread threads[4];
    pthread_mutex_init(&mutex,0);
    // spawn 10 threads:
    for (int i=0; i<3; ++i)
        threads[i] = std::thread(get_bal,i);
    threads[3] = std::thread(transfer,3);
    for (int i=0; i<4; ++i)
        threads[i].join();
}


int main(){
    //std::cout << "int size: " << sizeof(int) << std::endl;
    //std::cout << "spin_lock size: " << sizeof(spin_lock) << std::endl;
    //std::cout << "pthread_mutex_t: " << sizeof(pthread_mutex_t) << std::endl;
    //lock_test();
    //rwlock_test();
    condvar_test();
}
