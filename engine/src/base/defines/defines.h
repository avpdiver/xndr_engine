#pragma once

// for Linux
#undef False
#undef True
#undef Bool
#undef Success
#undef Status
#undef Always
#undef None

// for Windows
#undef DeleteFile
#undef CreateWindow
#undef CreateDirectory
#undef DrawText
#undef GetMonitorInfo
#undef BitScanForward
#undef BitScanReverse
#undef Yield
#undef CreateSemaphore
#undef LoadImage
#undef MemoryBarrier
#undef GetObject
#undef GetClassName
#undef min
#undef max
#undef UpdateResource
#undef _fstat
#undef _stat
#undef near
#undef far

#ifdef BE_COMPILER_MSVC
#define BE_FUNCTION_NAME __func__ //  local variable of type 'const char[]'
#elif defined(BE_COMPILER_GCC)
#define BE_FUNCTION_NAME __func__ //  local variable of type 'const char[]'
#else
#define BE_FUNCTION_NAME "unknown function"
#endif

#define BE_GETARG_0(_0_, ...) _0_
#define BE_GETARG_1(_0_, _1_, ...) _1_
#define BE_GETARG_2(_0_, _1_, _2_, ...) _2_
#define BE_GETARG_3(_0_, _1_, _2_, _3_, ...) _3_
#define BE_GETARG_4(_0_, _1_, _2_, _3_, _4_, ...) _4_
#define BE_GETARG_5(_0_, _1_, _2_, _3_, _4_, _5_, ...) _5_
#define BE_GETARG_6(_0_, _1_, _2_, _3_, _4_, _5_, _6_, ...) _6_
#define BE_GETRAW(_value_) _value_
#define BE_TOSTRING(...) #__VA_ARGS__
#define BE_CONCAT(_arg0_, _arg1_) _arg0_##_arg1_
#define BE_CONCAT_RAW(_arg0_, _arg1_) BE_CONCAT(_arg0_, _arg1_)
#define BE_ARGS(...) __VA_ARGS__

#define BE_ARGS_SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, N, ...) N
#define BE_ARGS_SIZE(...) BE_ARGS_SEQ(__VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#ifdef BE_DEBUG
#define BE_OPTIMIZE_IDS false
#define DEBUG_ONLY(/* code */...) __VA_ARGS__
#else
#define BE_OPTIMIZE_IDS true
#define DEBUG_ONLY(/* code */...)
#endif

#ifdef __GNUC__
#define PACKED_STRUCT __attribute__((packed, aligned(1)))
#else
#define PACKED_STRUCT
#endif

#define SHARED(R) using BE_CONCAT(R, Shared) = SharedPtr<R>
#define SHARED_N(N, R) using BE_CONCAT(R, Shared) = SharedPtr<N::R>

#define UNIQUE(R) using BE_CONCAT(R, Unique) = UniquePtr<R>
#define UNIQUE_N(N, R) using BE_CONCAT(R, Unique) = UniquePtr<N::R>

#define BIT(b) 1 << b

#define BIT_OPERATORS(T)                                                                                                \
    [[nodiscard]] constexpr T operator|(T lhs, T rhs) noexcept { return T(Be::ToNearUInt(lhs) | Be::ToNearUInt(rhs)); } \
    [[nodiscard]] constexpr T operator&(T lhs, T rhs) noexcept { return T(Be::ToNearUInt(lhs) & Be::ToNearUInt(rhs)); } \
                                                                                                                        \
    constexpr T &operator|=(T &lhs, T rhs) noexcept { return lhs = T(Be::ToNearUInt(lhs) | Be::ToNearUInt(rhs)); }      \
    constexpr T &operator&=(T &lhs, T rhs) noexcept { return lhs = T(Be::ToNearUInt(lhs) & Be::ToNearUInt(rhs)); }      \
                                                                                                                        \
    [[nodiscard]] constexpr T operator~(T lhs) noexcept { return T(~Be::ToNearUInt(lhs)); }                             \
    [[nodiscard]] constexpr bool operator!(T lhs) noexcept { return not Be::ToNearUInt(lhs); }

#include "base/defines/attribs.h"
#include "base/defines/config.h"
#include "base/defines/errors.h"
