#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiDriver::RhiDriver(const RhiDriverCreateInfo &create_info) noexcept
        : m_window{create_info.window},
          m_device{create_info.device}
    {
        PROFILER_SCOPE;

        if (!m_device)
        {
            m_device = MakeUnique<RhiDevice>(create_info.device_create_info);
        }

        const auto &queues = m_device->GetAdapter().GetQueues();
        for (const auto &q : queues)
        {
            m_command_queues[q.first] = MakeRefCounter<RhiQueue>(*m_device, q.first, q.second.index);
        }

        VmaVulkanFunctions vma_functions{};
        vma_functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
        vma_functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocator_info{};
        allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocator_info.instance = RhiInstance::VkHandle();
        allocator_info.physicalDevice = m_device->GetAdapter().VkHandle();
        allocator_info.device = m_device->VkHandle();
        allocator_info.pVulkanFunctions = &vma_functions;

        VK_VERIFY(vmaCreateAllocator(&allocator_info, &m_vma_allocator), "Failed to create VMA allocator.");

        m_swapchain = MakeUnique<RhiSwapchain>(*this, create_info.frame_count, create_info.vsync);

        m_deleters.resize(ThreadUtils::MaxThreadCount());
        m_command_pools.resize(ThreadUtils::MaxThreadCount());

        m_resource_uploader.Init(this);

        LOG_INFO("RhiDriver is created.");
    }

    RhiDriver::~RhiDriver() noexcept
    {
        PROFILER_SCOPE;

        while (DeleteEnqueuedResources())
        {
        }

        for (auto &t : m_command_pools)
        {
            for (auto &p : t)
            {
                m_device->VkHandle().destroyCommandPool(p.pool);
            }
        }

        if (m_vma_allocator)
        {
            vmaDestroyAllocator(m_vma_allocator);
            m_vma_allocator = VK_NULL_HANDLE;
        }

        if (m_device != nullptr)
        {
            m_command_queues.clear();
        }

        LOG_INFO("RhiDriver is destroyed.");
    }

    ERhiQueueType RhiDriver::GetAvailableQueueType(ERhiQueueType type) const noexcept
    {
        PROFILER_SCOPE;

        if (m_command_queues.contains(type))
        {
            return type;
        }
        return ERhiQueueType::eGraphics;
    }

    uint32_t RhiDriver::BeginFrame() noexcept
    {
        PROFILER_SCOPE;

        m_resource_uploader.BeginFrame();
        DeleteEnqueuedResources();
        return m_swapchain->NextImage();
    }

    void RhiDriver::EndFrame() noexcept
    {
        PROFILER_SCOPE;

        m_swapchain->Present();
        m_resource_uploader.EndFrame();
    }

    RhiBindlessDescriptorPool &RhiDriver::GetBindlessDescriptorPool(vk::DescriptorType type) noexcept
    {
        PROFILER_SCOPE;

        auto it = m_bindless_descriptor_pool.find(type);
        if (it == m_bindless_descriptor_pool.end())
        {
            it = m_bindless_descriptor_pool
                     .emplace(std::piecewise_construct,
                              std::forward_as_tuple(type),
                              std::forward_as_tuple(*m_device, type))
                     .first;
        }
        return it->second;
    }

    RhiCommandBufferHandle RhiDriver::CreateCommandBuffer(vk::CommandPool pool, ERhiQueueType type) noexcept
    {
        PROFILER_SCOPE;

        vk::CommandBufferAllocateInfo alloc_info{
            .commandPool = pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        auto handle = m_device->VkHandle().allocateCommandBuffers(alloc_info).front();
        auto r = new RhiCommandBuffer{*this, handle, type};

        return r;
    }

    RhiCommandBufferHandle RhiDriver::CreateCommandBuffer(ERhiQueueType type) noexcept
    {
        PROFILER_SCOPE;

        auto thread_index = ThreadUtils::GetCurrentThreadIndex();
        auto &cmd_pool = m_command_pools[thread_index];
        auto &pool = cmd_pool[type];

        if (!pool.buffers.empty())
        {
            auto handle = pool.buffers.front();
            pool.buffers.pop_front();
            return new RhiCommandBuffer{*this, handle, type};
        }

        if (!pool.pool)
        {
            vk::CommandPoolCreateInfo create_info{.flags = vk::CommandPoolCreateFlagBits::eTransient};
            create_info.queueFamilyIndex = GetQueue(type).GetQueueFamilyIndex();
            pool.pool = m_device->VkHandle().createCommandPool(create_info);
        }

        return CreateCommandBuffer(pool.pool, type);
    }

    RhiSemaphoreHandle RhiDriver::CreateSemaphore() noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiSemaphore{*this};
        return r;
    }

    RhiFenceHandle RhiDriver::CreateFence(uint64_t initial_value) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiFence{*this, initial_value};
        return r;
    }

    RhiBufferHandle RhiDriver::CreateBuffer(const RhiBufferDesc &desc) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiBuffer{*this, desc};
        return r;
    }

    RhiBufferViewHandle RhiDriver::CreateBufferView(const RhiBufferViewDesc &desc) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiBufferView{*this, desc};
        return r;
    }

    RhiSamplerHandle RhiDriver::CreateSampler(const RhiSamplerDesc &desc) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiSampler{*this, desc};
        return r;
    }

    RhiImageHandle RhiDriver::CreateImage(const RhiImageDesc &desc) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiImage{*this, desc};
        return r;
    }

    RhiImageViewHandle RhiDriver::CreateImageView(const RhiImageViewDesc &desc) noexcept
    {
        PROFILER_SCOPE;

        auto r = new RhiImageView{*this, desc};
        return r;
    }

    void RhiDriver::AllocMemory(vk::Buffer buffer,
                                const VmaAllocationCreateInfo &alloc_create_info,
                                VmaAllocation &allocation,
                                VmaAllocationInfo &allocation_info) noexcept
    {
        PROFILER_SCOPE;

        auto mem_req = m_device->VkHandle().getBufferMemoryRequirements(buffer);

        VK_VERIFY(vmaAllocateMemory(
                      m_vma_allocator,
                      (VkMemoryRequirements *)(&mem_req),
                      &alloc_create_info,
                      &allocation,
                      &allocation_info),
                  "Failed to allocate memory for GPU buufer.");
        VK_VERIFY(vmaBindBufferMemory(
                      m_vma_allocator,
                      allocation,
                      (VkBuffer)buffer),
                  "Failed to bind memory to GPU buffer.");
    }

    void RhiDriver::AllocMemory(vk::Image image,
                                const VmaAllocationCreateInfo &alloc_create_info,
                                VmaAllocation &allocation,
                                VmaAllocationInfo &allocation_info) noexcept
    {
        PROFILER_SCOPE;

        auto mem_req = m_device->VkHandle().getImageMemoryRequirements(image);

        VK_VERIFY(vmaAllocateMemory(
                      m_vma_allocator,
                      (VkMemoryRequirements *)(&mem_req),
                      &alloc_create_info,
                      &allocation,
                      &allocation_info),
                  "Failed to allocate memory for GPU image.");
        VK_VERIFY(vmaBindImageMemory(
                      m_vma_allocator,
                      allocation,
                      (VkImage)image),
                  "Failed to bind memory to GPU image.");
    }

    RhiDescriptorSetPool RhiDriver::AllocateDescriptorSet(const vk::DescriptorSetLayout &set_layout,
                                                          const Map<vk::DescriptorType, uint32_t> &count) noexcept
    {
        PROFILER_SCOPE;

        RhiDescriptorSetPool res{};

        Array<vk::DescriptorPoolSize> pool_sizes;
        for (auto &x : count)
        {
            pool_sizes.emplace_back();
            vk::DescriptorPoolSize &pool_size = pool_sizes.back();
            pool_size.type = x.first;
            pool_size.descriptorCount = x.second;
        }

        // TODO: fix me
        if (count.empty())
        {
            pool_sizes.emplace_back();
            vk::DescriptorPoolSize &pool_size = pool_sizes.back();
            pool_size.type = vk::DescriptorType::eSampler;
            pool_size.descriptorCount = 1;
        }

        vk::DescriptorPoolCreateInfo pool_info{};
        pool_info.poolSizeCount = uint32_t(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = 1;
        pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        res.pool = m_device->VkHandle().createDescriptorPool(pool_info);

        vk::DescriptorSetAllocateInfo alloc_info = {};
        alloc_info.descriptorPool = res.pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &set_layout;

        res.set = m_device->VkHandle().allocateDescriptorSets(alloc_info).front();

        return res;
    }

    void RhiDriver::UploadBuffer(RhiBuffer &buffer, const Data &data, ERhiResourceState final_state, const RhiUploaderReadyCallback &callback) noexcept
    {
        m_resource_uploader.UploadBuffer(buffer, data, final_state, callback);
    }

    void RhiDriver::UploadImage(RhiImage &image, const Array<RhiSubresourceData> &data, ERhiResourceState final_state, const RhiUploaderReadyCallback &callback) noexcept
    {
        m_resource_uploader.UploadImage(image, data, final_state, callback);
    }

    void RhiDriver::Delete(ERhiQueueType type, vk::CommandBuffer cmd) noexcept
    {
        PROFILER_SCOPE;

        cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        auto thread_index = ThreadUtils::GetCurrentThreadIndex();
        auto &cmd_pool = m_command_pools[thread_index];
        cmd_pool[type].buffers.push_back(cmd);
    }

    bool RhiDriver::DeleteEnqueuedResources() noexcept
    {
        PROFILER_SCOPE;

        bool has{false};
        for (auto &deleter : m_deleters)
        {
            std::apply(
                [&](auto &&...d)
                {
                    ((has |= d.Destroy(m_device->VkHandle(), m_vma_allocator)), ...);
                },
                deleter);
        }
        return has;
    }

#ifdef BE_DEBUG
    void RhiDriver::SetAllocationDebugName(VmaAllocation allocation, const String &name) noexcept
    {
        PROFILER_SCOPE;

        if (name.empty())
        {
            return;
        }
        vmaSetAllocationName(m_vma_allocator, allocation, name.c_str());
    }
#endif

}