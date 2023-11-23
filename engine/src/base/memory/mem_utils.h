#pragma once

namespace Be
{

    static constexpr inline usize_t DefaultAllocatorAlign{__STDCPP_DEFAULT_NEW_ALIGNMENT__};
    static constexpr inline usize_t SmallAllocationSize{4 << 10}; // 4 Kb
    static constexpr inline usize_t LargeAllocationSize{4 << 20}; // 4 Mb - large page size in WIndows
    static constexpr inline usize_t DefaultAllocationSize = SmallAllocationSize;

    template <typename T, typename A>
    [[nodiscard]] constexpr auto AlignDown(const T value, const A align) noexcept
    {
        ASSERT(align > 0);
        if constexpr (IsPointer<T>)
        {
            return BitCast<T>(usize_t(BitCast<usize_t>(value) & ~(align - 1)));
        }
        else
        {
            return value & ~(align - 1);
        }
    }

    template <typename T, typename A>
    [[nodiscard]] constexpr auto AlignUp(const T value, const A align) noexcept
    {
        ASSERT(align > 0);
        if constexpr (IsPointer<T>)
        {
            return BitCast<T>(usize_t(((BitCast<usize_t>(value) + align - 1) & ~(align - 1))));
        }
        else
        {
            return (value + align - 1) & ~(align - 1);
        }
    }

    template <typename T, typename A>
    [[nodiscard]] constexpr bool IsAligned(const T value, const A align) noexcept
    {
        ASSERT(align > 0);
        if constexpr (IsPointer<T>)
        {
            return BitCast<usize_t>(value) % align == 0;
        }
        else
        {
            return value % align == 0;
        }
    }

    template <typename T>
    [[nodiscard]] forceinline constexpr decltype(auto) AddressOf(T &value) noexcept
    {
        return std::addressof(value);
    }

    template <typename T>
    [[nodiscard]] forceinline constexpr decltype(auto) VoidAddressOf(T &value) noexcept
    {
        return Cast<void>(std::addressof(value));
    }

    template <typename LT, typename RT>
    [[nodiscard]] forceinline usize_t AddressDistance(LT &lhs, RT &rhs) noexcept
    {
        return usize_t(AddressOf(lhs)) - usize_t(AddressOf(rhs));
    }

#ifdef BE_DEBUG
    forceinline void DbgInitMem(OUT void *ptr, usize_t size) noexcept
    {
        ASSERT((size == 0) || ((ptr != nullptr) == (size != 0)));
        std::memset(OUT ptr, 0xCD, size);
    }

    template <typename T>
    forceinline void DbgInitMem(OUT T *value) noexcept
    {
        DbgInitMem(OUT value, sizeof(T));
    }

    forceinline void DbgFreeMem(OUT void *ptr, usize_t size) noexcept
    {
        ASSERT((size == 0) || ((ptr != nullptr) == (size != 0)));
        std::memset(OUT ptr, 0xFE, size);
    }

    template <typename T>
    forceinline void DbgFreeMem(OUT T *value) noexcept
    {
        DbgFreeMem(OUT value, sizeof(T));
    }

#endif

    /*
        copy-ctor may throw
        default-ctor & move-ctor should !throw
    */
    template <typename T, typename... Args>
    forceinline T *PlacementNew(OUT void *ptr, Args &&...args) noexcept(false)
    {
        ASSERT(ptr != nullptr);
        CheckPointerCast<T>(ptr);
        return std::construct_at(reinterpret_cast<T *>(ptr), std::forward<Args>(args)...); // throw
    }

    template <typename T>
    forceinline void PlacementDelete(INOUT T *ptr) noexcept
    {
        std::destroy_at(ptr);
        DEBUG_ONLY(DbgFreeMem(ptr));
    }

    template <typename T, typename... Args>
    forceinline void Reconstruct(INOUT T *ptr, Args &&...args) noexcept(false)
    {
        CheckPointerCast<T>(ptr);

        std::destroy_at(ptr);
        DEBUG_ONLY(DbgFreeMem(ptr));

        std::construct_at(ptr, std::forward<Args>(args)...); // throw
    }

    template <typename T1, typename T2>
    forceinline void MemCopy(OUT T1 &dst, const T2 &src) noexcept
    {
        STATIC_ASSERT(sizeof(dst) >= sizeof(src));
        STATIC_ASSERT(IsMemCopyAvailable<T1>);
        STATIC_ASSERT(IsMemCopyAvailable<T2>);
        STATIC_ASSERT(!IsConst<T1>);
        ASSERT(static_cast<const void *>(&dst) != static_cast<const void *>(&src));

        std::memcpy(OUT & dst, &src, sizeof(src));
    }

    forceinline void MemCopy(OUT void *dst, const void *src, usize_t size) noexcept
    {
        if (size > 0) [[likely]]
        {
            ASSERT((dst != nullptr) && (src != nullptr));
            ASSERT(!Math::RangesOverlap<const void *>(dst, (byte_t *)dst + size, src, (byte_t *)src + size));

            std::memcpy(OUT dst, src, size);
        }
    }

    forceinline void MemCopy(OUT void *dst, usize_t dst_size, const void *src, usize_t src_size) noexcept
    {
        if (src_size > 0) [[likely]]
        {
            ASSERT(src_size <= dst_size);
            ASSERT((dst != nullptr) && (src != nullptr));
            ASSERT(!Math::RangesOverlap<const void *>(dst, (byte_t *)dst + dst_size, src, (byte_t *)src + src_size));

            std::memcpy(OUT dst, src, std::min(src_size, dst_size));
        }
    }

    forceinline void MemMove(OUT void *dst, const void *src, usize_t size) noexcept
    {
        if (size > 0) [[likely]]
        {
            ASSERT((dst != nullptr) && (src != nullptr));

            std::memmove(OUT dst, src, size);
        }
    }

    forceinline void MemMove(OUT void *dst, usize_t dst_size, const void *src, usize_t src_size) noexcept
    {
        if (src_size > 0) [[likely]]
        {
            ASSERT(src_size <= dst_size);
            ASSERT((dst != nullptr) && (src != nullptr));

            std::memmove(OUT dst, src, std::min(src_size, dst_size));
        }
    }

    template <typename T>
    forceinline void ZeroMem(OUT T &value) noexcept
    {
        STATIC_ASSERT(IsZeroMemAvailable<T>);
        STATIC_ASSERT(!IsPointer<T>);

        std::memset(OUT & value, 0, sizeof(value));
    }

    forceinline void ZeroMem(OUT void *ptr, usize_t size) noexcept
    {
        ASSERT((size == 0) || ((ptr != nullptr) == (size != 0)));
        std::memset(OUT ptr, 0, size);
    }

    template <typename T>
    forceinline void ZeroMem(OUT T *ptr, usize_t count) noexcept
    {
        STATIC_ASSERT(IsZeroMemAvailable<T>);
        ASSERT((count == 0) || ((ptr != nullptr) == (count != 0)));

        std::memset(OUT ptr, 0, sizeof(T) * count);
    }

    [[nodiscard]] forceinline bool MemEqual(const void *lhs, const void *rhs, usize_t size) noexcept
    {
        return std::memcmp(lhs, rhs, size) == 0;
    }

    template <typename T>
    [[nodiscard]] forceinline bool MemEqual(const T &lhs, const T &rhs) noexcept
    {
        return MemEqual(&lhs, &rhs, sizeof(T));
    }

}