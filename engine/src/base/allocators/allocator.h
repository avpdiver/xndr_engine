#pragma once

namespace Be
{

    class MemoryAllocator
    {
    public:
        virtual ~MemoryAllocator() noexcept = default;

    public:
        [[nodiscard]] virtual void *Alloc(usize_t size) = 0;
        [[nodiscard]] virtual void *Alloc(usize_t size, usize_t align) = 0;
        virtual void Free(void *ptr) = 0;
        virtual void Reset() = 0;
    };

}