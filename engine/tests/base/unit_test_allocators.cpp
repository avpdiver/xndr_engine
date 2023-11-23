#include "unit_tests_common.h"

namespace
{
    using namespace Be;

    void LinearAllocator_Test(usize_t count)
    {
        constexpr auto arena_size = 4096;
        constexpr auto size = arena_size;
        MallocMemoryArena arena{arena_size};
        LinearAllocator allocator{arena};

        const auto begins = Clock::now();
        for (usize_t i = 0; i < count; i++)
        {
            auto ptr = allocator.Alloc(size);
            allocator.Free(nullptr);
        }
        const auto ends = Clock::now();

        const auto duration = ends - begins;

        LOG_INFO("Linear avg time:\t\t{}", duration.count() / count);
    }
}

extern void UnitTest_Allocators()
{
    constexpr auto COUNT = 1'000'000;
    constexpr auto SIZE = 1'000'000;

    LinearAllocator_Test(COUNT);

    TEST_PASSED();
}