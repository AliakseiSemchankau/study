#ifndef RING_BUFFER
#define RING_BUFFER

#include <atomic>
#include <vector>

const size_t CACHE_LINE_SIZE = 64;

template <typename T>
class spsc_ring_buffer
{
	struct node_t
	{
		char pad[CACHE_LINE_SIZE];
		T data;

		node_t(const T& data_) : data(data_)
		{}

		node_t()
		{}
	};

	struct atomic_pad
	{
		std::atomic <size_t> value;
		char pad[CACHE_LINE_SIZE];
	};

	std::vector <node_t> container;
	size_t capacity;
	atomic_pad head;
	atomic_pad tail;

public:
	spsc_ring_buffer(size_t size)
	{
		container.resize(size);
		capacity = size;
		head.value = 0;
		tail.value = 0;
	}

	size_t next(size_t pos)
	{
		return (++pos) % capacity;
	}

	bool enqueue(T element)
	{
		size_t cur_tail = tail.value.load(std::memory_order_relaxed);
		size_t cur_head = head.value.load(std::memory_order_acquire);
		if (next(cur_tail) == cur_head)
			return false;
		node_t local_var(element);
		container[cur_tail] = local_var;
		tail.value.store(next(cur_tail), std::memory_order_release);
		return true;
	}

	bool dequeue(T& element)
	{
		size_t cur_tail = tail.value.load(std::memory_order_acquire);
		size_t cur_head = head.value.load(std::memory_order_relaxed);
		if (cur_head == cur_tail)
			return false;
		element = container[cur_head].data;
		cur_head = next(cur_head);
		head.value.store(cur_head, std::memory_order_release);
		return true;
	}
};

#endif