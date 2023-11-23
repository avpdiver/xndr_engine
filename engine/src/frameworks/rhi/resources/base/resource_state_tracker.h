#pragma once

namespace Be::Framework::RHI
{

    class RhiResourceStateTracker final
    {
    public:
        RhiResourceStateTracker(RhiResource &resource) noexcept;

    public:
        bool HasResourceState() const noexcept;
        ERhiResourceState GetResourceState() const noexcept;
        void SetResourceState(ERhiResourceState state) noexcept;

    public:
        ERhiResourceState GetSubresourceState(uint32_t mip_level, uint32_t array_layer) const noexcept;
        void SetSubresourceState(uint32_t mip_level, uint32_t array_layer, ERhiResourceState state) noexcept;

    private:
        RhiResource &m_resource;

    private:
        ERhiResourceState m_resource_state{ERhiResourceState::eUnknown};
        Map<Pair<uint32_t, uint32_t>, ERhiResourceState> m_subresource_states;
        Map<ERhiResourceState, usize_t> m_subresource_state_groups;
    };

}