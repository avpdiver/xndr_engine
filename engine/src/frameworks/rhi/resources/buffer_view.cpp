#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    RhiBufferView::RhiBufferView(RhiDriver &driver, const RhiBufferViewDesc &desc) noexcept
        : RhiResourceView{driver},
          m_buffer{desc.buffer},
          m_desc{desc}
    {
        PROFILER_SCOPE;

        if (m_desc.format == vk::Format::eUndefined)
        {
            m_descriptor_buffer_info.buffer = m_buffer->VkHandle();
            m_descriptor_buffer_info.offset = m_desc.offset;
            m_descriptor_buffer_info.range = m_desc.size;

            m_write_descriptor_set.pBufferInfo = &m_descriptor_buffer_info;
        }
        else
        {
            vk::BufferViewCreateInfo buffer_view_desc{
                .buffer = m_buffer->VkHandle(),
                .format = m_desc.format,
                .offset = m_desc.offset,
                .range = m_desc.size,
            };
            m_handle = m_device.VkHandle().createBufferView(buffer_view_desc);

            m_write_descriptor_set.pTexelBufferView = &m_handle;
        }

        if (m_desc.bindless)
        {
            auto &pool = m_driver.GetBindlessDescriptorPool(m_desc.view_type);
            m_range = MakeUnique<RhiDescriptorPoolRange>(pool.Allocate(1));

            m_write_descriptor_set.descriptorType = m_desc.view_type;
            m_write_descriptor_set.dstBinding = pool.GetBinding();
            m_write_descriptor_set.dstArrayElement = m_range->GetOffset();
            m_write_descriptor_set.descriptorCount = 1;
            m_write_descriptor_set.dstSet = m_range->GetDescriptorSet();

            m_device.VkHandle().updateDescriptorSets(1, &m_write_descriptor_set, 0, nullptr);
        }
    }

    RhiBufferView::~RhiBufferView() noexcept
    {
        PROFILER_SCOPE;
        
        if (m_handle)
        {
            m_driver.Delete(m_handle);
        }
    }
}
