#pragma once

namespace Be
{

    class MallocMemoryArena final : public MovableOnly
    {
    public:
        MallocMemoryArena(usize_t size) noexcept
            : m_size{size}
        {
            m_begin = malloc(m_size);
            m_end = static_cast<byte_t *>(m_begin) + m_size;
        }

        ~MallocMemoryArena() noexcept
        {
            free(m_begin);
        }

    public:
        [[nodiscard]] forceinline void *Begin() const noexcept
        {
            return m_begin;
        }

        [[nodiscard]] forceinline void *End() const noexcept
        {
            return m_end;
        }

        [[nodiscard]] forceinline usize_t Size() const noexcept
        {
            return m_size;
        }

    private:
        void *m_begin{nullptr};
        void *m_end{nullptr};
        usize_t m_size{0};
    };

}