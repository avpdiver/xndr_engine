#pragma once

namespace Be::Math
{

    // Packing/Encoding Functions

    [[nodiscard]] constexpr forceinline uint16_t F32toF16(float value)
    {
        union
        {
            float f;
            uint32_t ui;
        } u{value};

        auto ui = u.ui;

        int s = (ui >> 16) & 0x8000;
        int em = ui & 0x7fffffff;

        /* bias exponent and round to nearest; 112 is relative exponent bias (127-15) */
        int h = (em - (112 << 23) + (1 << 12)) >> 13;

        /* underflow: flush to zero; 113 encodes exponent -14 */
        h = (em < (113 << 23)) ? 0 : h;

        /* overflow: infinity; 143 encodes exponent 16 */
        h = (em >= (143 << 23)) ? 0x7c00 : h;

        /* NaN; note that we convert all types of NaN to qNaN */
        h = (em > (255 << 23)) ? 0x7e00 : h;

        return (uint16_t)(s | h);
    }

    [[nodiscard]] constexpr forceinline uint32_t PackRg16Float(const Float2 &v)
    {
        return F32toF16(v.x) | (F32toF16(v.y) << 16u);
    }

    [[nodiscard]] constexpr forceinline uint16_t EncodeR16Snorm(float value)
    {
        return static_cast<uint16_t>(Clamp(value >= 0.0f ? (value * 32767.0f + 0.5f) : (value * 32767.0f - 0.5f), -32768.0f, 32767.0f));
    }

    [[nodiscard]] constexpr forceinline uint32_t PackRg16Snorm(const Float2 &v)
    {
        return EncodeR16Snorm(Clamp(v.x, -1.0f, 1.0f)) | (EncodeR16Snorm(Clamp(v.y, -1.0f, 1.0f)) << 16u);
    }

    [[nodiscard]] constexpr forceinline UInt2 PackRgba16Snorm(const Float4 &v)
    {
        return UInt2(PackRg16Snorm(Float2(v.x, v.y)), PackRg16Snorm(Float2(v.z, v.w)));
    }

    [[nodiscard]] constexpr forceinline uint32_t PackRgb10A2Snorm(const Float4 &v)
    {
        return ((int32_t)(std::round(Clamp(v.x, -1.0f, 1.0f) * 511.0f)) & 0x3FF) << 0 |
               ((int32_t)(std::round(Clamp(v.y, -1.0f, 1.0f) * 511.0f)) & 0x3FF) << 10 |
               ((int32_t)(std::round(Clamp(v.z, -1.0f, 1.0f) * 511.0f)) & 0x3FF) << 20 |
               ((int32_t)std::round(Clamp(v.w, -1.0f, 1.0f)) << 30);
    }

    [[nodiscard]] constexpr forceinline uint32_t PackRgba8Snorm(const Float4 &v)
    {
        return ((uint8_t)std::round(Clamp(v.x, -1.0f, 1.0f) * 127.0f) << 0) |
               ((uint8_t)std::round(Clamp(v.y, -1.0f, 1.0f) * 127.0f) << 8) |
               ((uint8_t)std::round(Clamp(v.z, -1.0f, 1.0f) * 127.0f) << 16) |
               ((uint8_t)std::round(Clamp(v.w, -1.0f, 1.0f) * 127.0f) << 24);
    }

    [[nodiscard]] constexpr forceinline uint32_t PackRgba8Unorm(const Float4 &v)
    {
        return ((uint8_t)std::round(Clamp(v.x, 0.0f, 1.0f) * 255.0f) << 0) |
               ((uint8_t)std::round(Clamp(v.y, 0.0f, 1.0f) * 255.0f) << 8) |
               ((uint8_t)std::round(Clamp(v.z, 0.0f, 1.0f) * 255.0f) << 16) |
               ((uint8_t)std::round(Clamp(v.w, 0.0f, 1.0f) * 255.0f) << 24);
    }

}