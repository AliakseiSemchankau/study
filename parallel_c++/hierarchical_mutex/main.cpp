#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "hierarchical_mutex.h"
#include "thread_joiners.h"

void work()
{
	hierarchical_mutex high_level_mtx(1500);
	hierarchical_mutex low_level_mtx(100);	
	std::thread first([&]()
	{
		try
		{
			std::unique_lock<hierarchical_mutex> high_lock(high_level_mtx, std::defer_lock);
			std::unique_lock<hierarchical_mutex> low_lock(low_level_mtx, std::defer_lock);
			high_lock.lock();
			low_lock.lock();
			high_lock.unlock();
			low_lock.unlock();
		}
		catch (std::logic_error& exception)
		{
			std::cout << exception.what() << std::endl;
		}
	});
	std::thread second([&]()
	{
		try
		{
			std::lock_guard<hierarchical_mutex> low_lock(low_level_mtx);
			std::lock_guard<hierarchical_mutex> high_lock(high_level_mtx);
		}
		catch (std::logic_error& exception)
		{
			std::cout << exception.what() << std::endl;
		}
	});
	first.join();
	second.join();
}

int main()
{
	work();
	return 0;
}