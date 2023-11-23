#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{
    static constexpr auto GRAPHICS_QUEUE_FLAGS = (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer).m_mask;
    static constexpr auto COMPUTE_QUEUE_FLAGS = (vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer).m_mask;
    static constexpr auto TRANSFER_QUEUE_FLAGS = vk::QueueFlagBits::eTransfer;
    static constexpr auto VIDEO_QUEUE_FLAGS = (vk::QueueFlagBits::eVideoDecodeKHR | vk::QueueFlagBits::eTransfer).m_mask;

    static constexpr auto GRAPHICS_COMPUTE_QUEUE_FLAGS = (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute).m_mask;

    RhiAdapter::RhiAdapter(const vk::PhysicalDevice &physical_device) noexcept
        : m_name(physical_device.getProperties().deviceName.data()),
          m_handle(physical_device)
    {
        m_queue_family_properties = m_handle.getQueueFamilyProperties();
        m_extensions = m_handle.enumerateDeviceExtensionProperties();

        // device features
        m_features.device_features.pNext = &m_features.device_features_vulkan_11;
        m_features.device_features_vulkan_11.pNext = &m_features.device_features_vulkan_12;
        m_features.device_features_vulkan_12.pNext = &m_features.device_features_vulkan_13;
        m_features.device_features_vulkan_13.pNext = &m_features.device_features_mesh_shader;
        m_features.device_features_mesh_shader.pNext = &m_features.device_features_shader_object;
        m_features.device_features_shader_object.pNext = &m_features.device_features_robustness;
        m_handle.getFeatures2(&m_features.device_features);

        // device properties
        m_features.device_properies.pNext = &m_features.device_properties_vulkan_11;
        m_features.device_properties_vulkan_11.pNext = &m_features.device_properties_vulkan_12;
        m_features.device_properties_vulkan_12.pNext = &m_features.device_properties_vulkan_13;
        m_features.device_properties_vulkan_13.pNext = &m_features.device_properties_mesh_shader;
        m_features.device_properties_mesh_shader.pNext = &m_features.device_properties_shader_object;
        m_handle.getProperties2(&m_features.device_properies);

        for (uint32_t i = 0; i < m_queue_family_properties.size(); ++i)
        {
            const auto &queue = m_queue_family_properties[i];
            if (queue.queueCount == 0)
            {
                continue;
            }

            const auto flags = queue.queueFlags.m_mask;
            if (AllBits(flags, GRAPHICS_QUEUE_FLAGS))
            {
                m_queues[ERhiQueueType::eGraphics].index = i;
                m_queues[ERhiQueueType::eGraphics].properties = queue;
            }
            else if (AllBits(flags, COMPUTE_QUEUE_FLAGS) && !AnyBits(flags, vk::QueueFlagBits::eGraphics))
            {
                m_queues[ERhiQueueType::eAsyncCompute].index = i;
                m_queues[ERhiQueueType::eAsyncCompute].properties = queue;
            }
            else if (AllBits(flags, TRANSFER_QUEUE_FLAGS) && !AnyBits(flags, GRAPHICS_COMPUTE_QUEUE_FLAGS))
            {
                m_queues[ERhiQueueType::eAsyncTransfer].index = i;
                m_queues[ERhiQueueType::eAsyncTransfer].properties = queue;
            }
            else if (AllBits(flags, VIDEO_QUEUE_FLAGS) && !AnyBits(flags, GRAPHICS_COMPUTE_QUEUE_FLAGS))
            {
                m_queues[ERhiQueueType::eAsyncVideo].index = i;
                m_queues[ERhiQueueType::eAsyncVideo].properties = queue;
            }
        }
    }

    void RhiAdapter::PrintInfo() const noexcept
    {
        LOG_INFO("GPU adapter info:");
        String type;
        switch (m_features.device_properies.properties.deviceType)
        {
        case vk::PhysicalDeviceType::eIntegratedGpu:
            type = "Integrated";
            break;
        case vk::PhysicalDeviceType::eDiscreteGpu:
            type = "Discrete";
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            type = "Virtual";
            break;
        case vk::PhysicalDeviceType::eCpu:
            type = "CPU";
            break;
        default:
            type = "Other";
        }

        String vendor;
        switch (m_features.device_properies.properties.vendorID)
        {
        case 0x8086:
            vendor = "Intel";
            break;
        case 0x10DE:
            vendor = "Nvidia";
            break;
        case 0x1002:
            vendor = "AMD";
            break;
        default:
            vendor = std::to_string(m_features.device_properies.properties.vendorID);
        }

        uint32_t api_version[3] = {VK_VERSION_MAJOR(m_features.device_properies.properties.apiVersion),
                                   VK_VERSION_MINOR(m_features.device_properies.properties.apiVersion),
                                   VK_VERSION_PATCH(m_features.device_properies.properties.apiVersion)};

        LOG_INFO("\tType:\t\t{}", type);
        LOG_INFO("\tDevice ID:\t{}", m_features.device_properies.properties.deviceID);
        LOG_INFO("\tVendor:\t\t{}", vendor);
        LOG_INFO("\tAdapter:\t{}", m_features.device_properies.properties.deviceName.data());
        LOG_INFO("\tAPI Version:\t{}.{}.{}", api_version[0], api_version[1], api_version[2]);
        LOG_INFO("\tExtensions:");
        for (const auto &ext : m_extensions)
        {
            LOG_INFO("\t\t{}", ext.extensionName.data());
        }
    }

}