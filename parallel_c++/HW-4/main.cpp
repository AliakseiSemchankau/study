#include <iostream>
#include <thread>
#include <future>
#include <time.h>
#include "ring_buffer.h"

uint64_t producer_work_loop(spsc_ring_buffer <int>& argument)
{
	uint64_t result = 0;
	for (int i = 0; i < 10000000; ++i)
	{
		while (!argument.enqueue(i))
			std::this_thread::yield();
		result += i;
	}
	return result;
}

uint64_t consumer_work_loop(spsc_ring_buffer <int>& argument)
{
	uint64_t result = 0;
	for (int i = 0; i < 10000000; ++i)
	{
		int local_var;
		while (!argument.dequeue(local_var))
			std::this_thread::yield();
		result += local_var;
	}
	return result;
}


int main()
{
	clock_t tStart = clock();
	spsc_ring_buffer <int> channel(1024);
	std::future<uint64_t> consumed_sum = std::async(consumer_work_loop, std::ref(channel));
	std::future<uint64_t> produced_sum = std::async(producer_work_loop, std::ref(channel));
	auto consumed_res = consumed_sum.get();
	auto produced_res = produced_sum.get();
	std::cout << produced_res << std::endl;
	if (produced_res == consumed_res)
		std::cout << "OK" << std::endl;
	printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	return 0;
}