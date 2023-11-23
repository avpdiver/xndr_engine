#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiSemaphore::RhiSemaphore(RhiDriver &driver) noexcept
        : RhiResource{driver}
    {
        PROFILER_SCOPE;

        vk::SemaphoreCreateInfo create_info{};
        m_handle = m_device.VkHandle().createSemaphore(create_info);
    }

    RhiSemaphore::~RhiSemaphore() noexcept
    {
        PROFILER_SCOPE;
        
        if (m_handle)
        {
            m_driver.Delete(m_handle);
        }
    }

}
