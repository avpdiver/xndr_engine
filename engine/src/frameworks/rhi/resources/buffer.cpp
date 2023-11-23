#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

namespace Be::Framework::RHI
{

    RhiBuffer::RhiBuffer(RhiDriver &driver, const RhiBufferDesc &desc) noexcept
        : RhiMemoryResource{driver},
          m_size{desc.size}
    {
        PROFILER_SCOPE;

        vk::BufferCreateInfo create_info{};
        create_info.sharingMode = vk::SharingMode::eExclusive;
        create_info.usage = ResourceBindToBufferUsage(desc.bind_flag) | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        create_info.size = m_size;

        VmaAllocationCreateInfo alloc_create_info{};
        if (desc.mem_usage == ERhiResourceUsage::eReadback)
        {
            create_info.usage |= vk::BufferUsageFlagBits::eTransferDst;
            alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        else if (desc.mem_usage == ERhiResourceUsage::eUpload)
        {
            create_info.usage |= vk::BufferUsageFlagBits::eTransferSrc;
            alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        m_handle = m_device.VkHandle().createBuffer(create_info);

        m_driver.AllocMemory(m_handle, alloc_create_info, m_allocation, m_allocation_info);

#ifdef BE_DEBUG
        m_driver.SetAllocationDebugName(m_allocation, desc.debug_name);
#endif

        if (desc.mem_usage == ERhiResourceUsage::eReadback || desc.mem_usage == ERhiResourceUsage::eUpload)
        {
            m_persistently_mapped_memory = m_allocation_info.pMappedData;
        }

        m_state_tracker.SetResourceState(ERhiResourceState::eUndefined);
    }

    RhiBuffer::~RhiBuffer() noexcept
    {
        PROFILER_SCOPE;

        if (m_handle)
        {
            m_driver.Delete(m_allocation);
            m_driver.Delete(m_handle);
        }
    }

    void RhiBuffer::Update(uint64_t buffer_offset, const Data &data) noexcept
    {
        PROFILER_SCOPE;

        void *dst_data = (uint8_t *)Map() + buffer_offset;
        memcpy(dst_data, data.data(), data.size_bytes());
        Unmap();
    }

    void RhiBuffer::UpdateWithTextureData(uint64_t buffer_offset, uint64_t buffer_row_pitch, uint64_t buffer_depth_pitch,
                                          const void *src_data, uint64_t src_row_pitch, uint64_t src_depth_pitch,
                                          uint32_t num_rows, uint32_t num_slices) noexcept
    {
        PROFILER_SCOPE;
        
        void *dst_data = (uint8_t *)Map() + buffer_offset;
        for (uint32_t z = 0; z < num_slices; ++z)
        {
            uint8_t *dest_slice = reinterpret_cast<uint8_t *>(dst_data) + buffer_depth_pitch * z;
            const uint8_t *src_slice = reinterpret_cast<const uint8_t *>(src_data) + src_depth_pitch * z;
            for (uint32_t y = 0; y < num_rows; ++y)
            {
                memcpy(dest_slice + buffer_row_pitch * y, src_slice + src_row_pitch * y, src_row_pitch);
            }
        }
        Unmap();
    }

}
