#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    RhiImage::RhiImage(RhiDriver &driver, vk::Image image, vk::Format format, uint32_t width, uint32_t height) noexcept
        : RhiMemoryResource{driver},
          m_handle{image},
          m_format{format},
          m_mip_levels{1},
          m_sample_count{1},
          m_back_buffer{true}
    {
        PROFILER_SCOPE;

        m_size.width = width;
        m_size.height = height;
        m_size.depth = 1;

        m_state_tracker.SetResourceState(ERhiResourceState::eUndefined);
    }

    RhiImage::RhiImage(RhiDriver &driver, const RhiImageDesc &desc) noexcept
        : RhiMemoryResource{driver},
          m_format{desc.format},
          m_mip_levels{desc.mip_levels},
          m_sample_count{desc.sample_count}
    {
        PROFILER_SCOPE;

        m_size.width = desc.width;
        m_size.height = desc.height;
        m_size.depth = desc.depth;

        if (m_mip_levels == 0)
        {
            m_mip_levels = (uint32_t)log2(std::max(m_size.width, m_size.height)) + 1;
        }

        vk::ImageUsageFlags usage = ResourceBindToImageUsage(desc.bind_flag);

        vk::ImageCreateInfo create_info{};
        create_info.extent.width = m_size.width;
        create_info.extent.height = m_size.height;
        create_info.mipLevels = m_mip_levels;
        create_info.format = m_format;
        create_info.tiling = vk::ImageTiling::eOptimal;
        create_info.initialLayout = vk::ImageLayout::eUndefined;
        create_info.usage = usage;
        create_info.samples = static_cast<vk::SampleCountFlagBits>(desc.sample_count);
        create_info.sharingMode = vk::SharingMode::eExclusive;

        switch (desc.type)
        {
        case ERhiImageType::e1D:
            create_info.imageType = vk::ImageType::e1D;
            create_info.extent.depth = 1;
            create_info.arrayLayers = m_size.depth;
            break;
        case ERhiImageType::e2D:
            create_info.imageType = vk::ImageType::e2D;
            create_info.extent.depth = 1;
            create_info.arrayLayers = m_size.depth;
            break;
        case ERhiImageType::e3D:
            create_info.imageType = vk::ImageType::e3D;
            create_info.extent.depth = m_size.depth;
            create_info.arrayLayers = 1;
            break;
        }

        if (create_info.arrayLayers % 6 == 0)
        {
            create_info.flags = vk::ImageCreateFlagBits::eCubeCompatible;
        }

        m_handle = m_device.VkHandle().createImage(create_info);

        VmaAllocationCreateInfo alloc_create_info{};
        alloc_create_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        m_driver.AllocMemory(m_handle, alloc_create_info, m_allocation, m_allocation_info);

#ifdef BE_DEBUG
        m_driver.SetAllocationDebugName(m_allocation, desc.debug_name);
#endif

        m_state_tracker.SetResourceState(ERhiResourceState::eUndefined);
    }

    RhiImage::~RhiImage() noexcept
    {
        PROFILER_SCOPE;

        if (m_handle && !m_back_buffer)
        {
            m_driver.Delete(m_allocation);
            m_driver.Delete(m_handle);
        }
    }

    vk::ImageLayout RhiImage::GetLayout() const noexcept
    {
        PROFILER_SCOPE;
        
        return ResourceStateToImageLayout(m_state_tracker.GetResourceState());
    }

}
