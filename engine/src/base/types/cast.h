#pragma once

namespace Be
{

#ifdef BE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

    template <typename To, typename From>
    [[nodiscard]] constexpr To CheckCast(const From &src) noexcept
    {
        if constexpr (IsSigned<From> && IsUnsigned<To>)
        {
            ASSERT(src >= From(0));
        }

        ASSERT(static_cast<From>(static_cast<To>(src)) == src);
        return static_cast<To>(src);
    }

    template <typename To, typename From>
    [[nodiscard]] constexpr bool CheckCast(OUT To &dst, const From &src) noexcept
    {
        dst = static_cast<To>(src);
        return static_cast<From>(static_cast<To>(src)) == src;
    }

#ifdef BE_COMPILER_MSVC
#pragma warning(pop)
#endif

    template <typename To, typename From>
    [[nodiscard]] forceinline constexpr To BitCast(const From &src) noexcept
    {
        STATIC_ASSERT(sizeof(To) == sizeof(From), "must be same size!");
        STATIC_ASSERT(IsMemCopyAvailable<From> && IsMemCopyAvailable<To>, "must be trivial types!");

#ifdef __cpp_lib_bit_cast
        if constexpr (std::is_trivially_copyable_v<From> && std::is_trivially_copyable_v<To>)
        {
            return std::bit_cast<To>(src);
        }
        else
        {
            To dst;
            std::memcpy(OUT & dst, &src, sizeof(To));
            return dst;
        }
#else
        To dst;
        std::memcpy(OUT & dst, &src, sizeof(To));
        return dst;
#endif
    }

    // Cast pointer

    [[nodiscard]] forceinline bool CheckPointerAlignment(void const *ptr, usize_t align) noexcept
    {
        ASSERT_MSG(((align & (align - 1)) == 0), "Align must be power of 2");
        return (usize_t(ptr) & (align - 1)) == 0;
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline bool CheckPointerAlignment(T const *ptr) noexcept
    {
        constexpr usize_t align = alignof(R);

        STATIC_ASSERT(((align & (align - 1)) == 0), "Align must be power of 2");

        return (usize_t(ptr) & (align - 1)) == 0;
    }

    template <typename R, typename T>
    forceinline void CheckPointerCast(T const *ptr) noexcept
    {
        Unused(ptr);
        DEBUG_ONLY(
            if constexpr (!IsVoid<R>) {
                if (!CheckPointerAlignment<R>(ptr))
                {
                    std::stringstream str;
                    str << "Failed to cast pointer from '" << typeid(T).name() << "' to '" << typeid(R).name()
                        << "': memory address " << std::hex << usize_t(ptr) << " is not aligned to " << std::dec << alignof(R)
                        << ", it may cause undefined behavior";
                    LOG_ERROR("{}", str.str());
                }
            })
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R const volatile *Cast(T const volatile *value) noexcept
    {
        STATIC_ASSERT(sizeof(R *) == sizeof(T *) && sizeof(T *) == sizeof(void *));
        CheckPointerCast<R>(value);
        return static_cast<R const volatile *>(static_cast<void const volatile *>(value));
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R volatile *Cast(T volatile *value) noexcept
    {
        STATIC_ASSERT(sizeof(R *) == sizeof(T *) && sizeof(T *) == sizeof(void *));
        CheckPointerCast<R>(value);
        return static_cast<R volatile *>(static_cast<void volatile *>(value));
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R const *Cast(T const *value) noexcept
    {
        STATIC_ASSERT(sizeof(R *) == sizeof(T *) && sizeof(T *) == sizeof(void *));
        CheckPointerCast<R>(value);
        return static_cast<R const *>(static_cast<void const *>(value));
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R *Cast(T *value) noexcept
    {
        STATIC_ASSERT(sizeof(R *) == sizeof(T *) && sizeof(T *) == sizeof(void *));
        CheckPointerCast<R>(value);
        return static_cast<R *>(static_cast<void *>(value));
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R *Cast(const UniquePtr<T> &value) noexcept
    {
        return Cast<R>(value.get());
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline SharedPtr<R> Cast(const SharedPtr<T> &other) noexcept
    {
        return std::static_pointer_cast<R>(other);
    }

    template <typename T>
        requires IsEnum<T>
    [[nodiscard]] forceinline constexpr typename std::underlying_type<T>::type Cast(T value)
    {
        return static_cast<typename std::underlying_type<T>::type>(value);
    }

    template <typename T>
    [[nodiscard]] forceinline T *Cast(std::nullptr_t) noexcept
    {
        return static_cast<T *>(nullptr);
    }

    // Cast reference

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R const volatile &RefCast(T const volatile &value) noexcept
    {
        return *Cast<R>(&value);
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R const &RefCast(T const &value) noexcept
    {
        return *Cast<R>(&value);
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R volatile &RefCast(T volatile &value) noexcept
    {
        return *Cast<R>(&value);
    }

    template <typename R, typename T>
    [[nodiscard]] forceinline constexpr R &RefCast(T &value) noexcept
    {
        return *Cast<R>(&value);
    }

}