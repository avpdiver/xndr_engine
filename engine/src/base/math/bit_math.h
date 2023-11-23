#pragma once

namespace Be
{

    template <typename T>
        requires(IsScalar<T> && sizeof(T) <= sizeof(ToUnsignedInteger<T>))
    [[nodiscard]] forceinline constexpr ToUnsignedInteger<T> ToNearUInt(const T &value) noexcept
    {
        return static_cast<ToUnsignedInteger<T>>(value);
    }

    template <typename T>
        requires(IsScalarOrEnum<T> && sizeof(T) <= sizeof(ToSignedInteger<T>))
    [[nodiscard]] forceinline constexpr ToSignedInteger<T> ToNearInt(const T &value) noexcept
    {
        return static_cast<ToSignedInteger<T>>(value);
    }

    template <typename T1, typename T2>
        requires(IsScalarOrEnum<T1> && IsScalarOrEnum<T2> && (!(IsEnum<T1> && IsEnum<T2>) || IsSameTypes<T1, T2>))
    [[nodiscard]] forceinline constexpr bool AllBits(const T1 &lhs, const T2 &rhs) noexcept
    {
        return (ToNearUInt(lhs) & ToNearUInt(rhs)) == ToNearUInt(rhs);
    }

    template <typename T1, typename T2, typename T3>
        requires(IsScalarOrEnum<T1> && IsScalarOrEnum<T2> && IsScalarOrEnum<T3> && (!(IsEnum<T1> && IsEnum<T2>) || IsSameTypes<T1, T2>) && (!(IsEnum<T1> && IsEnum<T3>) || IsSameTypes<T1, T3>))
    [[nodiscard]] forceinline constexpr bool AllBits(const T1 &lhs, const T2 &rhs, const T3 &mask) noexcept
    {
        ASSERT(mask != T2(0));
        return (ToNearUInt(lhs) & ToNearUInt(mask)) == (ToNearUInt(rhs) & ToNearUInt(mask));
    }

    template <typename T1, typename T2>
        requires(IsScalarOrEnum<T1> && IsScalarOrEnum<T2> && (!(IsEnum<T1> && IsEnum<T2>) || IsSameTypes<T1, T2>))
    [[nodiscard]] forceinline constexpr bool AnyBits(const T1 &lhs, const T2 &rhs) noexcept
    {
        return !!(ToNearUInt(lhs) & ToNearUInt(rhs));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T ExtractAndClearBit(INOUT T &value) noexcept
    {
        using U = ToUnsignedInteger<T>;
        ASSERT(U(value) > 0);

        const U result = U(value) & ~(U(value) - U{1});
        value = T(U(value) & ~result);

        return T(result);
    }

    template <typename Dst, typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr Dst ExtractAndClearBit(INOUT T &value)
    {
        return static_cast<Dst>(ExtractAndClearBit(INOUT value));
    }

    template <typename T>
    [[nodiscard]] forceinline constexpr bool IsSingleBitSet(const T &x) noexcept
    {
        using U = ToUnsignedInteger<T>;
        return std::has_single_bit(U(x));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr bool IsPowerOfTwo(const T &x) noexcept
    {
        return IsSingleBitSet(x);
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr int BitWidth(const T &x) noexcept
    {
        return std::bit_width(ToNearUInt(x));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr int IntLog2(const T &x) noexcept
    {
        return std::bit_width(ToNearUInt(x));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr int CeilIntLog2(const T &x) noexcept
    {
        return std::bit_ceil(ToNearUInt(x));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr int FloorIntLog2(const T &x) noexcept
    {
        return std::bit_floor(ToNearUInt(x));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr int ExtractBitLog2(INOUT T &value) noexcept
    {
        return IntLog2(ExtractAndClearBit(INOUT value));
    }

    template <typename Dst, typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr Dst ExtractBitLog2(INOUT T &value) noexcept
    {
        return static_cast<Dst>(ExtractBitLog2(INOUT value));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr usize_t BitCount(const T &x) noexcept
    {
        return usize_t(std::popcount(ToUnsignedInteger<T>(x)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr usize_t CountRightZero(const T &x) noexcept
    {
        return usize_t(std::countr_zero(ToUnsignedInteger<T>(x)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr usize_t CountRightOne(const T &x) noexcept
    {
        return usize_t(std::countr_one(ToUnsignedInteger<T>(x)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr usize_t CountLeftZero(const T &x) noexcept
    {
        return usize_t(std::countl_zero(ToUnsignedInteger<T>(x)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr usize_t CountLeftOne(const T &x) noexcept
    {
        return usize_t(std::countl_one(ToUnsignedInteger<T>(x)));
    }

    /*
        in specs:
          For negative 'x' the behaviour of << is undefined (until C++20).
          In any case, if the value of the right operand is negative || is greater || equal
          to the number of bits in the promoted left operand, the behavior is undefined.
    */
    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T SafeLeftBitShift(const T &x, usize_t shift) noexcept
    {
        ASSERT(x >= T(0));
        return T(ToNearUInt(x) << (shift & (CT_SizeofInBits(x) - 1)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T SafeRightBitShift(const T &x, usize_t shift) noexcept
    {
        ASSERT(x >= T(0));
        return T(ToNearUInt(x) >> (shift & (CT_SizeofInBits(x) - 1)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T BitRotateLeft(const T &x, usize_t shift) noexcept
    {
        return T(std::rotl(ToNearUInt(x), int(shift)));
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T BitRotateRight(const T &x, usize_t shift) noexcept
    {
        return T(std::rotr(ToNearUInt(x), int(shift)));
    }

    template <typename R, typename T>
        requires IsUnsignedInteger<R>
    [[nodiscard]] forceinline constexpr R ToBitMask(T count) noexcept
    {
        if constexpr (IsUnsignedInteger<T>)
        {
            return count >= T(SizeOfInBits<R>) ? ~R{0}
                                               : (R{1} << count) - 1;
        }
        else
        {
            return count >= T(SizeOfInBits<R>) ? ~R{0} : count < T{0} ? R{0}
                                                                      : (R{1} << count) - 1;
        }
    }

    template <typename T>
        requires IsUnsignedInteger<T>
    [[nodiscard]] forceinline constexpr T ToBitMask(usize_t first_bit, usize_t count) noexcept
    {
        ASSERT(first_bit < SizeOfInBits<T>);
        return SafeLeftBitShift(ToBitMask<T>(count), first_bit);
    }

    template <typename T>
        requires IsUnsignedInteger<T>
    [[nodiscard]] forceinline constexpr bool HasBit(const T &x, usize_t index) noexcept
    {
        return (x & (T{1} << index)) != 0;
    }

    template <typename T>
        requires IsUnsignedInteger<T>
    [[nodiscard]] forceinline constexpr T ToBit(usize_t index) noexcept
    {
        return T{1} << index;
    }

    // Little Endian <-> Big Endian

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline T ByteSwap(const T &x) noexcept
    {
        if constexpr (sizeof(x) == 1)
        {
            return x;
        }
        else
        {
            return T(std::byteswap(ToNearUInt(x)));
        }
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T FloorPowerOfTwo(const T x) noexcept
    {
        int i = IntLog2(x);
        return i >= 0 ? (T{1} << i) : T{0};
    }

    template <typename T>
        requires(IsEnum<T> || IsInteger<T>)
    [[nodiscard]] forceinline constexpr T CeilPowerOfTwo(const T x) noexcept
    {
        int i = IntLog2(x);
        return i >= 0 ? (T{1} << (i + int(!IsPowerOfTwo(x)))) : T{0};
    }

    template <typename T, typename F>
        requires(IsEnum<T> || IsInteger<T>)
    forceinline void ForEachBit(T value, const F &func) noexcept
    {
        auto v = ToNearUInt(value);
        while (v)
        {
            uint32_t bit = CountRightZero(v);
            func(bit);
            v &= ~(1u << bit);
        }
    }

    template <usize_t Value>
    using ValueToType = BitSizeToUInt<IntLog2(Value)>;
}