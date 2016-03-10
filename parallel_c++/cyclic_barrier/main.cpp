#include "barrier.h"
#include "thread_joiners.h"
#include <vector>
#include <random>
#include <chrono>

using uint = unsigned int;

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

void test()
{
	random_uints generator;
	std::mutex mtx;
	cyclic_barrier c_bar(5);
	std::vector<std::thread> workers;
	join_threads joiner(workers);
	for (int i = 0; i < 10; ++i)
		workers.emplace_back([&generator, &c_bar, &mtx]()
		{
			std::this_thread::sleep_for(std::chrono::seconds(generator.next(10)));
			c_bar.enter();
			std::lock_guard<std::mutex> lock(mtx);
			std::cout << std::this_thread::get_id() << std::endl;
		});
}

void hard_test()
{
	random_uints generator;
	std::mutex mtx;
	cyclic_barrier c_bar(5);
	std::vector<std::thread> workers;
	join_threads joiner(workers);
	for (int i = 0; i < 10; ++i)
		workers.emplace_back([&generator, &c_bar, &mtx]()
		{
			for (int j = 0; j < 100000; ++j)
			{
				c_bar.enter();
				std::lock_guard<std::mutex> lock(mtx);
				std::cout << std::this_thread::get_id() << std::endl;
			}
		});
}

int main()
{
	test();
	hard_test();
}