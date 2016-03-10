#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include "spinlock.h"

//using mutex_t = tas_spinlock;
using mutex_t = tatas_spinlock;

int main()
{
	std::vector <std::thread> workers;
	size_t variable = 0;
	size_t N = 10;
	size_t K = 10000000;
	mutex_t mtx;
	auto start_time = std::chrono::high_resolution_clock::now();
	
	for (size_t i = 0; i < N; ++i)
		workers.emplace_back([&]()
		{
			for (size_t j = 0; j < K; ++j)
			{
				std::lock_guard <mutex_t> lock(mtx);
				++variable;
			}
		});
	for (auto &i : workers)
		i.join();
	
	auto end_time = std::chrono::high_resolution_clock::now();
	auto time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	std::cout << variable << std::endl;
	std::cout << "elapsed: " << time_elapsed.count() << " microseconds" << std::endl;
	
}