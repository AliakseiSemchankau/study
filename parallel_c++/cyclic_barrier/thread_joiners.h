#ifndef THREAD_JOIN
#define THREAD_JOIN
#include <thread>
#include <vector>

class thread_guard
{
	std::thread& thread;
public:
	thread_guard(std::thread& current_thread) :
		thread(current_thread)
	{}

	thread_guard(const thread_guard&) = delete;
	thread_guard& operator=(const thread_guard&) = delete;

	~thread_guard()
	{
		if (thread.joinable())
			thread.join();
	}
};

class join_threads
{
	std::vector <std::thread>& threads;
public:
	join_threads(std::vector<std::thread>& threads_) :
		threads(threads_)
	{}


	join_threads(const join_threads&) = delete;
	join_threads& operator=(const join_threads&) = delete;

	~join_threads()
	{
		for (auto& i : threads)
		{
			if (i.joinable())
				i.join();
		}
	}
};

#endif