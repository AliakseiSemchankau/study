#ifndef STRIPED_HASH_SET
#define STRIPED_HASH_SET

#include <forward_list>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

const double DEFAULT_LOAD_FACTOR = 0.5;
const size_t DEFAULT_GROWTH_FACTOR = 2;

template <typename T, class H = std::hash<T>>
class striped_hash_set
{
	size_t concurrency_level;
	size_t growth_factor;
	size_t threshold_num_of_buckets = 10;
	double load_factor;
	std::vector <boost::shared_mutex> locks;
	std::vector <std::forward_list <T>> buckets;
	std::atomic <size_t> num_of_elements;
	H hash_function;
	
	double current_load_factor() const
	{
		return (double)num_of_elements / (double)buckets.size();
	}

	void free_add(const T& element)
	{
		auto element_hash = hash_function(element);
		buckets[num_of_bucket(element_hash)].push_front(element);
	}

	bool extend(size_t old_num_of_buckets, const T& element)
	{
		auto element_hash = hash_function(element);
		auto cur_lock = num_of_lock(element_hash);
		std::vector <std::unique_lock <boost::shared_mutex> > reallocate_lock;
		for (size_t i = 0; i < concurrency_level; ++i)
		{
			reallocate_lock.emplace_back(locks[i]);
			if (old_num_of_buckets < buckets.size())
				return false;
		}

		if (old_num_of_buckets < buckets.size())
			return false;
		auto num_of_buckets = buckets.size();
		std::vector <std::forward_list <T>> old_buckets;
		buckets.swap(old_buckets);
		buckets.clear();
		buckets.resize(growth_factor * num_of_buckets);
		for (auto &i : old_buckets)
			for (auto &j : i)
				free_add(j);
		return true;
	}

	size_t num_of_bucket(size_t element_hash) const
	{
		return element_hash % buckets.size();
	}

	size_t num_of_lock(size_t element_hash) const
	{
		return element_hash % concurrency_level;
	}

public:
	striped_hash_set(size_t concurrency_level_, double load_factor_, size_t growth_factor_) : 
		locks(concurrency_level_),
		num_of_elements(0), 
		concurrency_level(concurrency_level_), 
		load_factor(load_factor_), 
		growth_factor(growth_factor_)
	{
		if (concurrency_level_ < threshold_num_of_buckets)
			buckets.resize(threshold_num_of_buckets - threshold_num_of_buckets % concurrency_level_ + concurrency_level_);
		else
			buckets.resize(concurrency_level_);
	}

	striped_hash_set(size_t concurrency_level_) :
		locks(concurrency_level_),
		num_of_elements(0),
		concurrency_level(concurrency_level_),
		load_factor(DEFAULT_LOAD_FACTOR),
		growth_factor(DEFAULT_GROWTH_FACTOR)
	{
		if (concurrency_level_ < threshold_num_of_buckets)
			buckets.resize(threshold_num_of_buckets - threshold_num_of_buckets % concurrency_level_ + concurrency_level_);
		else
			buckets.resize(concurrency_level_);
	}

	bool contains(const T& element) 
	{
		auto element_hash = hash_function(element);
		boost::shared_lock <boost::shared_mutex> reader_lock(locks[num_of_lock(element_hash)]);
		std::forward_list<T>& cur_bucket = buckets[num_of_bucket(element_hash)];
		if (!cur_bucket.empty())
		{
			auto it = std::find(cur_bucket.begin(), cur_bucket.end(), element);
			if (it != cur_bucket.end())
				return true;
		}
		return false;
	}

	void add(const T& element)
	{
		auto element_hash = hash_function(element);
		boost::unique_lock <boost::shared_mutex> writer_lock(locks[num_of_lock(element_hash)]);
		std::forward_list<T>& cur_bucket = buckets[num_of_bucket(element_hash)];
		if (!cur_bucket.empty())
		{
			auto it = std::find(cur_bucket.begin(), cur_bucket.end(), element);
			if (it != cur_bucket.end())
				return;
		}
		buckets[num_of_bucket(element_hash)].push_front(element);
		++num_of_elements;
		if (current_load_factor() > load_factor)
		{
			size_t old_num_of_buckets = buckets.size();
			writer_lock.unlock();
			extend(old_num_of_buckets, element);
		}
	}

	void remove(const T& element)
	{
		auto element_hash = hash_function(element);
		boost::unique_lock <boost::shared_mutex> remove_lock(locks[num_of_lock(element_hash)]);
		std::forward_list<T>& cur_bucket = buckets[num_of_bucket(element_hash)];
		if (cur_bucket.empty())
			return;
		auto it = std::find(cur_bucket.begin(), cur_bucket.end(), element);
		if (it != cur_bucket.end())
		{
			cur_bucket.remove(element);
			--num_of_elements;
		}
	}
};

#endif

