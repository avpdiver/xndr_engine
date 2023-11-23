#include "unit_tests_common.h"

void Malloc_Test(size_t size)
{
    const auto value = std::byte(255);

    // warm up
    auto ptr = malloc(size);
    *((std::byte *)ptr) = value;
    free(ptr);

    // test
    void *ptrs[100'000];
    const auto begins = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 100'000; i++)
    {
        ptrs[i] = malloc(size);
    }
    const auto ends = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < 100'000; i++)
    {
        free(ptrs[i]);
    }

    const auto duration = ends - begins;
    std::cout << "Malloc avg time:\t" << (duration.count() / 1'000'000) << std::endl;
}

extern void UnitTest_Mallocs()
{
    constexpr size_t SIZE[] = {
        8,
        16,
        32,
        64,
        128,
        1024,
        2048,
        4096,
        1048576,
        2097152};

    for (auto size : SIZE)
    {
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "Alloc size: " << size << std::endl;
        Malloc_Test(size);
    }
}