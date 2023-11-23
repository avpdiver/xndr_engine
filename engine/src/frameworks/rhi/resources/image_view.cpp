#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    RhiImageView::RhiImageView(RhiDriver &driver, const RhiImageViewDesc &desc) noexcept
        : RhiResourceView{driver},
          m_image{desc.image},
          m_desc{desc}
    {
        PROFILER_SCOPE;

        m_format = m_image->GetFormat();

        if (m_desc.mip_levels == VK_REMAINING_MIP_LEVELS)
        {
            m_desc.mip_levels = m_image->GetMipLevels() - m_desc.base_mip_level;
        }
        if (m_desc.array_layers == VK_REMAINING_ARRAY_LAYERS)
        {
            m_desc.array_layers = m_image->GetArrayLayers() - m_desc.base_array_layer;
        }

        vk::ImageViewCreateInfo image_view_desc{};
        image_view_desc.image = m_image->VkHandle();
        image_view_desc.format = m_image->GetFormat();
        image_view_desc.viewType = m_desc.dimension;
        image_view_desc.subresourceRange.baseMipLevel = m_desc.base_mip_level;
        image_view_desc.subresourceRange.levelCount = m_desc.mip_levels;
        image_view_desc.subresourceRange.baseArrayLayer = m_desc.base_array_layer;
        image_view_desc.subresourceRange.layerCount = m_desc.array_layers;
        image_view_desc.subresourceRange.aspectMask = ImageFormatToImageAspect(image_view_desc.format);

        if (m_desc.view_type == vk::DescriptorType::eInputAttachment)
        {
            image_view_desc.subresourceRange.levelCount = 1;
        }

        if ((image_view_desc.subresourceRange.aspectMask & vk::ImageAspectFlagBits::eDepth) &&
            (image_view_desc.subresourceRange.aspectMask & vk::ImageAspectFlagBits::eStencil))
        {
            if (m_desc.slice == 0)
            {
                image_view_desc.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            }
            else
            {
                image_view_desc.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eStencil;
            }
        }

        m_write_descriptor_set.pImageInfo = &m_descriptor_image;

        m_handle = m_device.VkHandle().createImageView(image_view_desc);
        m_descriptor_image.imageView = m_handle;

        switch (m_desc.view_type)
        {
        case vk::DescriptorType::eSampledImage:
        case vk::DescriptorType::eCombinedImageSampler:
        {
            m_descriptor_image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            SetSampler(m_driver.CreateSampler({}));
            m_descriptor_image.sampler = m_sampler->VkHandle();
            break;
        }

        case vk::DescriptorType::eStorageImage:
            m_descriptor_image.imageLayout = vk::ImageLayout::eGeneral;
            break;

        default:
            return;
        }

        if (m_desc.bindless)
        {
            auto &pool = m_driver.GetBindlessDescriptorPool(m_desc.view_type);
            m_range = MakeUnique<RhiDescriptorPoolRange>(pool.Allocate(1));

            m_write_descriptor_set.descriptorType = m_desc.view_type;
            m_write_descriptor_set.dstBinding = pool.GetBinding();
            m_write_descriptor_set.dstSet = m_range->GetDescriptorSet();
            m_write_descriptor_set.dstArrayElement = m_range->GetOffset();
            m_write_descriptor_set.descriptorCount = 1;

            m_device.VkHandle().updateDescriptorSets(1, &m_write_descriptor_set, 0, nullptr);
        }
    }

    RhiImageView::~RhiImageView() noexcept
    {
        PROFILER_SCOPE;
        
        if (m_handle)
        {
            m_driver.Delete(m_handle);
        }
    }

}
