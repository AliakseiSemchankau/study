#ifndef PREFIX_SUM
#define PREFIX_SUM

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <iostream>
#include "barrier.h"

template <typename T, class Add>
void parallel_scan(const std::vector<T>& data, Add add, std::vector<T>& prefix_sums, size_t num_threads)
{
	std::vector <T> buffer = data;
	prefix_sums = buffer;
	size_t size = data.size();
	size_t local_power = 1 << static_cast <int> (std::log2(size));
	if (local_power != size)
		for (size_t j = size; j < 2 * local_power; ++j)
			buffer.push_back(0);
	size = buffer.size();
	cyclic_barrier gates(num_threads);
	size_t length_of_work = size / num_threads;
	std::vector <std::thread> workers;

	auto work = [&](size_t left_limit, size_t right_limit)
	{
		for (size_t step = 0; step <  static_cast<size_t> (std::log2(size)); ++step)
		{
			gates.enter();
			size_t power = 1 << (step + 1);
			for (size_t k = 0; k < size; k += power)	
				if (k + power - 1 < size && k >= left_limit && k < right_limit)
					buffer[k + power - 1] = add(buffer[k + (power / 2) - 1], buffer[k + power - 1]);
		}
		buffer[size - 1] = 0;
		for (int step = static_cast<int> (std::log2(size)) - 1; step >= 0; --step)
		{
			gates.enter();
			size_t power = 1 << (step + 1);
			for (size_t k = 0; k < size ; k += power)
				if (k >= left_limit && k < right_limit  && (k + power / 2) - 1 < size && k + power - 1 < size)
				{
					size_t temp = buffer[k + (power / 2) - 1];
					buffer[k + (power / 2) - 1] = buffer[k + power - 1];
					buffer[k + power - 1] = add(buffer[k + power - 1], temp);
				}
		}
		gates.enter();
		size_t left = left_limit;
		if (1 > left)
			left = 1;
		size_t right = right_limit;
		if (data.size() < right)
			right = data.size();
		for (size_t i = left; i < right; ++i)
			prefix_sums[i - 1] = buffer[i];
	};
	
	for (size_t i = 0; i < num_threads - 1; ++i)
		workers.emplace_back(work, i * length_of_work, (i + 1) * length_of_work);
	
	workers.emplace_back(work, (num_threads - 1) * length_of_work, size);
	

	for (size_t i = 0; i < num_threads; ++i)
		workers[i].join();

	if (data.size() > 1)
		prefix_sums[data.size() - 1] += prefix_sums[data.size() - 2];
}

#endif