#ifndef SAFE_THREAD_QUEUE 
#define SAFE_THREAD_QUEUE

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

template <typename T>
class thread_safe_queue
{
	std::queue<T> items;
	std::condition_variable queue_not_empty;
	std::condition_variable queue_not_full;
	std::atomic<bool> is_shutdown;
	std::mutex mtx;
	size_t max_size;
public:
	thread_safe_queue(size_t max_size_) : max_size(max_size_)
	{
		is_shutdown.store(false);
	}

	thread_safe_queue() : max_size(INT64_MAX)
	{
		is_shutdown.store(false);
	}

	bool try_take(T& item)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (!items.empty() && !is_shutdown.load())
		{
			item = std::move(items.front());
			items.pop();
			queue_not_full.notify_one();
			return true;
		}
		return false;
	}

	bool take(T& item)
	{
		std::unique_lock<std::mutex> lock(mtx);
		queue_not_empty.wait(lock, [this](){return !items.empty() || is_shutdown.load(); });
		if (!items.empty() && !is_shutdown.load())
		{
			item = std::move(items.front());
			items.pop();
			queue_not_full.notify_one();
			return true;
		}
		return false;
	}

	bool put(T&& new_element)
	{
		std::unique_lock<std::mutex> lock(mtx);
		queue_not_full.wait(lock, [this](){return items.size() < max_size || is_shutdown.load(); });
		if (is_shutdown.load())
			throw(std::logic_error("empty queue error"));
		if (items.size() < max_size)
		{
			items.emplace(std::move(new_element));
			queue_not_empty.notify_one();
			return true;
		}
		return false;
	}

	bool try_put(T&& new_element)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (is_shutdown.load())
			throw(std::logic_error("empty queue error"));
		if (items.size() < max_size)
		{
			items.emplace(std::move(new_element));
			queue_not_empty.notify_one();
			return true;
		}

		return false;
	}

	void shutdown()
	{
		std::unique_lock<std::mutex> lock(mtx);
		is_shutdown.store(true);
		queue_not_empty.notify_all();
	}
};

#endif