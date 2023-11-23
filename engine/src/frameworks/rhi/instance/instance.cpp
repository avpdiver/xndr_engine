#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI::RhiInstance
{

    static const std::set<String> REQUIRED_LAYERS =
        {
            "VK_LAYER_KHRONOS_validation",
            //"VK_LAYER_RENDERDOC_Capture",
    };

    static const std::set<String> REQUIRED_EXTENSIONS =
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
#ifdef BE_PLATFORM_WINDOWS
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif

#ifdef BE_DEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
#endif
    };

#ifdef BE_DEBUG
    static VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *pUserData) noexcept
    {
        String type;

        switch (messageTypes)
        {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "General";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "Validation";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "Performance";
            break;
        default:
            type = "Unknown";
            break;
        }

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            // LOG_INFO("Vulkan {} message:\n{}", type, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN("Vulkan {} message:\n{}", type, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("Vulkan {} message:\n{}", type, pCallbackData->pMessage);
            break;
        default:
            LOG_INFO("Vulkan {} message:\n{}", type, pCallbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }
#endif

    // --------------------- Members ------------------------------------------------------------------------
    static vk::Instance m_handle{VK_NULL_HANDLE};
    static vk::DynamicLoader m_dl{};
    static Array<RhiAdapter> m_adapters{};
    static bool m_created{false};

#ifdef BE_DEBUG
    static vk::DebugUtilsMessengerEXT m_debug_messenger{VK_NULL_HANDLE};
    static bool m_debug_utils_supported{false};
#endif

    // -------------------- Methods --------------------------------------------------------------------------

    static void EnumerateAdapters() noexcept
    {
        auto adapters = m_handle.enumeratePhysicalDevices();
        m_adapters.reserve(adapters.size());
        for (const auto &a : adapters)
        {
            m_adapters.emplace_back(a);
        }
    }

    void Create() noexcept
    {
        if (m_created)
        {
            return;
        }

        auto vkGetInstanceProcAddr = m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

#ifdef BE_DEBUG
        auto layers = vk::enumerateInstanceLayerProperties();

        LOG_INFO("Vulkan Instance validation layers:");
        Array<const char *> found_layers;
        for (const auto &layer : layers)
        {
            if (REQUIRED_LAYERS.count(layer.layerName.data()))
            {
                LOG_INFO("\t{}", layer.layerName.data());
                found_layers.push_back(layer.layerName);
            }
        }

        Array<vk::ValidationFeatureEnableEXT> enables{
            // vk::ValidationFeatureEnableEXT::eGpuAssisted,
            // vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot,
            vk::ValidationFeatureEnableEXT::eBestPractices,
            vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
        };

        vk::ValidationFeaturesEXT features{
            .enabledValidationFeatureCount = uint32_t(enables.size()),
            .pEnabledValidationFeatures = enables.data(),
        };

#endif

        auto extensions = vk::enumerateInstanceExtensionProperties();

        LOG_INFO("Vulkan Instance extensions:");
        Array<const char *> found_extension;
        for (const auto &extension : extensions)
        {
            if (REQUIRED_EXTENSIONS.count(extension.extensionName.data()))
            {
                LOG_INFO("\t{}", extension.extensionName.data());
                found_extension.push_back(extension.extensionName);
            }
        }

        vk::ApplicationInfo app_info{};
        app_info.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo create_info{
#ifdef BE_DEBUG
            .pNext = &features,
#endif
            .pApplicationInfo = &app_info,
#ifdef BE_DEBUG
            .enabledLayerCount = uint32_t(found_layers.size()),
            .ppEnabledLayerNames = found_layers.data(),
#endif
            .enabledExtensionCount = uint32_t(found_extension.size()),
            .ppEnabledExtensionNames = found_extension.data(),
        };

        m_handle = vk::createInstance(create_info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_handle);
    
#ifdef BE_DEBUG
        vk::DebugUtilsMessengerCreateInfoEXT msgr_create_info{
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = DebugMessengerCallback,
        };
        m_debug_messenger = m_handle.createDebugUtilsMessengerEXT(msgr_create_info);
        m_debug_utils_supported = true;
#endif

        EnumerateAdapters();

        m_created = true;

        LOG_INFO("RhiInstance is created.");
    }

    void Destroy() noexcept
    {
        m_adapters.clear();
#ifdef BE_DEBUG
        m_handle.destroyDebugUtilsMessengerEXT(m_debug_messenger);
#endif
        m_handle.destroy();
        m_handle = VK_NULL_HANDLE;

        m_created = false;

        LOG_INFO("RhiInstance is destroyed.");
    }

    vk::Instance VkHandle() noexcept
    {
        return m_handle;
    }

    const RhiAdapter &ChooseAdapter(const Array<const char *> &required_extensions) noexcept
    {
        for (const auto &adapter : m_adapters)
        {
            uint32_t found{0};

            for (const auto &required : required_extensions)
            {
                for (const auto &ext : adapter.GetExtensions())
                {
                    if (strcmp(required, ext.extensionName) == 0)
                    {
                        found++;
                        break;
                    }
                }
            }

            if (found == required_extensions.size())
            {
                adapter.PrintInfo();
                return adapter;
            }
        }

        FATAL("There is no GPU adapter that supports required extensions.");
    }

    bool IsDebugUtilsSupported() noexcept
    {
#ifdef BE_DEBUG
        return m_debug_utils_supported;
#else
        return false;
#endif
    }

}