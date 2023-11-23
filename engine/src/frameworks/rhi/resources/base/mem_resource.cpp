#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    void *RhiMemoryResource::Map() noexcept
    {
        PROFILER_SCOPE;

        if (m_persistently_mapped_memory)
        {
            return m_persistently_mapped_memory;
        }
        else
        {
            return m_driver.MapMemory(m_allocation);
        }
    }

    void RhiMemoryResource::Unmap() noexcept
    {
        PROFILER_SCOPE;
        
        if (m_persistently_mapped_memory)
        {
            return;
        }
        m_driver.UnmapMemory(m_allocation);
    }

}