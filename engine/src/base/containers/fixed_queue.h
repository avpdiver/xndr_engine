#pragma once

namespace Be
{

    template <typename T, usize_t Count>
    class FixedQueue : public Noncopyable
    {
    private:
        static constexpr usize_t ValueSize = sizeof(T);
        static constexpr usize_t ValueAlign = alignof(T);

    private:
        using Self = FixedQueue<T, Count>;
        using ValueType = T;
        using AlignedType = std::aligned_storage_t<ValueSize, ValueAlign>;

    public:
        FixedQueue() noexcept = default;

    public:
        forceinline usize_t Size() const noexcept
        {
            return m_size;
        }

        forceinline bool Empty() const noexcept
        {
            return (m_size == 0);
        }

    public:
        EnableIf<IsNothrowCopyAssignable<T>, void> Push(const T &value) noexcept
        {
            auto ptr = PushBack();
            (*ptr) = value;
        }

        EnableIf<IsNothrowMoveAssignable<T>, void> Push(T &&value) noexcept
        {
            auto ptr = PushBack();
            (*ptr) = std::move(value);
        }

        template <class... Args>
        EnableIf<IsConstructible<T, Args...>, T &> Emplace(Args &&...args) noexcept
        {
            auto ptr = PushBack();
            return (*PlacementNew<T>(ptr, std::forward<Args>(args)...));
        }

    public:
        EnableIf<IsNothrowMoveAssignable<T>, T &&> Pop() noexcept
        {
            auto ptr = PopFront();
            return std::move(*ptr);
        }

    private:
        T *PushBack() noexcept
        {
            ASSERT(m_size < Count);

            auto push = m_push % Count;
            m_push++;
            m_size++;
            return reinterpret_cast<T *>(&m_data[push]);
        }

        T *PopFront() noexcept
        {
            ASSERT(m_size > 0);
            auto pop = m_pop % Count;
            m_pop++;
            m_size--;
            return reinterpret_cast<T *>(&m_data[pop]);
        }

    private:
        usize_t m_push{0};
        usize_t m_pop{0};
        usize_t m_size{0};

    private:
        AlignedType m_data[Count];
    };

}