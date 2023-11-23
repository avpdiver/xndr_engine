#pragma once

namespace Be
{

    class LinearAllocator final : public MemoryAllocator, public MovableOnly
    {
    public:
        LinearAllocator(const MemoryArena auto &arena) noexcept
            : m_ptr{static_cast<byte_t *>(arena.Begin())},
              m_begin{static_cast<byte_t *>(arena.Begin())},
              m_end{static_cast<byte_t *>(arena.End())}
        {
        }

    public:
        [[nodiscard]] forceinline void *Alloc(usize_t size) noexcept override
        {
            return Alloc(size, 1);
        }

        [[nodiscard]] forceinline void *Alloc(usize_t size, usize_t align) noexcept override
        {
            size = AlignUp(size, align);
            auto ptr = AlignUp(m_ptr, align);
            size += (ptr - m_ptr);

            ASSERT(m_ptr + size <= m_end);
            m_ptr += size;

            return ptr;
        }

        forceinline void Free(void *) noexcept override
        {
            FATAL("Method not supported");
        }

        forceinline void Reset() noexcept override
        {
            m_ptr = m_begin;
        }

    private:
        byte_t *m_ptr{nullptr};

    private:
        byte_t *const m_begin{nullptr};
        byte_t *const m_end{nullptr};
    };

    class LinearAllocatorWaitFree final : public MemoryAllocator, public MovableOnly
    {
    public:
        LinearAllocatorWaitFree(const MemoryArena auto &arena) noexcept
            : m_ptr{static_cast<byte_t *>(arena.Begin())},
              m_begin{static_cast<byte_t *>(arena.Begin())},
              m_end{static_cast<byte_t *>(arena.End())}
        {
        }

    public:
        [[nodiscard]] forceinline void *Alloc(usize_t size) noexcept override
        {
            return Alloc(size, 1);
        }

    public:
        [[nodiscard]] void *Alloc(usize_t size, usize_t align) noexcept override
        {
            size = AlignUp(size, align);
            auto ptr = m_ptr.fetch_add(size, std::memory_order_release);
            ASSERT(ptr + size <= m_end);

            return ptr;
        }

        forceinline void Free(void *) noexcept override
        {
            FATAL("Method not supported");
        }

        forceinline void Reset() noexcept override
        {
            m_ptr = m_begin;
        }
    private:
        Atomic<byte_t *> m_ptr{nullptr};

    private:
        byte_t *const m_begin{nullptr};
        byte_t *const m_end{nullptr};
    };

}