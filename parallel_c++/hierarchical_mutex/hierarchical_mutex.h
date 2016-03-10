#ifndef HIERARCHICAL_MUTEX
#define HIERARCHICAL_MUTEX

#include <iostream>
#include <thread>
#include <mutex>
#include <exception>

using uint = unsigned int;

class hierarchical_mutex : public std::mutex
{
	static __declspec(thread) uint current_thread_hierarchical_value;
	uint previous_hierarchical_value;
	const uint hierarchical_value;
	std::mutex mtx;
public:
	hierarchical_mutex(uint value) : hierarchical_value(value), previous_hierarchical_value(0)
	{}

	void check_hierarchical_value()
	{
		if (current_thread_hierarchical_value <= hierarchical_value)
			throw(std::logic_error("wrong hierarchy value"));
	}

	void update_hierarchical_value()
	{
		previous_hierarchical_value = current_thread_hierarchical_value;
		current_thread_hierarchical_value = hierarchical_value;
	}

	void lock()
	{
		check_hierarchical_value();
		mtx.lock();
		update_hierarchical_value();
	}

	void unlock()
	{
		current_thread_hierarchical_value = previous_hierarchical_value;
		mtx.unlock();
	}

	bool try_lock()
	{
		check_hierarchical_value();
		if (mtx.try_lock())
		{
			update_hierarchical_value();
			return true;
		}
		return false;
	}
};

_declspec(thread) uint hierarchical_mutex::current_thread_hierarchical_value(INT_MAX);
#endif