#pragma once

namespace Be::Framework::RHI
{

    struct RhiDriverCreateInfo
    {
        Window &window;
        RhiDevice *device{nullptr};
        RhiDeviceCreateInfo device_create_info{};
        uint32_t frame_count{2};
        bool vsync{false};
    };

    class RhiDriver final : public Noncopyable
    {
    private:
        struct CommandPool
        {
            vk::CommandPool pool{VK_NULL_HANDLE};
            Queue<vk::CommandBuffer> buffers;
        };

        using ThreadLocalCommandPool = FixedArray<CommandPool, ERhiQueueTypeEnum::Count>;

    private:
        template <typename T>
        class ResourceDeleter final
        {
        public:
            forceinline void Push(T r)
            {
                enqueue_pool.push_back(r);
            }

            bool Destroy(vk::Device device, VmaAllocator allocator)
                requires(!IsSameType<T, VmaAllocation>)
            {
                for (auto &r : destroy_pool)
                {
                    device.destroy(r);
                }
                return Swap();
            }

            bool Destroy(vk::Device device, VmaAllocator allocator)
                requires IsSameType<T, VmaAllocation>
            {
                for (auto &r : destroy_pool)
                {
                    vmaFreeMemory(allocator, r);
                }
                return Swap();
            }

        private:
            forceinline bool Swap()
            {
                destroy_pool.clear();
                destroy_pool.swap(enqueue_pool);
                return !destroy_pool.empty();
            }

        private:
            Array<T> destroy_pool;
            Array<T> enqueue_pool;
        };

    private:
        using AllDeleters = Tuple<ResourceDeleter<VmaAllocation>,
                                  ResourceDeleter<vk::Semaphore>,
                                  ResourceDeleter<vk::Sampler>,
                                  ResourceDeleter<vk::Image>,
                                  ResourceDeleter<vk::ImageView>,
                                  ResourceDeleter<vk::Buffer>,
                                  ResourceDeleter<vk::BufferView>,
                                  ResourceDeleter<vk::CommandPool>,
                                  ResourceDeleter<vk::ShaderModule>,
                                  ResourceDeleter<vk::ShaderEXT>,
                                  ResourceDeleter<vk::DescriptorSetLayout>,
                                  ResourceDeleter<vk::DescriptorPool>,
                                  ResourceDeleter<vk::PipelineLayout>,
                                  ResourceDeleter<vk::QueryPool>>;

    public:
        RhiDriver(const RhiDriverCreateInfo &create_info) noexcept;
        ~RhiDriver() noexcept;

    public:
        [[nodiscard]] forceinline Window &GetWindow() noexcept
        {
            return m_window;
        }

        [[nodiscard]] forceinline RhiDevice &GetDevice() noexcept
        {
            return *m_device;
        }

    public:
        [[nodiscard]] ERhiQueueType GetAvailableQueueType(ERhiQueueType type) const noexcept;

        [[nodiscard]] forceinline RhiQueue &GetQueue(ERhiQueueType type) noexcept
        {
            PROFILER_SCOPE;

            return *(m_command_queues.at(GetAvailableQueueType(type)));
        }

    public:
        uint32_t BeginFrame() noexcept;
        void EndFrame() noexcept;

    public:
        [[nodiscard]] RhiCommandBufferHandle CreateCommandBuffer(vk::CommandPool pool, ERhiQueueType type) noexcept;
        [[nodiscard]] RhiCommandBufferHandle CreateCommandBuffer(ERhiQueueType type) noexcept;

    public:
        [[nodiscard]] RhiSemaphoreHandle CreateSemaphore() noexcept;
        [[nodiscard]] RhiFenceHandle CreateFence(uint64_t initial_value = 0) noexcept;

    public:
        [[nodiscard]] RhiBufferHandle CreateBuffer(const RhiBufferDesc &desc) noexcept;
        [[nodiscard]] RhiBufferViewHandle CreateBufferView(const RhiBufferViewDesc &desc) noexcept;

    public:
        [[nodiscard]] RhiSamplerHandle CreateSampler(const RhiSamplerDesc &desc = {}) noexcept;
        [[nodiscard]] RhiImageHandle CreateImage(const RhiImageDesc &desc) noexcept;
        [[nodiscard]] RhiImageViewHandle CreateImageView(const RhiImageViewDesc &desc) noexcept;

    public:
        template <typename T>
        forceinline void Delete(T resource) noexcept
        {
            PROFILER_SCOPE;

            std::get<ResourceDeleter<T>>(m_deleters[ThreadUtils::GetCurrentThreadIndex()]).Push(resource);
        }

        void Delete(ERhiQueueType type, vk::CommandBuffer cmd) noexcept;

    public:
        void AllocMemory(vk::Buffer buffer, const VmaAllocationCreateInfo &alloc_create_info, OUT VmaAllocation &allocation, OUT VmaAllocationInfo &allocation_info) noexcept;
        void AllocMemory(vk::Image image, const VmaAllocationCreateInfo &alloc_create_info, OUT VmaAllocation &allocation, OUT VmaAllocationInfo &allocation_info) noexcept;

    public:
        [[nodiscard]] forceinline void *MapMemory(VmaAllocation allocation) noexcept
        {
            PROFILER_SCOPE;

            void *ptr;
            VK_VERIFY(vmaMapMemory(m_vma_allocator, allocation, &ptr), "Failed to map GPU resource memory.");
            return ptr;
        }

        forceinline void UnmapMemory(VmaAllocation allocation) noexcept
        {
            PROFILER_SCOPE;
            
            vmaUnmapMemory(m_vma_allocator, allocation);
        }

    public:
        [[nodiscard]] RhiBindlessDescriptorPool &GetBindlessDescriptorPool(vk::DescriptorType type) noexcept;
        [[nodiscard]] RhiDescriptorSetPool AllocateDescriptorSet(const vk::DescriptorSetLayout &set_layout, const Map<vk::DescriptorType, uint32_t> &count) noexcept;

    public:
        void UploadBuffer(RhiBuffer &buffer, const Data &data,
                          ERhiResourceState final_state = ERhiResourceState::eShaderResource,
                          const RhiUploaderReadyCallback &callback = {}) noexcept;
        void UploadImage(RhiImage &image, const Array<RhiSubresourceData> &data,
                         ERhiResourceState final_state = ERhiResourceState::eShaderResource,
                         const RhiUploaderReadyCallback &callback = {}) noexcept;

    private:
        bool DeleteEnqueuedResources() noexcept;

#ifdef BE_DEBUG
    public:
        void SetAllocationDebugName(VmaAllocation allocation, const String &name) noexcept;
#endif

    private:
        Window &m_window;
        UniquePtr<RhiDevice> m_device{nullptr};
        VmaAllocator m_vma_allocator{VK_NULL_HANDLE};
        UniquePtr<RhiSwapchain> m_swapchain{nullptr};

    private:
        Map<ERhiQueueType, RhiQueueHandle> m_command_queues;

    private:
        Array<ThreadLocalCommandPool> m_command_pools;

    private:
        Map<vk::DescriptorType, RhiBindlessDescriptorPool> m_bindless_descriptor_pool;

    private:
        RhiResourceUploader m_resource_uploader;
        Array<AllDeleters> m_deleters;
    };

}
