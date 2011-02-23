#include <rtthread.h>
#include <stddef.h>

void* operator new(size_t size)
{
	void *ptr = rt_malloc(size);
	return ((void *)ptr);
}

void* operator new[](size_t size)
{
	void *ptr = rt_malloc(size);
	return ((void *)ptr);
}

void operator delete(void *ptr)
{
	rt_free(ptr);
}

void operator delete[](void * ptr)
{
	rt_free(ptr);
}
