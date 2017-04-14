#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

bool ready(false);
std::mutex m;
std::condition_variable cv;
int i=0;

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
	std::cout << "we are" << ready;
	while(!ready){
		std::cout << "Notifying true change. \n";
		lk.unlock();
		cv.notify_one();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		lk.lock();
	}
}

int main(){
	std::thread t1(waits), t2(signals);
	t1.join();
	t2.join();
}
