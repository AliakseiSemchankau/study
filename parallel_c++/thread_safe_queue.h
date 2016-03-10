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
	std::atomic<bool> is_shutdown = false;
	std::mutex mtx;
	size_t max_size;
public:
	thread_safe_queue(size_t max_size_) : max_size(max_size_)
	{}
	bool take(T& item)
	{
		std::unique_lock<std::mutex> lock(mtx);
		queue_not_empty.wait(lock, [this](){return !items.empty() || is_shutdown; });
		if (!items.empty())
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
		queue_not_full.wait(lock, [this](){return items.size() < max_size || is_shutdown; });
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
		if (items.size() < max_size && !is_shutdown)
		{
			items.emplace(std::move(new_element));
			queue_not_empty.notify_one();
			return true;
		}
		return false;
	}

	void shutdown()
	{
		is_shutdown = true;
		queue_not_empty.notify_all();
	}
};

#endif