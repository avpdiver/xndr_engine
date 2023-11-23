#pragma once

namespace Be
{

    struct MemResizePolicy
    {
    private:
        static constexpr usize_t MinSize = 16;

    public:
        [[nodiscard]] static constexpr usize_t ResizeInBytes(usize_t new_size) noexcept
        {
            const bool use_large_blocks = (new_size >> 20) > 5;
            return AlignUp(new_size, use_large_blocks ? LargeAllocationSize : SmallAllocationSize);
        }

    public:
        [[nodiscard]] static constexpr usize_t Resize(usize_t count, bool allow_reserve) noexcept
        {
            return std::max((allow_reserve ? count * 2 : count), MinSize);
        }

        [[nodiscard]] static constexpr usize_t Resize(usize_t element_size_of, usize_t count, bool allow_reserve) noexcept
        {
            const auto new_count = Resize(count, allow_reserve);
            const auto new_size = ResizeInBytes(element_size_of * new_count);
            const auto result = new_size / element_size_of;
            ASSERT(result >= new_count);
            return result;
        }

        template <typename T>
        [[nodiscard]] static constexpr usize_t Resize(usize_t count, bool allow_reserve) noexcept
        {
            return Resize(sizeof(T), count, allow_reserve);
        }
    };

}