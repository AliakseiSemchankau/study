#include "thread_safe_queue.h"
#include "thread_joiners.h"
#include "thread_pool.h"

#include <random>
#include <vector>
#include <string>
#include <iostream>

using uint = unsigned int;
const int NUM_OF_TASKS = 100;
const int UPPER_BOUND = 100000;
const uint MAX_NUM_OF_TASKS = 5;

bool prime_test(int number)
{
	for (uint i = 2; i <= static_cast<uint>(sqrt(number)) + 1; ++i)
		if (number % i == 0)
			return false;
	return true;
}

class random_uints
{
	std::mt19937 mt_engine_;
	std::uniform_int_distribution <uint> uniform_uints_;
public:
	random_uints()
		: mt_engine_(std::random_device()()) 
	{}

	uint next(const uint upper_bound)
	{
		return uniform_uints_(mt_engine_) % upper_bound;
	}
};

int main()
{
	thread_pool<bool> thr_pool(MAX_NUM_OF_TASKS);
	std::vector<std::pair < std::future<bool>, int> > results;
	random_uints generator;
	
	for (int i = 0; i < NUM_OF_TASKS; ++i)
	{
		auto number = generator.next(UPPER_BOUND);
		std::function<bool()> current_function = std::bind(prime_test, number);
		std::future<bool> future_result = thr_pool.submit(current_function);
		results.emplace_back(std::make_pair(std::move(future_result), number));
	}

	for (auto& i : results)
	{
		try
		{
			auto result = i.first.get();
			if (result)
				std::cout << i.second << " is prime" << std::endl;
			else
				std::cout << i.second << " is not prime" << std::endl;
		}
		catch (std::exception& fail)
		{
			std::cout << fail.what() << std::endl;
		}
	}
	return 0;
}
