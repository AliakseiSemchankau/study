#ifndef RING_BUFFER
#define RING_BUFFER

#include <atomic>
#include <vector>

template <typename T>
class spsc_ring_buffer
{
	std::vector <T> container;
	size_t capacity;
	std::atomic <size_t> head;
	std::atomic <size_t> tail;
public:
	spsc_ring_buffer(size_t size) : head(0), tail(0), capacity(size)
	{
		container.resize(size);
	}

	size_t next(size_t pos)
	{
		return (++pos) % capacity;
	}

	bool enqueue(T element)
	{
		size_t cur_tail = tail.load(std::memory_order_relaxed); // ��������� ������ ��������� ������ �������� tail, �� ����������� �������� �������� ���������
		size_t cur_head = head.load(std::memory_order_acquire); // ������ ���������� ����� ������� � ������ dequeue : ���� �� �����������, ��� �� �� ����� ��������� ���������� �������� head
		if (next(cur_tail) == cur_head)
			return false;
		container[cur_tail] = element;
		tail.store(next(cur_tail), std::memory_order_release); // ������ ���������� ����� ������� � ���������� tail 
		return true;
	}

	bool dequeue(T& element)
	{
		size_t cur_tail = tail.load(std::memory_order_acquire);// ���������� 
		size_t cur_head = head.load(std::memory_order_relaxed);
		if (cur_head == cur_tail)
			return false;
		element = container[cur_head];
		cur_head = next(cur_head);
		head.store(cur_head, std::memory_order_release);
		return true;
	}
};

#endif