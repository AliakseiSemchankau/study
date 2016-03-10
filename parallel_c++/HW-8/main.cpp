#include "prefix_sum.h"
#include <iostream>

class steady_timer 
{
public:
	steady_timer()
	{
		reset();
	}

	void reset()
	{
		start_ = std::chrono::steady_clock::now();
	}

	double seconds_elapsed() const
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast< std::chrono::duration<double> >(now - start_);
		return elapsed.count();
	}

private:
	std::chrono::steady_clock::time_point start_;
};

class sum
{
public:
	size_t operator()(size_t a, size_t b)
	{
		return a + b;
	}
};

int main()
{
	int num_of_elements = 10000;
	std::vector <int> temp;
	for (int i = 0; i < num_of_elements; ++i)
		temp.push_back(i);
	std::vector <int> prefix_sums;
	steady_timer timer;
	parallel_scan(temp, sum(), prefix_sums, 3);
	double elapsed_time = timer.seconds_elapsed();
	std::cout << "elapsed time " << elapsed_time << std::endl;
	std::vector<int> result(num_of_elements);
	result[0] = temp[0];
	for (int i = 1; i < num_of_elements; ++i)
		result[i] = result[i - 1] + temp[i];
	for (int i = 0; i < num_of_elements; ++i)
		if (result[i] != prefix_sums[i])
			std::cout << "right answer: " << result[i] << ", my answer: " << prefix_sums[i] << std::endl;
}
