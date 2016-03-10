#include "thread_pool.h"
#include <algorithm>

template <typename T>
class merge_task
{
	std::vector <T>& data;
	thread_pool <bool>& thr_pool;
	typename std::vector <T>::iterator first_left, first_right, second_left, second_right, buffer;
public:
	merge_task(std::vector <T>& data_, typename std::vector <T>::iterator buffer_, typename std::vector <T>::iterator first_left_,
		typename std::vector <T>::iterator first_right_, typename std::vector <T>::iterator second_left_, typename std::vector <T>::iterator second_right_,
		thread_pool <bool>& thr_pool_) :
		data(data_), first_left(first_left_), first_right(first_right_), second_left(second_left_),
		second_right(second_right_), thr_pool(thr_pool_), buffer(buffer_)
	{}
	bool operator ()()
	{
		const size_t MERGE_CUT_OFF = 2000;
		auto res = static_cast <size_t>(second_right - first_left);

		if (static_cast <size_t> (first_right - first_left) + static_cast <size_t> (second_right - second_left) <= MERGE_CUT_OFF)
		{
			std::merge(first_left, first_right, second_left, second_right, buffer);
		}
		else
		{
			typename std::vector <T>::iterator first_middle;
			typename std::vector <T>::iterator second_middle;
			if (first_right - first_left < second_right - second_left)
			{
				second_middle = second_left + (second_right - second_left) / 2;
				first_middle = std::upper_bound(first_left, first_right, *second_middle);
			}
			else
			{
				first_middle = first_left + (first_right - first_left) / 2;
				second_middle = std::lower_bound(second_left, second_right, *first_middle);
			}
			typename std::vector <T>::iterator new_buffer = buffer + (first_middle - first_left) + (second_middle - second_left);
			merge_task<T> merge_left_part(data, buffer, first_left, first_middle, second_left, second_middle, thr_pool);
			merge_task<T> merge_right_part(data, new_buffer, first_middle, first_right, second_middle, second_right, thr_pool);
			auto left_part_merged = thr_pool.submit(merge_left_part);
			merge_right_part();
			thr_pool.active_wait(left_part_merged);
			//left_part_merged.get();
		}
		return true;
	}
};

template <typename T>
class sort_task
{
	std::vector <T>& data;
	thread_pool <bool>& thr_pool;
	typename std::vector<T>::iterator left, right, buffer;
	bool inplace;
public:
	sort_task(std::vector <T>& data_, typename std::vector<T>::iterator left_, typename std::vector<T>::iterator right_,
		typename std::vector<T>::iterator buffer_, thread_pool <bool>& thr_pool_, bool inplace_) :
		data(data_), left(left_), right(right_), thr_pool(thr_pool_), buffer(buffer_), inplace(inplace_)
	{}
	bool operator ()()
	{
		const size_t SORT_CUT_OFF = 500;
		if (static_cast <size_t> (right - left) <= SORT_CUT_OFF)
		{
			std::stable_sort(left, right);
			if (!inplace)
				std::move(left, right, buffer);
		}
		else
		{
			typename std::vector <T>::iterator middle = left + (right - left) / 2;
			typename std::vector <T>::iterator buffer_left = buffer + (middle - left);
			typename std::vector <T>::iterator buffer_right = buffer + (right - left);
			sort_task<T> sort_left_part(data, left, middle, buffer, thr_pool, !inplace);
			sort_task<T> sort_right_part(data, middle, right, buffer_left, thr_pool, !inplace);
			auto left_part_sorted = thr_pool.submit(sort_left_part);
			sort_right_part();
			thr_pool.active_wait(left_part_sorted);
			//left_part_sorted.get();
			if (inplace)
			{
				merge_task<T> left_part_merge(data, left, buffer, buffer_left, buffer_left, buffer_right, thr_pool);
				left_part_merge();
			}
			else
			{
				merge_task<T> right_part_merge(data, buffer, left, middle, middle, right, thr_pool);
				right_part_merge();
			}
		}
		return true;
	}
};

template <typename T>
void merge_sort(std::vector <T>& data, typename std::vector<T>::iterator left, typename std::vector<T>::iterator right)
{
	std::vector <T> buffer(data.size());
	auto buffer_it = buffer.begin();
	thread_pool <bool> task_queue;
	sort_task <T> whole_sort(data, left, right, buffer_it, task_queue, true);
	whole_sort();
}