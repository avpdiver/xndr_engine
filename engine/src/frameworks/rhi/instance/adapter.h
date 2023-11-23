#pragma once

namespace Be::Framework::RHI
{

    class RhiAdapter final
    {
    public:
        struct Features
        {
            vk::PhysicalDeviceFeatures2 device_features{};
            vk::PhysicalDeviceVulkan11Features device_features_vulkan_11{};
            vk::PhysicalDeviceVulkan12Features device_features_vulkan_12{};
            vk::PhysicalDeviceVulkan13Features device_features_vulkan_13{};

            vk::PhysicalDeviceMeshShaderFeaturesEXT device_features_mesh_shader{};
            vk::PhysicalDeviceShaderObjectFeaturesEXT device_features_shader_object{};
            vk::PhysicalDeviceRobustness2FeaturesEXT device_features_robustness{};

            vk::PhysicalDeviceProperties2 device_properies{};
            vk::PhysicalDeviceVulkan11Properties device_properties_vulkan_11{};
            vk::PhysicalDeviceVulkan12Properties device_properties_vulkan_12{};
            vk::PhysicalDeviceVulkan13Properties device_properties_vulkan_13{};

            vk::PhysicalDeviceMeshShaderPropertiesEXT device_properties_mesh_shader{};
            vk::PhysicalDeviceShaderObjectPropertiesEXT device_properties_shader_object{};
        };

        struct QueueInfo
        {
            uint32_t index{MaxValue};
            vk::QueueFamilyProperties properties{};
        };

    public:
        RhiAdapter(const vk::PhysicalDevice &physical_device) noexcept;

    public:
        [[nodiscard]] forceinline vk::PhysicalDevice VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline const String &GetName() const noexcept
        {
            return m_name;
        }

    public:
        [[nodiscard]] forceinline const Array<vk::QueueFamilyProperties> &GetQueueFamilyProperties() const noexcept
        {
            return m_queue_family_properties;
        }

        [[nodiscard]] forceinline const Array<vk::ExtensionProperties> &GetExtensions() const noexcept
        {
            return m_extensions;
        }

        [[nodiscard]] forceinline const Features &GetFeatures() const noexcept
        {
            return m_features;
        }

        [[nodiscard]] forceinline const Map<ERhiQueueType, QueueInfo> &GetQueues() const noexcept
        {
            return m_queues;
        }

    public:
        [[nodiscard]] forceinline Array<vk::SurfaceFormatKHR> GetSurfaceFormats(const vk::SurfaceKHR &surface) const noexcept
        {
            return m_handle.getSurfaceFormatsKHR(surface);
        }

        [[nodiscard]] forceinline vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities(const vk::SurfaceKHR &surface) const noexcept
        {
            vk::SurfaceCapabilitiesKHR capabilities{};
            VK_VERIFY(m_handle.getSurfaceCapabilitiesKHR(surface, &capabilities), "Failed to get surface caps.");
            return capabilities;
        }

        [[nodiscard]] forceinline bool IsSurfaceSupported(uint32_t queue_index, const vk::SurfaceKHR &surface) const noexcept
        {
            vk::Bool32 is_supported{VK_FALSE};
            VK_VERIFY(m_handle.getSurfaceSupportKHR(queue_index, surface, &is_supported), "Failed to get surface support.");
            return is_supported;
        }

        [[nodiscard]] forceinline Array<vk::PresentModeKHR> GetSurfacePresentModes(const vk::SurfaceKHR &surface) const noexcept
        {
            return m_handle.getSurfacePresentModesKHR(surface);
        }

    public:
        void PrintInfo() const noexcept;

    private:
        String m_name;

    private:
        vk::PhysicalDevice m_handle;

    private:
        Array<vk::QueueFamilyProperties> m_queue_family_properties{};
        Array<vk::ExtensionProperties> m_extensions{};

    private:
        Map<ERhiQueueType, QueueInfo> m_queues{};

    private:
        Features m_features{};
    };

}