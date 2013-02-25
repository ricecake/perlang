#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <mutex>

static std::mutex pIntLock;

int main() {
	pIntLock.lock();
	std::thread* p;
	for(int i=0;i<500;i++) {
		p = new std::thread([i]{
				std::cout<<"starting thread "<<i<<std::endl;
				pIntLock.lock();
		});
		p->detach();
	}
}
