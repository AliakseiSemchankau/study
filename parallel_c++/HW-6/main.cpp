
#include <iostream>
#include <atomic>
#include <thread>
#include <future>
#include <cstdint>
#include <random>

#include "lock_free_queue.h"


#include <chrono>

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


class steady_timer {
public:
	steady_timer() {
		reset();
	}

	void reset() {
		start_ = std::chrono::steady_clock::now();
	}

	double seconds_elapsed() const {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast< std::chrono::duration<double> >(now - start_);
		return elapsed.count();
	}

private:
	std::chrono::steady_clock::time_point start_;
};


//////////////////////////////////////////////////////////////////////////

using int_t = uint64_t;
using ts_queue_t = lock_free_queue<int_t>;

//////////////////////////////////////////////////////////////////////////

bool try_pop_from_queue(ts_queue_t& q, int_t& e) {
	//return queue.pop(e);

	std::shared_ptr<int_t> popped;

	if (q.pop(popped))
	{
		e = *popped;
		return true;
	}
	else 
	{
		return false;
	}
}

void push_to_queue(ts_queue_t& q, int_t e) {
	q.push(e);
}

int_t pop_from_queue_while_not_empty(ts_queue_t& q) {
	int_t sum = 0;
	int_t e;
	while (try_pop_from_queue(q, e)) {
		sum += e;
	}
	return sum;
}

//////////////////////////////////////////////////////////////////////////

void pause(int_t e) {
	// 1) no pause
	// return;

	// 2) busy loop
	volatile size_t step = 0;
	while (step * step < e) {
		++step;
	}
}

void perf_test(const size_t num_producers, const size_t num_elements_to_produce, const size_t num_consumers) {
	std::cout << "start perf test..." << std::endl;
	ts_queue_t q;

	cyclic_barrier start_barrier(num_producers + num_consumers);
	std::atomic_size_t producers_done_cnt(0);

	steady_timer timer;

	std::vector<std::thread> producers;
	std::vector<std::future<int_t>> producers_results;
	for (size_t i = 0; i < num_producers; ++i) 
	{
		auto producer = [&](std::promise<int_t>& prms) 
		{
			start_barrier.enter();

			std::random_device random_dev;
			std::mt19937 mt_random_engine(random_dev());
			std::uniform_int_distribution<int_t> gen_random_int(1, 10);

			int_t produced_sum = 0;
			for (size_t k = 0; k < num_elements_to_produce; ++k) {
				auto e = gen_random_int(mt_random_engine);
				push_to_queue(q, e);
				produced_sum += e;
				pause(e);
			}

			producers_done_cnt.fetch_add(1);
			prms.set_value(produced_sum);
		};
		std::promise<int_t> prms;
		std::future<int_t> cur_fut = prms.get_future();
		producers_results.push_back(std::move(cur_fut));
		producers.emplace_back(producer, std::move(prms));
	}

	std::vector<std::thread> consumers;
	std::vector<std::future<int_t>> consumers_results;
	for (size_t i = 0; i < num_consumers; ++i)
	{
		auto consumer = [&](std::promise<int_t>& prms) 
		{
			start_barrier.enter();

			int_t consumed_sum = 0;

			do {
				consumed_sum += pop_from_queue_while_not_empty(q);
				std::this_thread::yield();
			} while (producers_done_cnt != num_producers);
			consumed_sum += pop_from_queue_while_not_empty(q);

			prms.set_value(consumed_sum);
		};
		std::promise<int_t> prms;
		std::future<int_t> cur_fut = prms.get_future();
		consumers_results.push_back(std::move(cur_fut));
		consumers.emplace_back(consumer, std::move(prms));
	}

	int_t produced_sum = 0;
	for (auto& f : producers_results) 
	{
		produced_sum += f.get();
	}

	int_t consumed_sum = 0;
	for (auto& f : consumers_results) 
	{
		consumed_sum += f.get();
	}
	for (auto &i : producers)
		i.join();
	for (auto &i : consumers)
		i.join();
	double elapsed = timer.seconds_elapsed();

	std::cout << "perf test: num producers = " << num_producers << ", num_consumers = " << num_consumers << std::endl;
	std::cout << "produced sum = " << produced_sum << std::endl;
	std::cout << "consumed sum = " << consumed_sum << std::endl;
	std::cout << "result: " << ((produced_sum == consumed_sum) ? "SUCCEEDED" : "FAILED") << std::endl;
	std::cout << "test takes " << elapsed << " seconds" << std::endl;
}

//////////////////////////////////////////////////////////////////////////

int main() {
	//perf_test(1, 10, 1);
	//perf_test(1, 10, 4);
	perf_test(10, 10000, 10);
	//perf_test(20, 100000, 20);

	return 0;
}
