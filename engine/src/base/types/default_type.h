#pragma once

namespace Be
{
    namespace details
    {
        template <typename T, bool IsEnum>
        struct EnumWithUnknown
        {
            static constexpr bool value = false;
        };

        template <typename T>
        struct EnumWithUnknown<T, true>
        {
            static constexpr bool value = true; // Detect_Unknown<T>::value;
        };

        template <typename T>
        static constexpr bool IsEnumWithUnknown = EnumWithUnknown<T, std::is_enum_v<T>>::value;

        template <typename T, int Index>
        struct DefaultValueForUninitialized
        {
        };

        template <typename T>
        struct DefaultValueForUninitialized<T, 0>
        {
            static constexpr T Get() noexcept
            {
                return T();
            }
        };

        template <typename T>
        struct DefaultValueForUninitialized<T, /*int, float, pointer*/ 2>
        {
            static constexpr T Get() noexcept
            {
                return T(0);
            }
        };

        template <typename T>
        struct DefaultValueForUninitialized<T, /*enum*/ 1>
        {
            static constexpr T Get() noexcept
            {
                return T::eUnknown;
            }
        };

        template <typename T>
        struct GetDefaultValueForUninitialized
        {
            static constexpr int GetIndex() noexcept
            {
                return IsEnumWithUnknown<T> ? 1 : std::is_floating_point<T>::value || std::is_integral<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value ? 2
                                                                                                                                                                        : 0;
            }

            static constexpr T GetDefault() noexcept
            {
                return DefaultValueForUninitialized<T, GetIndex()>::Get();
            }
        };

        struct DefaultType final
        {
            constexpr DefaultType() noexcept
            {
            }

            template <typename T>
            [[nodiscard]] constexpr operator T() const noexcept
            {
                return GetDefaultValueForUninitialized<T>::GetDefault();
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator==(const T &lhs, const DefaultType &) noexcept
            {
                return lhs == GetDefaultValueForUninitialized<T>::GetDefault();
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator!=(const T &lhs, const DefaultType &rhs) noexcept
            {
                return !(lhs == rhs);
            }
        };

    }

    static constexpr inline details::DefaultType Default = {};

}