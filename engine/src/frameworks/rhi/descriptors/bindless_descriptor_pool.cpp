#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiDescriptorPoolRange::RhiDescriptorPoolRange(RhiBindlessDescriptorPool &pool,
                                                   vk::DescriptorSet descriptor_set,
                                                   uint32_t offset,
                                                   uint32_t size,
                                                   vk::DescriptorType type) noexcept
        : m_pool(pool),
          m_descriptor_set(descriptor_set),
          m_type(type),
          m_offset(offset),
          m_size(size),
          m_callback(this, [offset = m_offset, size = m_size, pool = m_pool](auto)
                     { pool.get().OnRangeDestroy(offset, size); })
    {
    }

    vk::DescriptorSet RhiDescriptorPoolRange::GetDescriptorSet() const noexcept
    {
        return m_descriptor_set;
    }

    uint32_t RhiDescriptorPoolRange::GetOffset() const noexcept
    {
        return m_offset;
    }

    RhiBindlessDescriptorPool::RhiBindlessDescriptorPool(RhiDevice &device, vk::DescriptorType type) noexcept
        : m_device{device}, m_type(type)
    {
    }

    void RhiBindlessDescriptorPool::ResizeHeap(uint32_t req_size) noexcept
    {
        PROFILER_SCOPE;

        if (req_size > MAX_BINDLESS_HEAP_SIZE)
        {
            FATAL("Requested size for bindless pool more than MAX_BINDLESS_HEAP_SIZE");
        }

        if (m_size >= req_size)
        {
            return;
        }

        Descriptor descriptor{};

        // Create pool
        {
            vk::DescriptorPoolSize pool_size{};
            pool_size.type = m_type;
            pool_size.descriptorCount = req_size;

            vk::DescriptorPoolCreateInfo pool_info{};
            pool_info.poolSizeCount = 1;
            pool_info.pPoolSizes = &pool_size;
            pool_info.maxSets = 1;
            pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

            descriptor.pool = m_device.VkHandle().createDescriptorPoolUnique(pool_info);
        }

        // Create layout
        {
            vk::DescriptorSetLayoutBinding binding{};
            binding.binding = GetBinding();
            binding.descriptorType = m_type;
            binding.descriptorCount = MAX_BINDLESS_HEAP_SIZE;
            binding.stageFlags = vk::ShaderStageFlagBits::eAll;

            vk::DescriptorBindingFlags binding_flag = vk::DescriptorBindingFlagBits::eVariableDescriptorCount;

            vk::StructureChain<vk::DescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo> chain{
                {
                    .bindingCount = 1,
                    .pBindings = &binding,
                },
                {
                    .bindingCount = 1,
                    .pBindingFlags = &binding_flag,
                },
            };

            descriptor.set_layout = m_device.VkHandle().createDescriptorSetLayoutUnique(chain.get<vk::DescriptorSetLayoutCreateInfo>());
        }

        // Create descriptor set
        {
            vk::StructureChain<vk::DescriptorSetAllocateInfo, vk::DescriptorSetVariableDescriptorCountAllocateInfo> chain{
                {
                    .descriptorPool = descriptor.pool.get(),
                    .descriptorSetCount = 1,
                    .pSetLayouts = &descriptor.set_layout.get(),
                },
                {
                    .descriptorSetCount = 1,
                    .pDescriptorCounts = &req_size,
                },
            };

            auto v = m_device.VkHandle().allocateDescriptorSetsUnique(chain.get<vk::DescriptorSetAllocateInfo>());
            descriptor.set = std::move(v[0]);
        }

        // Copy old descriptors

        if (m_size > 0u)
        {
            /*
            auto sampler = m_device.CreateSampler({});

            Array<vk::DescriptorImageInfo> infos{};
            infos.resize(req_size - m_size);
            for (auto &i : infos)
            {
                i.sampler = sampler->VkHandle();
            }
            */

            vk::WriteDescriptorSet write_descriptors{};
            write_descriptors.descriptorType = m_type;
            write_descriptors.dstBinding = GetBinding();
            write_descriptors.dstSet = descriptor.set.get();
            write_descriptors.dstArrayElement = m_size;
            // write_descriptors.descriptorCount = uint32_t(infos.size());
            // write_descriptors.pImageInfo = infos.data();

            vk::CopyDescriptorSet copy_descriptors{};
            copy_descriptors.srcBinding = GetBinding();
            copy_descriptors.dstBinding = GetBinding();
            copy_descriptors.srcSet = m_descriptor.set.get();
            copy_descriptors.dstSet = descriptor.set.get();
            copy_descriptors.descriptorCount = m_size;

            m_device.VkHandle().updateDescriptorSets(1, &write_descriptors, 1, &copy_descriptors);
        }

        m_size = req_size;

        m_descriptor.set = std::move(descriptor.set);
        m_descriptor.set_layout = std::move(descriptor.set_layout);
        m_descriptor.pool = std::move(descriptor.pool);
    }

    RhiDescriptorPoolRange RhiBindlessDescriptorPool::Allocate(uint32_t count) noexcept
    {
        PROFILER_SCOPE;

        auto it = m_empty_ranges.lower_bound(count);
        if (it != m_empty_ranges.end())
        {
            auto size = uint32_t(it->first);
            auto offset = uint32_t(it->second);
            m_empty_ranges.erase(it);

            return RhiDescriptorPoolRange(*this, m_descriptor.set.get(), offset, size, m_type);
        }

        if (m_offset + count > m_size)
        {
            ResizeHeap(std::max(m_offset + count, 2 * (m_size + 1)));
        }

        m_offset += count;

        return RhiDescriptorPoolRange(*this, m_descriptor.set.get(), m_offset - count, count, m_type);
    }

    void RhiBindlessDescriptorPool::OnRangeDestroy(uint32_t offset, uint32_t size) noexcept
    {
        PROFILER_SCOPE;
        
        m_empty_ranges.emplace(size, offset);
    }

}
