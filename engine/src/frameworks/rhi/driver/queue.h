#pragma once

namespace Be::Framework::RHI
{

    class RhiQueue final : public RefCounter
    {
    public:
        RhiQueue(RhiDevice &device, ERhiQueueType type, uint32_t queue_family_index) noexcept;

    public:
        [[nodiscard]] forceinline vk::Queue VkHandle() noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiQueueType GetQueueType() noexcept
        {
            return m_queue_type;
        }

        [[nodiscard]] forceinline uint32_t GetQueueFamilyIndex() noexcept
        {
            return m_queue_family_index;
        }

    public:
        [[nodiscard]] forceinline RhiDevice &GetDevice() noexcept
        {
            return m_device;
        }

    public:
        void AddCommandBuffer(RhiCommandBuffer &command_list) noexcept;
        void AddCommandBuffers(const Array<RhiCommandBufferHandle> &command_lists) noexcept;

    public:
        void AddWaitSemaphore(const RhiSemaphore &semaphore,
                              vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eAllCommands) noexcept;
        void AddSignalSemaphore(const RhiSemaphore &semaphore,
                                vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eAllCommands) noexcept;

    public:
        void AddWaitFence(const RhiFence &fence, uint64_t wait_value,
                          vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eAllCommands) noexcept;
        void AddSignalFence(const RhiFence &fence, uint64_t signal_value,
                            vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eAllCommands) noexcept;

    public:
        void Submit() noexcept;
        void Submit(const Array<RhiCommandBufferHandle> &command_lists) noexcept;

    private:
        vk::Queue m_handle{VK_NULL_HANDLE};

    private:
        RhiDevice &m_device;
        uint32_t m_queue_family_index;
        ERhiQueueType m_queue_type;

    private:
        Array<RhiCommandBuffer*> m_command_lists;
        Array<vk::SemaphoreSubmitInfo> m_wait_semaphores;
        Array<vk::SemaphoreSubmitInfo> m_signal_semaphores;
    };

    DEFINE_RHI_HANDLE(RhiQueue);
}