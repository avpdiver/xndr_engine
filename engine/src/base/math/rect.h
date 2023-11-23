#pragma once

namespace Be
{

    template <typename T>
    struct Rectangle
    {
        T x;
        T y;
        T width;
        T height;
    };

    using RectUint32 = Rectangle<uint32_t>;
    using RectInt32 = Rectangle<int32_t>;
    using RectFloat = Rectangle<float>;
    using RectDouble = Rectangle<double>;
    using Rect = Rectangle<BE_MATH_PRECISION>;
    
}