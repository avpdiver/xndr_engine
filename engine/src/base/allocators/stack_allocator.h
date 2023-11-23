#pragma once

namespace Be
{

    class StackAllocator final : public MemoryAllocator, public MovableOnly
    {
    public:
        explicit StackAllocator(const MemoryArena auto &arena) noexcept
            : m_ptr{static_cast<byte_t *>(arena.Begin())},
              m_begin{static_cast<byte_t *>(arena.Begin())},
              m_end{static_cast<byte_t *>(arena.End())}
        {
        }

    public:
        [[nodiscard]] void *Alloc(usize_t size) noexcept override
        {
            return Alloc(size, 1);
        }

        [[nodiscard]] void *Alloc(usize_t size, usize_t alignment) noexcept override
        {
            size = AlignUp(size, alignment);
            auto ptr = AlignUp(m_ptr, alignment);
            size += (ptr - m_ptr);

            ASSERT(m_ptr + size <= m_end);
            m_allocations.emplace_back(m_ptr);
            m_ptr += size;

            return ptr;
        }

        forceinline void Free(void *) noexcept override
        {
            ASSERT(m_allocations.size() > 0);
            m_ptr = m_allocations.back();
            m_allocations.pop_back();
        }

        void Reset() noexcept override
        {
            m_allocations.clear();
            m_ptr = m_begin;
        }

    private:
        byte_t *m_ptr{nullptr};
        Array<byte_t *> m_allocations;

    private:
        byte_t *const m_begin{nullptr};
        byte_t *const m_end{nullptr};
    };

}