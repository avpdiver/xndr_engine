#pragma once

namespace Be
{

    struct MemCopyPolicy final : public Noninstanceable
    {
        // interface:
        //  Create      - create default elements
        //  Destroy     - destroy elements
        //  Copy        - copy elements from one memblock to another memblock, using copy constructor
        //  Move        - move elements from one memblock to another memblock, using move constructor without destructor on source element
        //  Replace     - replace elements inside memory block, using move constructor and destructor on source element - replace to the left
        //  ReplaceRev  - replace elements inside memory block with reverse order                                       - replace to the right
        //
        //  in Replace & ReplaceRev :
        //      in_single_mem_block is used when source and destination memory is aliased

    private:
        static constexpr uint32_t NonTrivialCtor = 1u << 0;
        static constexpr uint32_t NonTrivialDtor = 1u << 1;
        static constexpr uint32_t NonTrivialCopyCtor = 1u << 2;
        static constexpr uint32_t NonTrivialMoveCtor = 1u << 3;

    private:
        template <typename T, uint32_t Flags>
        struct DefaultImpl : public Noninstanceable
        {
            static void Create(OUT T *ptr, const usize_t count) noexcept
            {
                ASSERT((count == 0) || ((ptr != nullptr) == (count != 0)));

                if constexpr (!!(Flags & NonTrivialCtor))
                {
                    for (usize_t i = 0; i < count; ++i)
                    {
                        PlacementNew<T>(OUT ptr + i); // nothrow
                    }
                }
                else
                {
                    STATIC_ASSERT(IsZeroMemAvailable<T>);
                    ZeroMem(OUT ptr, sizeof(T) * count);
                }
            }

            static void Destroy(INOUT T *ptr, const usize_t count) noexcept
            {
                ASSERT((count == 0) || ((ptr != nullptr) == (count != 0)));

                if constexpr (!!(Flags & NonTrivialDtor))
                {
                    STATIC_ASSERT(IsNothrowDtor<T>);
                    for (usize_t i = 0; i < count; ++i)
                    {
                        ptr[i].~T();
                    }
                }
                else
                {
                    STATIC_ASSERT(IsZeroMemAvailable<T> || IsTrivialDtor<T>);
                }
                DEBUG_ONLY(DbgInitMem(OUT ptr, sizeof(T) * count));
            }

            static void Copy(OUT T *dst, const T *const src, const usize_t count) noexcept(IsNothrowCopyCtor<T>)
            {
                ASSERT((count == 0) || ((dst != nullptr) == (count != 0)));
                ASSERT((count == 0) || ((src != nullptr) == (count != 0)));

                if constexpr (!!(Flags & NonTrivialCopyCtor))
                {
                    for (usize_t i = 0; i < count; ++i)
                    {
                        PlacementNew<T>(OUT dst + i, src[i]); // throw
                    }
                }
                else
                {
                    STATIC_ASSERT(IsMemCopyAvailable<T>);
                    MemCopy(OUT dst, src, sizeof(T) * count);
                }
            }

            static void Move(OUT T *dst, INOUT T *src, const usize_t count) noexcept
            {
                ASSERT((count == 0) || ((dst != nullptr) == (count != 0)));
                ASSERT((count == 0) || ((src != nullptr) == (count != 0)));

                if constexpr (!!(Flags & NonTrivialMoveCtor))
                {
                    for (usize_t i = 0; i < count; ++i)
                    {
                        PlacementNew<T>(OUT dst + i, RVRef(src[i])); // nothrow
                    }
                }
                else
                {
                    STATIC_ASSERT(IsMemCopyAvailable<T>);
                    MemMove(OUT dst, src, sizeof(T) * count);
                }
            }

            static void Replace(OUT T *dst, INOUT T *src, const usize_t count, bool in_single_mem_block = false) noexcept
            {
                ASSERT((count == 0) || ((dst != nullptr) == (count != 0)));
                ASSERT((count == 0) || ((src != nullptr) == (count != 0)));
                ASSERT(src != dst);
                Unused(in_single_mem_block);

                if constexpr (!!(Flags & NonTrivialMoveCtor))
                {
                    for (usize_t i = 0; i < count; ++i)
                    {
                        PlacementNew<T>(OUT dst + i, RVRef(src[i])); // nothrow

                        if constexpr (!!(Flags & NonTrivialDtor))
                        {
                            STATIC_ASSERT(IsNothrowDtor<T>);
                            src[i].~T();
                        }
                        else
                        {
                            STATIC_ASSERT(IsZeroMemAvailable<T> || IsTrivialDtor<T>);
                        }
                    }
                }
                else
                {
                    STATIC_ASSERT(IsMemCopyAvailable<T>);
                    MemMove(OUT dst, src, sizeof(T) * count);
                }

                // clear old values after replace
                DEBUG_ONLY(
                    if (in_single_mem_block) {
                        for (T *t = src; t < src + count; ++t)
                        {
                            if (t < dst || t >= dst + count)
                            {
                                DbgInitMem(t);
                            }
                        }
                    })
            }

            static void ReplaceRev(OUT T *dst, INOUT T *src, const usize_t count, bool in_single_mem_block = false) noexcept
            {
                ASSERT((count == 0) || ((dst != nullptr) == (count != 0)));
                ASSERT((count == 0) || ((src != nullptr) == (count != 0)));
                ASSERT(src != dst);
                Unused(in_single_mem_block);

                if constexpr (!!(Flags & NonTrivialMoveCtor))
                {
                    for (usize_t i = count - 1; i < count; --i)
                    {
                        PlacementNew<T>(OUT dst + i, RVRef(src[i])); // nothrow

                        if constexpr (!!(Flags & NonTrivialDtor))
                        {
                            STATIC_ASSERT(std::is_nothrow_destructible_v<T>);
                            src[i].~T();
                        }
                        else
                        {
                            STATIC_ASSERT(IsZeroMemAvailable<T> || IsTrivialDtor<T>);
                        }
                    }
                }
                else
                {
                    STATIC_ASSERT(IsMemCopyAvailable<T>);
                    MemMove(OUT dst, src, sizeof(T) * count);
                }

                // clear old values after replace
                DEBUG_ONLY(
                    if (in_single_mem_block) {
                        for (T *t = src; t < src + count; ++t)
                        {
                            if (t < dst || t >= dst + count)
                            {
                                DbgInitMem(t);
                            }
                        }
                    })
            }
        };

    private:
        template <typename T>
        struct AutoDetected
        {
            static constexpr uint32_t Flags = (IsMemCopyAvailable<T> ? 0 : (NonTrivialCopyCtor | NonTrivialMoveCtor)) |
                                              (IsTrivialDtor<T> ? 0 : NonTrivialDtor) |
                                              (IsZeroMemAvailable<T> ? 0 : NonTrivialCtor);
            using type = DefaultImpl<T, Flags>;
        };

    public:
        template <typename T>
        struct CopyAndMoveCtor final : DefaultImpl<T, NonTrivialCtor | NonTrivialDtor | NonTrivialCopyCtor | NonTrivialMoveCtor>
        {
        };

        template <typename T>
        struct MemCopyWithoutCtor final : DefaultImpl<T, 0>
        {
        };

        template <typename T>
        using AutoDetect = typename AutoDetected<T>::type;
    };

}