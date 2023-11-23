#pragma once

namespace Be
{

    namespace details
    {
        struct UMax
        {
            template <typename T>
            [[nodiscard]] constexpr operator const T() const noexcept
            {
                STATIC_ASSERT(T(~T{0}) > T{0});
                return T(~T{0});
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator==(const T &left, const UMax &right) noexcept
            {
                return T(right) == left;
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator!=(const T &left, const UMax &right) noexcept
            {
                return T(right) != left;
            }
        };

        struct Zero
        {
            template <typename T>
            [[nodiscard]] constexpr operator const T() const noexcept
            {
                return T(0);
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator==(const T &left, const Zero &right) noexcept
            {
                return T(right) == left;
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator!=(const T &left, const Zero &right) noexcept
            {
                return T(right) != left;
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator>(const T &left, const Zero &right) noexcept
            {
                return left > T(right);
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator<(const T &left, const Zero &right) noexcept
            {
                return left < T(right);
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator>=(const T &left, const Zero &right) noexcept
            {
                return left >= T(right);
            }

            template <typename T>
            [[nodiscard]] friend constexpr auto operator<=(const T &left, const Zero &right) noexcept
            {
                return left <= T(right);
            }
        };
    }

    static constexpr inline details::UMax UMax{};
    static constexpr inline details::Zero Zero{};

    static constexpr inline usize_t MaxAlignSize = sizeof(std::max_align_t);
}