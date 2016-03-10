#ifndef LOCK_FREE_QUEUE
#define LOCK_FREE_QUEUE

#include <atomic>
#include <memory>
#include <iostream>
#include <mutex>

template <typename T>
class lock_free_queue
{
public:
	struct node
	{
		std::shared_ptr<T> value;
		std::atomic <node*> next;
		node(T value_)
		{
			next = nullptr;
			value = std::make_shared<T>(value_);
		}
		node()
		{
			next = nullptr;
		}
	};
	std::atomic <node*> to_be_deleted;

	static void delete_nodes(node* nodes)
	{
		while (nodes)
		{
			node* next = nodes->next;
			delete nodes;
			nodes = next;
		}
	}

	std::atomic <size_t> curr_threads;

	void try_reclaim(node* old_head) 
	{
		if (curr_threads.load() == 1) 
		{
			node* new_head = old_head->next;
			node* curr_node = to_be_deleted.exchange(new_head);
			while (curr_node != new_head) 
			{
				node* next_node = curr_node->next;
				delete curr_node;
				curr_node = next_node;
			}
		}
	}
	
	std::atomic <node*> tail;
	std::atomic <node*> head;

public:
	lock_free_queue()
	{
		node* dummy = new node(0);
		curr_threads.store(0);
		to_be_deleted.store(dummy);
		tail.store(dummy);
		head.store(dummy);
	}

	~lock_free_queue()
	{
		delete_nodes(head.load());
	}

	void push(const T& value)
	{
		curr_threads.fetch_add(1);
		node* new_node = new node(value);
		node* cur_tail = tail.load();
		while (true)
		{
			cur_tail = tail.load();
			node* cur_tail_next = cur_tail->next.load();
			
			if (cur_tail == tail.load())
				if (cur_tail_next == nullptr)
				{
					if ((cur_tail->next).compare_exchange_weak(cur_tail_next, new_node))
					{
						tail.compare_exchange_strong(cur_tail, new_node);
						break;
					}
				}
				else
				{
					tail.compare_exchange_strong(cur_tail, cur_tail_next);
				}
		}
		curr_threads.fetch_sub(1);
	}

	bool pop(std::shared_ptr<T>& data)
	{
		curr_threads.fetch_add(1);
		while (true)
		{
			node* old_head = head.load();
			node* old_tail = tail.load();
			node* next = old_head->next;
			if (old_head == head.load())
				if (old_head == old_tail)
				{
					if (next == nullptr)
					{
						curr_threads.fetch_sub(1);
						return false;
					}
					tail.compare_exchange_weak(old_tail, next);
				}
				else
				{
					data = next->value;
					if (head.compare_exchange_strong(old_head, next))
					{
						try_reclaim(old_head);
						break;
					}
				}
		}
		curr_threads.fetch_sub(1);
		return true;
	}
};


#endif