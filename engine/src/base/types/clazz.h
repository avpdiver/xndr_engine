#pragma once

namespace Be
{
    enum EUninitialized
    {
        uninitialized
    };

    class Noncopyable
    {
    public:
        Noncopyable() noexcept = default;

        Noncopyable(const Noncopyable &) = delete;
        Noncopyable(Noncopyable &&) = delete;

        Noncopyable &operator=(const Noncopyable &) = delete;
        Noncopyable &operator=(Noncopyable &&) = delete;
    };

    class MovableOnly
    {
    public:
        MovableOnly() noexcept = default;

        MovableOnly(MovableOnly &&) noexcept = default;
        MovableOnly &operator=(MovableOnly &&) noexcept = default;

        MovableOnly(const MovableOnly &) = delete;
        MovableOnly &operator=(const MovableOnly &) = delete;
    };

    class Noninstanceable
    {
    protected:
        Noninstanceable() = delete;
        Noninstanceable(const Noninstanceable &) = delete;
        Noninstanceable(Noninstanceable &&) = delete;

        Noninstanceable &operator=(const Noninstanceable &) = delete;
        Noninstanceable &operator=(Noninstanceable &&) = delete;
    };

    class NonAllocatable
    {
    public:
        [[nodiscard]] static void *operator new(usize_t) noexcept
        {
            return nullptr;
        }

        [[nodiscard]] static void *operator new[](usize_t) noexcept
        {
            return nullptr;
        }

        static void operator delete(void *, usize_t) noexcept {}
        static void operator delete[](void *, usize_t) noexcept {}

        // placement new
        [[nodiscard]] static void *operator new(usize_t, void *where) noexcept
        {
            return where;
        }

        static void operator delete(void *, void *) noexcept {}
    };
}