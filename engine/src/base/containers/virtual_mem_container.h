#pragma once

namespace Be
{

    template <typename T>
    class VirtualMemoryContainer : public MovableOnly
    {
    private:
        static constexpr usize_t ValueSize = sizeof(T);
        static constexpr usize_t ValueAlign = alignof(T);

    private:
        using Self = VirtualMemoryContainer<T>;
        using ValueType = T;
        using AlignedType = std::aligned_storage_t<ValueSize, ValueAlign>;

    public:
        VirtualMemoryContainer(usize_t max_elements) noexcept
        {
            m_size = max_elements;
            m_mem_size = sizeof(AlignedType) * max_elements;
            m_data = (T *)Platform::AllocateVirtualMemory(m_mem_size);
        }

        ~VirtualMemoryContainer()
        {
            if (m_data != nullptr)
            {
                Platform::FreeVirtualMemory(m_data, m_mem_size);
            }
        }

    public:
        [[nodiscard]] forceinline T &operator[](size_t idx)
        {
            return m_data[idx];
        }

        [[nodiscard]] forceinline const T &operator[](size_t idx) const
        {
            return m_data[idx];
        }

        [[nodiscard]] forceinline size_t size() const
        {
            return m_size;
        }

    private:
        size_t m_size{0};
        usize_t m_mem_size{0};

    private:
        T *m_data{nullptr};
    };

}