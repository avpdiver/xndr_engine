#pragma once

#include <vulkan/utility/vk_format_utils.h>

namespace Be::Framework::RHI::RhiFormat
{
    uint32_t BytesPerPixel(vk::Format format) noexcept;
    uint32_t BytesPerPixel(const VKU_FORMAT_INFO &info) noexcept;
    uint32_t BitsPerPixel(const VKU_FORMAT_INFO &info) noexcept;
    void GetSizes(vk::Format format, uint32_t width, uint32_t height, uint64_t &num_bytes, uint64_t &row_bytes) noexcept;
    void GetSizes(vk::Format format, uint32_t width, uint32_t height, uint64_t &num_bytes, uint64_t &row_bytes, uint32_t &num_rows, uint32_t alignment) noexcept;
}
