#include "base/base.h"

void *operator new(size_t size)
{
    auto ptr = malloc(size);
    PROFILER_MEM_ALLOC(ptr, size);
    return ptr;
}

void *operator new[](size_t size)
{
    auto ptr = malloc(size);
    PROFILER_MEM_ALLOC(ptr, size);
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    PROFILER_MEM_FREE(ptr);
    free(ptr);
}

void operator delete(void *ptr, size_t size) noexcept
{
    PROFILER_MEM_FREE(ptr);
    free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    PROFILER_MEM_FREE(ptr);
    free(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept
{
    PROFILER_MEM_FREE(ptr);
    free(ptr);
}
