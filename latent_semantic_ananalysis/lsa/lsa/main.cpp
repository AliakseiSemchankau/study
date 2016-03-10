#include "lsa.h"
#include "svd.h"
#include <stdlib.h>
#include <cstdio>
#include <chrono>

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


int main()
{
	lsa example("start_document.txt");
	steady_timer timer;
	auto res = example.query(3, 3);
	auto work_time = timer.seconds_elapsed();
	std::cout << work_time << " seconds" << std::endl;
	for (auto &i : res)
		std::cout << i << " ";
	/*std::fstream input;
	
	input.open("input.txt");
	
	int n;
	input >> n;
	std::vector <std::vector<long double> > A(n);
	std::vector <long double> temp(n);
	for (int j = 0; j < n; ++j)
	{
		for (int i = 0; i < n; ++i)
		{
			long double value;
			input >> value;
			temp[i] = value;
		}
		A[j] = temp;
	}
	input.close();
	steady_timer timer;
	svd_approximation(A);
	std::cout << timer.seconds_elapsed() << std::endl;*/
}