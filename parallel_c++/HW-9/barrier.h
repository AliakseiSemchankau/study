#ifndef BARRIER
#define BARRIER

#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <atomic>

class cyclic_barrier
{
	std::mutex mtx;
	std::condition_variable barrier_not_come;
	std::condition_variable barrier_not_gone;
	std::atomic<int> number_in_front_of_barrier = 0;
	std::atomic<int> number_of_gone_barrier = 0;
	size_t limit_of_barrier;
public:
	void enter()
	{
		std::unique_lock<std::mutex> lock(mtx);
		barrier_not_come.wait(lock, [this]() {return number_of_gone_barrier == 0; });
		++number_in_front_of_barrier;
		barrier_not_gone.notify_all();
		barrier_not_gone.wait(lock, [this]() {return number_in_front_of_barrier == limit_of_barrier; });
		++number_of_gone_barrier;
		if (number_of_gone_barrier == limit_of_barrier)
		{
			number_of_gone_barrier = 0;
			number_in_front_of_barrier = 0;
			barrier_not_come.notify_all();
		}
	}
	cyclic_barrier(size_t limit_) : limit_of_barrier(limit_)
	{}
};

#endif