#pragma once

namespace Be::Framework::RHI
{

    class RhiSemaphore final : public RhiResource
    {
    public:
        RhiSemaphore(RhiDriver &driver) noexcept;

    public:
        ~RhiSemaphore() noexcept;

    public:
        [[nodiscard]] forceinline vk::Semaphore VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eSemaphore;
        }

    private:
        vk::Semaphore m_handle{VK_NULL_HANDLE};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiSemaphore);

}
