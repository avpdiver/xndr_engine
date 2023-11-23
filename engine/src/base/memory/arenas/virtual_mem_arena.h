#pragma once

namespace Be
{

    class VirtualMemoryArena final : public MovableOnly
    {
    public:
        explicit VirtualMemoryArena(usize_t size) noexcept;
        ~VirtualMemoryArena() noexcept;

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