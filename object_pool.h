#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstring>

#include <vector>

#define OBJECT_POOL_DEBUG_MODE

template<class T>
class ObjectPool
{
private:
#ifdef OBJECT_POOL_DEBUG_MODE
	static constexpr unsigned char GUARD = 0xff;
#endif // OBJECT_POOL_DEBUG_MODE

	struct Node_
	{
#ifdef OBJECT_POOL_DEBUG_MODE
		unsigned char pre_guard;
#endif // OBJECT_POOL_DEBUG_MODE
		T data;
#ifdef OBJECT_POOL_DEBUG_MODE
		unsigned char post_guard;
#endif // OBJECT_POOL_DEBUG_MODE
		Node_* next;
		unsigned long long id;
		unsigned int in_use;
	};

public:
	ObjectPool(int size) : id_(unique_id()), size_(size), usage_(0)
	{
		pool_.resize(size);

#ifdef OBJECT_POOL_DEBUG_MODE
		pre_guard_size = static_cast<int>(reinterpret_cast<char*>(&head_.data) -
			reinterpret_cast<char*>(&head_.pre_guard));
		post_guard_size = static_cast<int>(reinterpret_cast<char*>(&head_.next) -
			reinterpret_cast<char*>(&head_.post_guard));
#endif // OBJECT_POOL_DEBUG_MODE

		Node_* node;
		Node_* prev = &head_;
		for (size_t i = 0; i < size; ++i)
		{
			node = new Node_;

#ifdef OBJECT_POOL_DEBUG_MODE
			std::memset(&(node->pre_guard), GUARD, pre_guard_size);
			std::memset(&(node->post_guard), GUARD, post_guard_size);
#endif // OBJECT_POOL_DEBUG_MODE

			node->next = prev;
			node->id = id_;
			node->in_use = 0;

			pool_[i] = node;

			head_.next = node;
			prev = node;
		}
	}

	~ObjectPool()
	{
		size_t size = pool_.size();

		for (size_t i = 0; i < size; ++i)
		{
			delete pool_[i];
		}

		// option
		if (usage_ > 0)
		{
			// log
		}
	}

public:
	size_t size() const noexcept
	{
		return size_;
	}

	size_t usage() const noexcept
	{
		return usage_;
	}

	size_t remain() const noexcept
	{
		return size_ - usage_;
	}

	bool empty() const noexcept
	{
		return (size_ - usage_ == 0);
	}

public:
	T* alloc()
	{
		Node_* node;
		if (size_ > usage_)
		{
			node = head_.next;
			head_.next = node->next;
		}
		else
		{
			node = new Node_;

#ifdef OBJECT_POOL_DEBUG_MODE
			std::memset(&(node->pre_guard), GUARD, pre_guard_size);
			std::memset(&(node->post_guard), GUARD, post_guard_size);
#endif // OBJECT_POOL_DEBUG_MODE

			node->id = id_;
			pool_.push_back(node);
			++size_;
		}

		node->next = nullptr;
		node->in_use = 1;

		++usage_;

		return &(node->data);
	}

	bool dealloc(T* data)
	{
		do
		{
			if (data == nullptr)
			{
				break;
			}

#ifdef OBJECT_POOL_DEBUG_MODE
			Node_* node = reinterpret_cast<Node_*>(reinterpret_cast<char*>(data) - pre_guard_size);

			if (!check_guard(node))
			{
				break;
			}
#else
			Node_* node = reinterpret_cast<Node_*>(data);
#endif // OBJECT_POOL_DEBUG_MODE

			if (node->id != id_)
			{
				break;
			}

			if (node->in_use == 0)
			{
				break;
			}

			if (node->next)
			{
				break;
			}

			node->next = head_.next;
			node->in_use = 0;
			head_.next = node;

			--usage_;

			return true;
		} while (false);
		return false;
	}

private:
	unsigned long long unique_id()
	{
		static unsigned long long id = 1;
		return _InterlockedIncrement(&id);
	}

	bool check_guard(Node_* node)
	{
		unsigned char* guard = &(node->pre_guard);
		for (int i = 0; i < pre_guard_size; ++i)
		{
			if (*guard++ != GUARD)
			{
				return false;
			}
		}

		guard = &(node->post_guard);
		for (int i = 0; i < post_guard_size; ++i)
		{
			if (*guard++ != GUARD)
			{
				return false;
			}
		}

		return true;
	}

private:
	Node_ head_;
	unsigned long long id_;
	size_t size_;
	size_t usage_;
	std::vector<Node_*> pool_;
#ifdef OBJECT_POOL_DEBUG_MODE
	int pre_guard_size;
	int post_guard_size;
#endif // OBJECT_POOL_DEBUG_MODE
};