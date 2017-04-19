#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

bool ready(false);
std::mutex m;
std::condition_variable cv;
int i=0;
pthread_mutex_t mutex1;

void waits(){
	std::unique_lock<std::mutex> lk(m);
	std::cout << "Waiting.. \n";
	cv.wait(lk, []{return i==1;});
	std::cout << "...finished waiting. i==1\n";
	ready = true;
}

void signals(){
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "Notifying falsely \n";
	cv.notify_one();
	std::cout << "after notify\n";	
	std::unique_lock<std::mutex> lk(m);
	i=1;
	std::cout << "we are " << ready << std::endl;
	while(!ready){
		std::cout << "Notifying true change. \n";
		lk.unlock();
		cv.notify_one();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		lk.lock();
	}
}
void try_test(){
	
	std::cout << "locking..\n";
    pthread_mutex_lock(&mutex1);
	std::cout << "unlocking.. \n";
    pthread_mutex_unlock(&mutex1);
	std::cout <<"done\n";

}
int main(){
    std::cout << "Doing simple lock test\n";
    pthread_mutex_init(&mutex1,NULL);
    pthread_mutex_lock(&mutex1);
    pthread_mutex_unlock(&mutex1);
    std::cout << "Done with simple lock test\n";
	try_test();
	try_test();
	try_test();    
//std::cout << "Doing with simple condvar test\n";
	//std::thread t1(waits), t2(signals);
	//t1.join();
	//t2.join();
}
