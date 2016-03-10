#ifndef GAUSS
#define GAUSS

#define EPS 1e-9

#include <vector>
#include <algorithm>
#include <atomic>
#include "thread_joiners.h"
#include "barrier.h" 

class gauss_method
{
	size_t num_of_threads;
	std::vector<std::vector<long double>>& matrix;
	std::vector<long double>& column_of_system;
	bool answer;
	cyclic_barrier gates;
	std::vector<std::atomic<size_t>> pivot_indexes;
	std::atomic<size_t> pivot;

	void swap_rows(size_t first, size_t second, size_t num_of_thread)
	{
		size_t num_of_columns = matrix.size();
		size_t size_of_task = num_of_threads / num_of_columns;
		if (num_of_thread < num_of_threads - 1)
			for (size_t i = num_of_thread * size_of_task; i < (num_of_thread + 1) * size_of_task; ++i)
				std::swap(matrix[first][i], matrix[second][i]);
		else
		{
			for (size_t i = num_of_thread * size_of_task; i <= matrix.size(); ++i)
				std::swap(matrix[first][i], matrix[second][i]);
		}
	}

	size_t find_pivot(size_t num_of_column, size_t num_of_thread)
	{
		long double max_value;
		max_value = abs(matrix[num_of_column][num_of_column]);
		size_t num_of_string = num_of_column;
		for (size_t i = num_of_thread; i < matrix.size(); i += num_of_threads)
		{
			if (i > num_of_column)
			{
				long double cur_value = abs(matrix[i][num_of_column]);
				if (cur_value > max_value)
				{
					max_value = cur_value;
					num_of_string = i;
				}
			}
		}
		return num_of_string;
	}

	size_t normalize_string(size_t num_of_string, size_t num_of_thread, long double divider)
	{
		size_t string_thread = -1;
		for (size_t i = num_of_thread; i <= matrix.size(); i += num_of_threads)
			if (i != num_of_string)
				matrix[num_of_string][i] /= divider;
			else
				string_thread = num_of_thread;
		return string_thread;
	}

	void reduce_rows(size_t main_string, size_t num_of_thread, bool back)
	{
		if (!back)
		{
			for (size_t i = num_of_thread; i < matrix.size(); i += num_of_threads)
				if (i > main_string)
				{
					long double multiplier = matrix[i][main_string];
					for (size_t j = main_string; j <= matrix.size(); ++j)
						matrix[i][j] -= matrix[main_string][j] * multiplier;
				}
		}
		else
		{
			for (size_t i = num_of_thread; i < main_string; i += num_of_threads)
			{
				long double multiplier = matrix[i][main_string];
				for (size_t j = main_string; j <= matrix.size(); ++j)
					matrix[i][j] -= matrix[main_string][j] * multiplier;
			}
		}
	}

	bool task(size_t num_of_thread)
	{
		for (size_t num_of_column = 0; num_of_column < matrix.size(); ++num_of_column)
		{
			pivot_indexes[num_of_thread].store(find_pivot(num_of_column, num_of_thread));
			gates.enter();
			if (num_of_thread == 0)
			{
				long double max_value = abs(matrix[pivot_indexes[0]][num_of_column]);
				pivot.store(pivot_indexes[0]);
				for (auto &j : pivot_indexes)
					if (abs(matrix[j.load()][num_of_column]) > max_value)
					{
						max_value = abs(matrix[j.load()][num_of_column]);
						pivot.store(j.load());
					}
			}
			gates.enter();
			if (abs(matrix[pivot][num_of_column]) < EPS || !answer)
			{
				if (abs(matrix[num_of_column][matrix.size()]) < EPS)
					continue;
				else
				{
					answer = false;
					return false;
				}
			}
			if (num_of_column != pivot.load())
				swap_rows(num_of_column, pivot.load(), num_of_thread);
			gates.enter();
			size_t thread_column = normalize_string(num_of_column, num_of_thread, matrix[num_of_column][num_of_column]);
			gates.enter();
			if (thread_column == num_of_thread)
				matrix[num_of_column][num_of_column] = 1;
			reduce_rows(num_of_column, num_of_thread, false);

			//gates.enter();
		}

		for (int num_of_column = matrix.size() - 1; num_of_column >= 0; --num_of_column)
		{
			reduce_rows(num_of_column, num_of_thread, true);
			gates.enter();
		}
		return true;
	}

public:
	gauss_method(std::vector <std::vector <long double> >& matrix_, std::vector <long double>& column_of_system_, size_t num_of_threads_) :
		matrix(matrix_), num_of_threads(num_of_threads_), column_of_system(column_of_system_), answer(true), gates(num_of_threads_), pivot_indexes(num_of_threads)
	{
		for (size_t i = 0; i < matrix.size(); ++i)
			matrix[i].push_back(column_of_system[i]);
		
		for (auto &i : pivot_indexes)
			i.store(0);

		cyclic_barrier gates(num_of_threads);
		pivot.store(0);

		std::vector <std::thread> workers;
		for (size_t i = 0; i < num_of_threads; ++i)
			workers.emplace_back(&gauss_method::task, this, i);
		for (auto &i : workers)
			i.join();
	}

	bool get_solution(std::vector<long double>& sollution)
	{
		if (answer)
		{
			for (size_t i = 0; i < matrix.size(); ++i)
				sollution[i] = matrix[i][matrix.size()];
			return true;
		}
		return false;
	}

};

#endif 