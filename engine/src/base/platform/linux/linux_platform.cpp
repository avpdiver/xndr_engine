#ifdef BE_PLATFORM_LINUX

#include "base/base.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

namespace Be::Platform
{

    usize_t GetPageSize() noexcept
    {
        auto res = sysconf(_SC_PAGESIZE);
        VERIFY(res != EINVAL, "Failed to get size of memory page");
        return usize_t(res);
    }

    usize_t GetVirtualMemorySize() noexcept
    {
        struct rlimit limit;
        auto res = getrlimit(RLIMIT_AS, &limit);
        VERIFY(res == 0, "Failed to get size of virtual memory");
        return usize_t(limit.rlim_max);
    }

    void *AllocateVirtualMemory(usize_t size) noexcept
    {
        auto ptr = mmap(nullptr, size,
                        PROT_NONE,
                        MAP_PRIVATE | MAP_ANONYMOUS, //| MAP_HUGETLB | MAP_HUGE_2MB,
                        -1, 0);
        VERIFY(ptr != MAP_FAILED, "Failed to map virtual memory");

        const auto page_size = GetPageSize();
        const auto page_count = size / page_size;
        const auto pages_per_block = page_count / 128;
        const ssize_t block_size = pages_per_block * GetPageSize();
        
        ssize_t remains = size;
        auto head = reinterpret_cast<byte_t *>(ptr);
        while (remains > 0)
        {
            auto res = mprotect(head, std ::min(block_size, remains), PROT_READ | PROT_WRITE);
            VERIFY(res == 0, "Failed to protect virtual memory");

            head += block_size;
            remains -= block_size;
        }

        return ptr;
    }

    void FreeVirtualMemory(void *ptr, usize_t size) noexcept
    {
        auto res = munmap(ptr, size);
        VERIFY(res == 0, "Failed to unmap virtual memory");
    }
}

#endif