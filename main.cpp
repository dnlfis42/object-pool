#include "object_pool.h"

#include <cstdio>

#include <vector>

struct St
{
	int a;
	double b;
};

int main()
{
	ObjectPool<St> pool(10);
	std::vector<St*> vec;

	for (int i = 0; i < 15; ++i)
	{
		St* obj = pool.alloc();
		if (obj == nullptr)
		{
			__debugbreak();
		}

		obj->a = i;
		obj->b = i * 1.1;

		vec.push_back(obj);
	}

	wprintf(L"%zu\n", pool.size());

	for (auto v : vec)
	{
		if (!pool.dealloc(v))
		{
			__debugbreak();
		}
	}

	if (pool.dealloc(vec[0]))
	{
		__debugbreak();
	}

	return 0;
}