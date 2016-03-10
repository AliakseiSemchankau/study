#include "gauss.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <iomanip>

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
	std::fstream input;
	std::fstream output;
	input.open("input.txt");
	output.open("output.txt");
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
	for (int i = 0; i < n; ++i)
	{
		long double value;
		input >> value;
		temp[i] = value;
	}
	input.close();
	steady_timer timer;
	gauss_method resolve(A, temp, 8);
	auto work_time = timer.seconds_elapsed();
	bool answer = resolve.get_solution(temp);
	if (answer)
	{
		for (auto &i : temp)
			output << std::fixed << std::setprecision(9) << i << " ";
		output << std::endl;
	}
	else
		output << "no solution" << std::endl;
	output << "work time " << work_time << std::endl;
	output.close();
}