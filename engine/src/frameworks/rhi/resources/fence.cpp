#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiFence::RhiFence(RhiDriver &driver, uint64_t initial_value) noexcept
        : RhiResource{driver}
    {
        PROFILER_SCOPE;

        vk::SemaphoreTypeCreateInfo timeline_create_info{};
        timeline_create_info.initialValue = initial_value;
        timeline_create_info.semaphoreType = vk::SemaphoreType::eTimeline;

        vk::SemaphoreCreateInfo create_info{.pNext = &timeline_create_info};

        m_handle = m_device.VkHandle().createSemaphore(create_info);
    }

    RhiFence::~RhiFence() noexcept
    {
        PROFILER_SCOPE;

        if (m_handle)
        {
            m_driver.Delete(m_handle);
        }
    }

    uint64_t RhiFence::GetCurrentValue() const noexcept
    {
        PROFILER_SCOPE;

        return m_device.VkHandle().getSemaphoreCounterValue(m_handle);
    }

    void RhiFence::Wait(uint64_t value) const noexcept
    {
        PROFILER_SCOPE;

        vk::SemaphoreWaitInfo wait_info{};
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &m_handle;
        wait_info.pValues = &value;
        VK_VERIFY(m_device.VkHandle().waitSemaphores(wait_info, UINT64_MAX), "Failed to wait GPU semaphore.");
    }

    void RhiFence::Signal(uint64_t value) const noexcept
    {
        PROFILER_SCOPE;
        
        vk::SemaphoreSignalInfo signal_info{};
        signal_info.semaphore = m_handle;
        signal_info.value = value;
        m_device.VkHandle().signalSemaphore(signal_info);
    }

}