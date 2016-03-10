#ifndef THREAD_POOL
#define THREAD_POOL

#include "thread_safe_queue.h"
#include "thread_joiners.h"
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>

const size_t DEFAULT_NUM_OF_WORKERS = 8;

template <typename result_type>
class thread_pool
{
	using task_function = std::function<result_type()>;
	using safe_prms = std::unique_ptr<std::promise<result_type> >;
	using task = std::pair<task_function, safe_prms>;
	using async_result = std::future<result_type>;

	thread_safe_queue<task> tasks;
	std::vector<std::thread> workers;
	join_threads joiner;
	std::atomic<bool> done;
	size_t num_of_workers;

public:
	thread_pool(int max_num_of_tasks_, size_t num_of_workers_) : joiner(workers), tasks(max_num_of_tasks_), num_of_workers(num_of_workers_)
	{
		done.store(false);
		for (size_t i = 0; i < num_of_workers; ++i)
			workers.emplace_back(&thread_pool::worker, this);
	}
	
	thread_pool(int max_num_of_tasks_) : joiner(workers), tasks(max_num_of_tasks_), num_of_workers(default_num_of_workers())
	{
		done.store(false);
		for (size_t i = 0; i < num_of_workers; ++i)
			workers.emplace_back(&thread_pool::worker, this);
	}

	thread_pool() : joiner(workers), tasks(), num_of_workers(default_num_of_workers())
	{
		done.store(false);
		for (size_t i = 0; i < num_of_workers; ++i)
			workers.emplace_back(&thread_pool::worker, this);
	}

	size_t default_num_of_workers()
	{
		size_t num_of_workers_ = std::thread::hardware_concurrency();
		if (num_of_workers_ == 0)
			return DEFAULT_NUM_OF_WORKERS;
		else 
			return num_of_workers_;
	}

	void active_wait(async_result& cur_result)
	{
		while (cur_result.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		{
			task current_task;
			try
			{
				if (tasks.try_take(current_task))
				{
					auto promise_ptr = std::move(current_task.second);
					try
					{
						promise_ptr->set_value(current_task.first());
					}
					catch (...)
					{
						promise_ptr->set_exception(std::current_exception());
					}
				}
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
	}

	void worker()
	{
		while (!done.load())
		{
			task current_task;
			try
			{
				if (tasks.take(current_task))
				{
					auto promise_ptr = std::move(current_task.second);
					try
					{
						promise_ptr->set_value(current_task.first());
					}
					catch (...)
					{
						promise_ptr->set_exception(std::current_exception());
					}
				}
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
	}

	thread_pool(const thread_pool&) = delete;
	thread_pool& operator=(const thread_pool&) = delete;

	void shutdown()
	{
		done.store(true);
		tasks.shutdown();
	}

	~thread_pool()
	{
		shutdown();
	}

	bool try_submit(task_function current_function, async_result& result)
	{

		safe_prms current_prms(new std::promise<result_type>);
		try
		{
			if (tasks.try_put(move(std::make_pair(current_function, std::move(current_prms)))))
			{
				result = current_prms->get_future();
				return true;
			}
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}
	
	async_result submit(task_function current_function)
	{
		safe_prms current_prms(new std::promise<result_type>);
		async_result result = current_prms->get_future();
		try
		{
			tasks.put(move(std::make_pair(current_function, std::move(current_prms))));
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return result;
	}

};

#endif