#pragma once

#include "base/math/bit_math.h"

#define BE_DEFAULT_EPSILON 0.00000001
#define BE_DEFAULT_EPSILON_T static_cast<T>(BE_DEFAULT_EPSILON)
#define BE_MATH_PRECISION double

namespace Be::Math
{

    // 180 degrees
    static const BE_MATH_PRECISION PI = 3.1415926535897932384626433832795028841971693993751058209749445;

    // 360 degrees
    static const BE_MATH_PRECISION PI2 = PI * 2;

    // 90 degrees
    static const BE_MATH_PRECISION PI_OVER_2 = PI * 0.5;

    // 45 degrees
    static const BE_MATH_PRECISION PI_OVER_4 = PI * 0.25;

    // 30 degrees
    static const BE_MATH_PRECISION PI_DIV6 = PI / 6.0;

    // 1 degree
    static const BE_MATH_PRECISION PI_OVER_180 = PI / 180.0;

    template <class T>
    [[nodiscard]] constexpr forceinline T Radians(const T &rhs) noexcept
    {
        return rhs * T(PI_OVER_180);
    }

    template <class T>
    [[nodiscard]] forceinline bool Equals(T lhs, T rhs, T epsilon = BE_DEFAULT_EPSILON_T) noexcept
    {
        auto min = std::min(lhs, rhs);
        auto max = std::max(lhs, rhs);
        return (max - min) <= epsilon;
    }

    template <class T>
    [[nodiscard]] constexpr forceinline T Clamp(const T &value, T const lo = T(0), T const hi = T(1)) noexcept
    {
        ASSERT_MSG(hi >= lo, "Low is greater then Hi");

        T result = value;

        if (value > hi)
        {
            result = hi;
        }

        if (value < lo)
        {
            result = lo;
        }

        return result;
    }

    template <typename T>
    [[nodiscard]] forceinline T DeadZone(const T &min, const T &value) noexcept
    {
        return std::abs(value) >= std::abs(min) ? value : T(0);
    }

    template <typename T, bool ALLOW_TOUCH = true>
    [[nodiscard]] forceinline bool RangesOverlap(const T &begin1, const T &end1, const T &begin2, const T &end2) noexcept
    {
        ASSERT(begin1 <= end1 && begin2 <= end2);

        if constexpr (ALLOW_TOUCH)
        {
            return !(begin1 > end2 || begin2 > end1);
        }
        else
        {
            return !(begin1 >= end2 || begin2 >= end1);
        }
    }

}

#include "base/math/vectors.h"
#include "base/math/matrix.h"
#include "base/math/quaternion.h"
#include "base/math/rect.h"
#include "base/math/bound.h"
#include "base/math/pack_math.h"