#pragma once

namespace Be
{

    template <typename T>
    static constexpr bool IsTrivial = std::is_trivially_destructible_v<T> &&
                                      std::is_trivially_move_assignable_v<T> &&
                                      std::is_trivially_move_constructible_v<T>;

    namespace details
    {
        template <typename T>
        struct IsOptional : std::false_type
        {
        };

        template <typename T>
        struct IsOptional<Optional<T>> : std::true_type
        {
        };

        template <typename... Types>
        struct IsVariant : std::false_type
        {
        };

        template <typename... Types>
        struct IsVariant<Variant<Types...>> : std::true_type
        {
        };

        template <typename T, template <typename...> class Templ>
        struct IsSpecializationOf : std::bool_constant<false>
        {
        };

        template <template <typename...> class Templ, typename... Args>
        struct IsSpecializationOf<Templ<Args...>, Templ> : std::bool_constant<true>
        {
        };

        template <typename T>
        struct MemCopyAvailable
        {
            static constexpr bool value = IsTrivial<T>;
        };

        template <typename T, usize_t I>
        struct MemCopyAvailable<T[I]>
        {
            static constexpr bool value = MemCopyAvailable<T>::value;
        };

        template <typename T, usize_t I>
        struct MemCopyAvailable<const T[I]>
        {
            static constexpr bool value = MemCopyAvailable<T>::value;
        };

        template <typename T>
        struct ZeroMemAvailable
        {
            static constexpr bool value = IsTrivial<T>;
        };

        template <typename T, usize_t I>
        struct ZeroMemAvailable<T[I]>
        {
            static constexpr bool value = ZeroMemAvailable<T>::value;
        };

        template <typename T, usize_t I>
        struct ZeroMemAvailable<const T[I]>
        {
            static constexpr bool value = ZeroMemAvailable<T>::value;
        };

        template <typename T>
        struct TriviallySerializable
        {
            using A = std::remove_cv_t<T>;
            static constexpr bool value = IsTrivial<A> && !std::is_pointer_v<A> && !std::is_reference_v<A>;
        };

        template <typename T, usize_t I>
        struct TriviallySerializable<T[I]>
        {
            static constexpr bool value = TriviallySerializable<std::remove_cv_t<T>>::value;
        };

        template <typename T, usize_t I>
        struct TriviallySerializable<const T[I]>
        {
            static constexpr bool value = TriviallySerializable<std::remove_cv_t<T>>::value;
        };

        struct MinValue
        {
            template <typename T>
            [[nodiscard]] constexpr operator const T() const noexcept
            {
                return std::numeric_limits<T>::min();
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator==(const T &left, const MinValue &right) noexcept
            {
                return T(right) == left;
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator!=(const T &left, const MinValue &right) noexcept
            {
                return T(right) != left;
            }
        };

        struct MaxValue
        {
            template <typename T>
            [[nodiscard]] constexpr operator const T() const noexcept
            {
                return std::numeric_limits<T>::max();
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator==(const T &left, const MaxValue &right) noexcept
            {
                return T(right) == left;
            }

            template <typename T>
            [[nodiscard]] friend constexpr bool operator!=(const T &left, const MaxValue &right) noexcept
            {
                return T(right) != left;
            }
        };

    }

    template <typename T>
    inline constexpr bool IsPod = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

    template <typename T>
    inline constexpr bool IsClass = std::is_class_v<T>;

    template <typename T>
    inline constexpr bool IsUnion = std::is_union_v<T>;

    template <typename T>
    static constexpr bool IsVoid = std::is_void_v<T>;

    template <typename T>
    inline constexpr bool IsInteger = std::is_integral_v<T>;

    template <typename T>
    inline constexpr bool IsSigned = std::is_signed_v<T>;

    template <typename T>
    inline constexpr bool IsUnsigned = std::is_unsigned_v<T>;

    template <typename T>
    static constexpr bool IsSignedInteger = IsInteger<T> && IsSigned<T>;

    template <typename T>
    static constexpr bool IsUnsignedInteger = (IsInteger<T> && IsUnsigned<T>);

    template <typename T>
    inline constexpr bool IsFloat = std::is_floating_point_v<T>;

    template <class T>
    inline constexpr bool IsArithmetic = std::is_arithmetic<T>::value;

    template <class T>
    inline constexpr bool IsScalar = std::is_scalar<T>::value;

    template <typename T>
    inline constexpr bool IsOptional = details::IsOptional<T>::value;

    template <typename... Types>
    inline constexpr bool IsVariant = details::IsVariant<Types...>::value;

    template <typename T>
    inline constexpr bool IsEnum = std::is_enum_v<T>;

    template <typename T>
    inline constexpr bool IsScalarOrEnum = (IsScalar<T> || IsEnum<T>);

    template <typename T1, typename T2>
    inline constexpr bool IsSameType = std::is_same_v<T1, T2>;

    template <typename T1, typename T2>
    inline constexpr bool IsConvertible = std::is_convertible_v<T1, T2>;

    template <typename T, typename... Args>
    inline constexpr bool IsConstructible = std::is_constructible_v<T, Args...>;

    template <typename BASE, typename DERIVED>
    inline constexpr bool IsBaseOf = std::is_base_of_v<BASE, DERIVED>;

    template <typename T, template <typename...> class Templ>
    inline constexpr bool IsSpecializationOf = details::IsSpecializationOf<T, Templ>::value;

    template <typename T1, typename T2>
    static constexpr bool IsSameTypes = std::is_same_v<T1, T2>;

    template <typename T>
    inline constexpr bool IsPointer = std::is_pointer_v<T>;

    template <typename T>
    inline constexpr bool IsArray = std::is_array_v<T>;

    template <typename T>
    inline constexpr bool IsNullPtr = std::is_null_pointer_v<T>;

    template <typename T>
    inline constexpr bool IsLValueRef = std::is_lvalue_reference_v<T>;

    template <typename T>
    inline constexpr bool IsRValueRef = std::is_rvalue_reference_v<T>;

    template <typename T>
    inline constexpr bool IsReference = std::is_reference_v<T>;

    template <typename T>
    inline constexpr bool IsConst = std::is_const_v<T>;

    template <typename T>
    inline constexpr bool IsConstRef = std::is_const_v<std::remove_reference_t<T>>;

    template <typename T>
    inline constexpr bool IsConstPtr = std::is_const_v<std::remove_pointer_t<T>>;

    template <typename T>
    using RemovePointer = std::remove_pointer_t<T>;

    template <typename T>
    using RemoveReference = std::remove_reference_t<T>;

    template <typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

    template <typename T>
    using RemoveCV = std::remove_cv_t<T>; // remove 'const', 'volatile', 'const volatile'

    template <typename T>
    inline constexpr bool IsDestructible = std::is_destructible<T>::value;

    template <typename T>
    inline constexpr bool IsTrivialDtor = std::is_trivially_destructible_v<T>;

    template <typename T>
    static constexpr bool IsNothrowCopyCtor = std::is_nothrow_copy_constructible_v<T> ||
                                              std::is_trivially_copy_constructible_v<T>;

    template <typename T>
    static constexpr bool IsNothrowDtor = std::is_nothrow_destructible_v<T> ||
                                          std::is_trivially_destructible_v<T>;
    template <typename T>
    static constexpr bool IsNothrowMoveCtor = std::is_nothrow_move_constructible_v<T> ||
                                              std::is_trivially_move_constructible_v<T>;
    template <typename T>
    static constexpr bool IsNothrowDefaultCtor = std::is_nothrow_default_constructible_v<T> ||
                                                 std::is_trivially_default_constructible_v<T>;
    template <typename T>
    static constexpr bool IsNothrowCopyAssignable = std::is_nothrow_copy_assignable_v<T> ||
                                                    (IsNothrowCopyCtor<T> && IsNothrowDtor<T>); // if used dtor + ctor
    template <typename T>
    static constexpr bool IsNothrowMoveAssignable = std::is_nothrow_move_assignable_v<T> ||
                                                    (IsNothrowMoveCtor<T> && IsNothrowDtor<T>); // if used dtor + move ctor

    template <typename T, typename... Args>
    static constexpr bool IsNothrowCtor = std::is_nothrow_constructible_v<T, Args...> ||
                                          std::is_trivially_constructible_v<T, Args...> ||
                                          (std::is_constructible_v<T, Args...> && std::is_trivial_v<T>);

    template <typename T>
    static constexpr bool IsMemCopyAvailable = details::MemCopyAvailable<T>::value;

    template <typename T>
    static constexpr bool IsZeroMemAvailable = details::ZeroMemAvailable<T>::value;

    template <typename T>
    static constexpr bool IsTriviallySerializable = details::TriviallySerializable<RemoveCV<T>>::value;

    template <bool Test, typename Type = void>
    using EnableIf = typename std::enable_if<Test, Type>::type;

    template <bool Test, typename Type = void>
    using DisableIf = typename std::enable_if<!Test, Type>::type;

    template <bool Test, typename IfTrue, typename IfFalse>
    using Conditional = std::conditional_t<Test, IfTrue, IfFalse>;

    template <typename T>
    static constexpr usize_t SizeOfInBits = sizeof(T) << 3;

    template <usize_t Bits>
    using BitSizeToUInt = Conditional<Bits <= SizeOfInBits<uint8_t>, uint8_t,
                                      Conditional<Bits <= SizeOfInBits<uint16_t>, uint16_t,
                                                  Conditional<Bits <= SizeOfInBits<uint32_t>, uint32_t,
                                                              Conditional<Bits <= SizeOfInBits<uint64_t>, uint64_t,
                                                                          void>>>>;

    template <usize_t Bits>
    using BitSizeToInt = Conditional<Bits <= SizeOfInBits<int8_t>, int8_t,
                                     Conditional<Bits <= SizeOfInBits<int16_t>, int16_t,
                                                 Conditional<Bits <= SizeOfInBits<int32_t>, int32_t,
                                                             Conditional<Bits <= SizeOfInBits<int64_t>, int64_t,
                                                                         void>>>>;

    template <usize_t ByteCount>
    using ByteSizeToUInt = BitSizeToUInt<ByteCount * 8>;

    template <usize_t ByteCount>
    using ByteSizeToInt = BitSizeToInt<ByteCount * 8>;

    template <typename T>
    using ToUnsignedInteger = BitSizeToUInt<SizeOfInBits<T>>;

    template <typename T>
    using ToSignedInteger = BitSizeToInt<SizeOfInBits<T>>;

    static constexpr details::MinValue MinValue{};
    static constexpr details::MaxValue MaxValue{};

    // --------------------------------------------------------------------------
    // METHODS

    // Infinity
    template <typename T>
        requires IsFloat<T>
    [[nodiscard]] forceinline constexpr auto Infinity()
    {
        return std::numeric_limits<T>::infinity();
    }

    // NaN
    template <typename T>
        requires IsFloat<T>
    [[nodiscard]] forceinline constexpr auto NaN()
    {
        return std::numeric_limits<T>::quiet_NaN();
    }

    // methods

    template <typename T, typename... Args>
        requires IsConstructible<T, Args...>
    [[nodiscard]] forceinline UniquePtr<T> MakeUnique(Args &&...args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
        requires IsConstructible<T, Args...>
    [[nodiscard]] forceinline SharedPtr<T> MakeShared(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T, size_t N>
    constexpr auto MakeFixedArray(T value) -> FixedArray<T, N>
    {
        FixedArray<T, N> a{};
        for (auto &x : a)
        {
            x = value;
        }
        return a;
    }

    template <typename... Args>
    constexpr void Unused(Args &&...) noexcept {}
}