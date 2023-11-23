#pragma once

namespace Be::Framework::RHI
{

    class RhiMemoryResource : public RhiResource
    {
    public:
        RhiMemoryResource(RhiDriver &driver) noexcept
            : RhiResource{driver},
              m_state_tracker(*this)
        {
        }

        virtual ~RhiMemoryResource() noexcept = default;

    public:
        [[nodiscard]] forceinline uint64_t GetMemorySize() const noexcept
        {
            return m_allocation_info.size;
        }

    public:
        void *Map() noexcept;
        void Unmap() noexcept;

    public:
        [[nodiscard]] forceinline RhiResourceStateTracker &GetStateTracker() noexcept
        {
            return m_state_tracker;
        }

        [[nodiscard]] forceinline const RhiResourceStateTracker &GetStateTracker() const noexcept
        {
            return m_state_tracker;
        }

    protected:
        VmaAllocation m_allocation{};
        VmaAllocationInfo m_allocation_info{};
        void *m_persistently_mapped_memory{nullptr};

    protected:
        RhiResourceStateTracker m_state_tracker;
    };

}