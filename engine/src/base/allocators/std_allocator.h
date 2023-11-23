#pragma once

namespace Be
{

    template <typename T, typename AllocatorPtrType>
    class StdAllocatorWrapper
    {
    public:
        // std
        using value_type = T;
        using size_type = usize_t;
        using difference_type = ssize_t;

    public:
        using Self = StdAllocatorWrapper<T, AllocatorPtrType>;

    public:
        explicit StdAllocatorWrapper(AllocatorPtrType alloc) noexcept : m_alloc{std::forward<AllocatorPtrType>(alloc)} {}
        StdAllocatorWrapper(Self &&other) noexcept : m_alloc{std::move(other.m_alloc)} {}
        StdAllocatorWrapper(const Self &other) noexcept : m_alloc{other.m_alloc} {}

        template <typename B>
        StdAllocatorWrapper(const StdAllocatorWrapper<B, AllocatorPtrType> &other) noexcept : m_alloc{other.GetAllocatorPtr()} {}

    public:
        Self &operator=(const Self &) = delete;
        Self &operator=(Self &&) noexcept = default;

    public:
        // std
        [[nodiscard]] T *allocate(const usize_t count) noexcept(false)
        {
            T *ptr = Cast<T>(m_alloc->Alloc(sizeof(T) * count, alignof(T)));
            if (ptr == nullptr) [[unlikely]]
            {
                throw std::bad_alloc{};
            }
            return ptr;
        }

        void deallocate(T *const ptr, const usize_t count) noexcept
        {
            return m_alloc->Free(ptr);
        }

        [[nodiscard]] Self select_on_container_copy_construction() const noexcept
        {
            return Self{*m_alloc};
        }

        [[nodiscard]] bool operator==(const Self &rhs) const noexcept
        {
            return m_alloc == rhs.m_alloc;
        }

    public:
        [[nodiscard]] AllocatorPtrType GetAllocatorPtr() const noexcept { return m_alloc; }

    private:
        AllocatorPtrType m_alloc;
    };

}

namespace std
{
    template <typename T, typename A>
    void swap(Be::StdAllocatorWrapper<T, A> &lhs, Be::StdAllocatorWrapper<T, A> &rhs) noexcept
    {
        auto tmp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(tmp);
    }
}