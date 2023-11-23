#pragma once

namespace Be::Framework::RHI
{

    struct RhiDeviceCreateInfo
    {
        RhiAdapter *adapter{nullptr};
    };

    class RhiDevice final : public IDevice
    {
    public:
        RhiDevice(const RhiDeviceCreateInfo &create_info) noexcept;
        ~RhiDevice() noexcept;

    public:
        void Create() noexcept override;
        void Destroy() noexcept override;

    public:
        [[nodiscard]] forceinline vk::Device VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline const RhiAdapter &GetAdapter() const noexcept
        {
            return *m_adapter;
        }

    public:
        void WaitIdle() const noexcept;

    private:
        vk::Device m_handle{VK_NULL_HANDLE};

    private:
        const RhiAdapter *m_adapter{nullptr};
    };

}