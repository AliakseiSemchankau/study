#ifndef SPINLOCK
#define SPINLOCK

#include <atomic>
#include <thread>

class tatas_spinlock 
{
	std::atomic<bool> locked;
public:
	void lock()
	{
		size_t counter = 0;
		while (true)
		{
			while (locked.load(std::memory_order_relaxed))
				std::this_thread::yield();
			if (!locked.exchange(true, std::memory_order_acquire))
				return;
			std::this_thread::yield();
		}
	}

	bool try_lock() 
	{
		return !locked.exchange(true, std::memory_order_acquire);
	}

	void unlock() 
	{
		locked.store(false, std::memory_order_release);
	}
};

class tas_spinlock
{
	std::atomic<bool> locked;
public:
	void lock()
	{
		while (locked.exchange(true, std::memory_order_acquire))
			std::this_thread::yield();
	}

	bool try_lock()
	{
		return !locked.exchange(true, std::memory_order_acquire);
	}

	void unlock()
	{
		locked.store(false, std::memory_order_release);
	}
};

#endif