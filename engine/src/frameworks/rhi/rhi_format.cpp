#include "frameworks/rhi/rhi.h"

#include <vulkan/vulkan_format_traits.hpp>

namespace Be::Framework::RHI::RhiFormat
{

    uint32_t BytesPerPixel(vk::Format format) noexcept
    {
        const auto &info = vkuGetFormatInfo(VkFormat(format));
        return BytesPerPixel(info);
    }

    uint32_t BytesPerPixel(const VKU_FORMAT_INFO &info) noexcept
    {
        const auto block_size = info.block_size;
        const auto &block_extent = info.block_extent;
        return block_size / (block_extent.width * block_extent.height * block_extent.depth);
    }

    uint32_t BitsPerPixel(const VKU_FORMAT_INFO &info) noexcept
    {
        return BytesPerPixel(info) * 8;
    }

    void GetSizes(vk::Format format, uint32_t width, uint32_t height, uint64_t &num_bytes, uint64_t &row_bytes) noexcept
    {
        uint32_t num_rows = 0;
        uint32_t alignment = 1;
        return GetSizes(format, width, height, num_bytes, row_bytes, num_rows, alignment);
    }

    void GetSizes(vk::Format format, uint32_t width, uint32_t height,
                  uint64_t &num_bytes, uint64_t &row_bytes, uint32_t &num_rows,
                  uint32_t alignment) noexcept
    {
        const auto &info = vkuGetFormatInfo(VkFormat(format));
        const auto block_size = info.block_size;
        const auto &block_extent = info.block_extent;

        if (vkuFormatIsCompressed(VkFormat(format)))
        {
            row_bytes = block_size * ((width + block_extent.width - 1) / block_extent.width);
            row_bytes = AlignUp(row_bytes, alignment);
            num_rows = ((height + block_extent.height - 1) / block_extent.height);
        }
        else
        {
            row_bytes = width * BytesPerPixel(info);
            row_bytes = AlignUp(row_bytes, alignment);
            num_rows = height;
        }
        num_bytes = row_bytes * num_rows;
    }

}
