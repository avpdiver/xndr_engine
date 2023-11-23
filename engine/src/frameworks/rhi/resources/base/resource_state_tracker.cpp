#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    ////////////////////////////////////////////////////////////////////////////////////////
    /// State Tracker

    RhiResourceStateTracker::RhiResourceStateTracker(RhiResource &resource) noexcept
        : m_resource(resource)
    {
    }

    bool RhiResourceStateTracker::HasResourceState() const noexcept
    {
        return m_subresource_states.empty();
    }

    ERhiResourceState RhiResourceStateTracker::GetResourceState() const noexcept
    {
        return m_resource_state;
    }

    void RhiResourceStateTracker::SetResourceState(ERhiResourceState state) noexcept
    {
        PROFILER_SCOPE;

        m_subresource_states.clear();
        m_resource_state = state;
        m_subresource_state_groups.clear();
    }

    ERhiResourceState RhiResourceStateTracker::GetSubresourceState(uint32_t mip_level, uint32_t array_layer) const noexcept
    {
        PROFILER_SCOPE;

        auto it = m_subresource_states.find({mip_level, array_layer});
        if (it != m_subresource_states.end())
        {
            return it->second;
        }
        return m_resource_state;
    }

    void RhiResourceStateTracker::SetSubresourceState(uint32_t mip_level, uint32_t array_layer, ERhiResourceState state) noexcept
    {
        PROFILER_SCOPE;
        
        if (HasResourceState() && GetResourceState() == state)
        {
            return;
        }

        uint32_t level_count{0u};
        uint32_t layer_count{0u};
        if (m_resource.GetResourceType() == ERhiResourceType::eImage)
        {
            level_count = ((RhiImage &)m_resource).GetMipLevels();
            layer_count = ((RhiImage &)m_resource).GetArrayLayers();
        }

        Pair<uint32_t, uint32_t> key = {mip_level, array_layer};
        if (m_subresource_states.contains(key))
        {
            if (--m_subresource_state_groups[m_subresource_states[key]] == 0)
            {
                m_subresource_state_groups.erase(m_subresource_states[key]);
            }
        }

        m_subresource_states[key] = state;
        ++m_subresource_state_groups[state];
        if (m_subresource_state_groups.size() == 1 &&
            m_subresource_state_groups.begin()->second == layer_count * level_count)
        {
            m_subresource_state_groups.clear();
            m_subresource_states.clear();
            m_resource_state = state;
        }
    }

}