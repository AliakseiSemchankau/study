#ifndef BARRIER
#define BARRIER

#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <atomic>

class cyclic_barrier
{
	std::atomic<int> number_in_front_of_barrier;
	std::atomic<bool> gone;
	size_t limit_of_barrier;
public:
	void enter()
	{
		while (gone.load())
		{
		}
		number_in_front_of_barrier.fetch_add(1);
		while (number_in_front_of_barrier.load() < limit_of_barrier && !gone.load())
		{
			std::this_thread::yield();
		}
		if (!gone.load())
			gone.store(true);
		number_in_front_of_barrier.fetch_sub(1);
		while (number_in_front_of_barrier.load() > 0 && gone.load())
		{
			std::this_thread::yield();
		}
		if (gone.load())
			gone.store(false);
	}
	cyclic_barrier(size_t limit_) : limit_of_barrier(limit_)
	{
		gone.store(false);
		number_in_front_of_barrier.store(0);
	}
};

#endif