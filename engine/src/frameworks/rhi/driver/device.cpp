#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{
    static const Array<const char *> REQUIRED_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
        VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
    };

    static Atomic<usize_t> g_devices_count{0};

    RhiDevice::RhiDevice(const RhiDeviceCreateInfo &create_info) noexcept
        : m_adapter{create_info.adapter}
    {
        PROFILER_SCOPE;

        RhiInstance::Create();

        if (m_adapter == nullptr)
        {
            m_adapter = &RhiInstance::ChooseAdapter(REQUIRED_EXTENSIONS);
        }

        g_devices_count++;
    }

    RhiDevice::~RhiDevice() noexcept
    {
        PROFILER_SCOPE;

        Destroy();
    }

    void RhiDevice::Create() noexcept
    {
        PROFILER_SCOPE;

        const auto queue_priority = 1.0f;

        Array<vk::DeviceQueueCreateInfo> queues_create_info;
        for (const auto &queue_info : m_adapter->GetQueues())
        {
            auto &queue_create_info = queues_create_info.emplace_back();
            queue_create_info.queueFamilyIndex = queue_info.second.index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
        }

        vk::DeviceCreateInfo device_create_info{};
        device_create_info.pNext = &(m_adapter->GetFeatures().device_features);
        device_create_info.queueCreateInfoCount = uint32_t(queues_create_info.size());
        device_create_info.pQueueCreateInfos = queues_create_info.data();
        device_create_info.pEnabledFeatures = nullptr;
        device_create_info.enabledExtensionCount = uint32_t(REQUIRED_EXTENSIONS.size());
        device_create_info.ppEnabledExtensionNames = REQUIRED_EXTENSIONS.data();

        m_handle = m_adapter->VkHandle().createDevice(device_create_info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_handle);

        LOG_INFO("RhiDevice is created.");
    }

    void RhiDevice::Destroy() noexcept
    {
        PROFILER_SCOPE;

        WaitIdle();

        m_handle.destroy();
        m_handle = VK_NULL_HANDLE;

        if (--g_devices_count == 0)
        {
            RhiInstance::Destroy();
        }

        LOG_INFO("RhiDevice is destroyed.");
    }

    void RhiDevice::WaitIdle() const noexcept
    {
        PROFILER_SCOPE;
        
        m_handle.waitIdle();
    }

}