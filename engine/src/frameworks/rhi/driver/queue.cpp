#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiQueue::RhiQueue(RhiDevice &device, ERhiQueueType type, uint32_t queue_family_index) noexcept
        : m_device{device},
          m_queue_family_index{queue_family_index},
          m_queue_type{type}
    {
        m_handle = m_device.VkHandle().getQueue(m_queue_family_index, 0);
    }

    void RhiQueue::AddCommandBuffer(RhiCommandBuffer &command_list) noexcept
    {
        PROFILER_SCOPE;

        m_command_lists.emplace_back(&command_list);
    }

    void RhiQueue::AddCommandBuffers(const Array<RhiCommandBufferHandle> &command_lists) noexcept
    {
        PROFILER_SCOPE;

        std::ranges::transform(command_lists, std::end(m_command_lists),
                               [](auto &c)
                               { return c.Get(); });
    }

    void RhiQueue::AddWaitSemaphore(const RhiSemaphore &semaphore, vk::PipelineStageFlags2 stage) noexcept
    {
        PROFILER_SCOPE;

        auto &s = m_wait_semaphores.emplace_back();
        s.semaphore = semaphore.VkHandle();
        s.stageMask = stage;
    }

    void RhiQueue::AddSignalSemaphore(const RhiSemaphore &semaphore, vk::PipelineStageFlags2 stage) noexcept
    {
        PROFILER_SCOPE;

        auto &s = m_signal_semaphores.emplace_back();
        s.semaphore = semaphore.VkHandle();
        s.stageMask = stage;
    }

    void RhiQueue::AddWaitFence(const RhiFence &fence, uint64_t wait_value, vk::PipelineStageFlags2 stage) noexcept
    {
        PROFILER_SCOPE;

        auto &s = m_wait_semaphores.emplace_back();
        s.semaphore = fence.VkHandle();
        s.value = wait_value;
        s.stageMask = stage;
    }

    void RhiQueue::AddSignalFence(const RhiFence &fence, uint64_t signal_value, vk::PipelineStageFlags2 stage) noexcept
    {
        PROFILER_SCOPE;

        auto &s = m_signal_semaphores.emplace_back();
        s.semaphore = fence.VkHandle();
        s.value = signal_value;
        s.stageMask = stage;
    }

    void RhiQueue::Submit() noexcept
    {
        PROFILER_SCOPE;

        if (!m_command_lists.empty() || !m_wait_semaphores.empty() || !m_signal_semaphores.empty())
        {
            Array<vk::CommandBufferSubmitInfo> buffer_submit_infos;
            buffer_submit_infos.reserve(m_command_lists.size());
            for (auto &command_list : m_command_lists)
            {
                buffer_submit_infos.emplace_back().commandBuffer = command_list->VkHandle();
            }

            vk::SubmitInfo2 submit_info{
                .waitSemaphoreInfoCount = uint32_t(m_wait_semaphores.size()),
                .pWaitSemaphoreInfos = m_wait_semaphores.empty() ? nullptr : m_wait_semaphores.data(),
                .commandBufferInfoCount = uint32_t(buffer_submit_infos.size()),
                .pCommandBufferInfos = buffer_submit_infos.empty() ? nullptr : buffer_submit_infos.data(),
                .signalSemaphoreInfoCount = uint32_t(m_signal_semaphores.size()),
                .pSignalSemaphoreInfos = m_signal_semaphores.empty() ? nullptr : m_signal_semaphores.data(),
            };

            VK_VERIFY(m_handle.submit2(1, &submit_info, {}), "Failed to submit queue.");

            m_wait_semaphores.clear();
            m_signal_semaphores.clear();
            m_command_lists.clear();
        }
    }

    void RhiQueue::Submit(const Array<RhiCommandBufferHandle> &command_lists) noexcept
    {
        PROFILER_SCOPE;
        
        AddCommandBuffers(command_lists);
        Submit();
    }
}