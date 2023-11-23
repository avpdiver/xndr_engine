#pragma once

namespace Be::Framework::RHI
{

    class RhiFence final : public RhiResource
    {
    public:
        RhiFence(RhiDriver &driver, uint64_t initial_value = 0) noexcept;

    public:
        ~RhiFence() noexcept;

    public:
        [[nodiscard]] forceinline vk::Semaphore VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eFence;
        }

    public:
        uint64_t GetCurrentValue() const noexcept;
        void Wait(uint64_t value) const noexcept;
        void Signal(uint64_t value) const noexcept;

    private:
        vk::Semaphore m_handle{VK_NULL_HANDLE};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiFence);

}
