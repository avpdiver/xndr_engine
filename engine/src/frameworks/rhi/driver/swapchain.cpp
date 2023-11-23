#include "frameworks/rhi/rhi.h"
#include "frameworks/rhi/utils/vulkan_helpers.h"

#include <GLFW/glfw3.h>

namespace Be::Framework::RHI
{

    RhiSwapchain::RhiSwapchain(RhiDriver &driver, uint32_t frame_count, bool vsync) noexcept
        : m_driver{driver},
          m_back_buffers_count{frame_count},
          m_vsync{vsync}
    {
        VK_VERIFY(glfwCreateWindowSurface(RhiInstance::VkHandle(), m_driver.GetWindow().Handle(), nullptr, (VkSurfaceKHR *)&m_surface),
                  "Failed to create surface.");

        Create();
    }

    RhiSwapchain::~RhiSwapchain() noexcept
    {
        if (m_handle)
        {
            m_driver.GetDevice().VkHandle().destroy(m_handle);
            m_handle = VK_NULL_HANDLE;
        }
        if (m_surface)
        {
            RhiInstance::VkHandle().destroy(m_surface);
            m_surface = VK_NULL_HANDLE;
        }

        LOG_INFO("RhiSwapchain is destroyed.");
    }

    void RhiSwapchain::Create() noexcept
    {
        PROFILER_SCOPE;

        const auto &device = m_driver.GetDevice();

        const auto &adapter = device.GetAdapter();

        auto surface_capabilities = adapter.GetSurfaceCapabilities(m_surface);
        auto width = surface_capabilities.currentExtent.width;
        auto height = surface_capabilities.currentExtent.height;
        if (m_size.x == width && m_size.y == height)
        {
            return;
        }

        if (m_handle)
        {
            device.VkHandle().destroySwapchainKHR(m_handle);
            m_handle = VK_NULL_HANDLE;
        }

        vk::ColorSpaceKHR color_space{};
        auto surface_formats = adapter.GetSurfaceFormats(m_surface);
        for (const auto &f : surface_formats)
        {
            if (!vkuFormatIsSRGB(VkFormat(f.format)))
            {
                m_swapchain_color_format = f.format;
                color_space = f.colorSpace;
                break;
            }
        }
        VERIFY(m_swapchain_color_format != vk::Format::eUndefined, "Swapchain format is undefined.");

        auto &command_queue = m_driver.GetQueue(ERhiQueueType::eGraphics);

        auto is_supported_surface = adapter.IsSurfaceSupported(command_queue.GetQueueFamilyIndex(), m_surface);
        VERIFY(is_supported_surface, "Surface is not supported");

        auto modes = adapter.GetSurfacePresentModes(m_surface);

        vk::SwapchainCreateInfoKHR swap_chain_create_info{
            .surface = m_surface,
            .minImageCount = m_back_buffers_count,
            .imageFormat = m_swapchain_color_format,
            .imageColorSpace = color_space,
            .imageExtent = vk::Extent2D(width, height),
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .clipped = true,
        };

        if (m_vsync)
        {
            if (std::count(modes.begin(), modes.end(), vk::PresentModeKHR::eFifoRelaxed))
            {
                swap_chain_create_info.presentMode = vk::PresentModeKHR::eFifoRelaxed;
            }
            else
            {
                swap_chain_create_info.presentMode = vk::PresentModeKHR::eFifo;
            }
        }
        else
        {
            if (std::count(modes.begin(), modes.end(), vk::PresentModeKHR::eMailbox))
            {
                swap_chain_create_info.presentMode = vk::PresentModeKHR::eMailbox;
            }
            else
            {
                swap_chain_create_info.presentMode = vk::PresentModeKHR::eImmediate;
            }
        }

        m_handle = device.VkHandle().createSwapchainKHR(swap_chain_create_info);

        auto images = device.VkHandle().getSwapchainImagesKHR(m_handle);
        m_back_buffers.clear();
        m_back_buffers.reserve(m_back_buffers_count);
        for (uint32_t i = 0; i < m_back_buffers_count; ++i)
        {
            auto &bb = m_back_buffers.emplace_back();

            bb.image = new RhiImage(m_driver, images[i], GetFormat(), width, height);

            RhiImageViewDesc view_desc{
                .image = bb.image,
                .view_type = vk::DescriptorType::eInputAttachment,
                .dimension = vk::ImageViewType::e2D,
                .bindless = false,
                .debug_name = "BackBufferInputAttachmentView_" + i,
            };
            bb.input_attachment_view = m_driver.CreateImageView(view_desc);
        }

        m_acquire_semaphore = m_driver.CreateSemaphore();

        m_size.x = width;
        m_size.y = height;

        LOG_INFO("RhiSwapchain is created.");
    }

    void RhiSwapchain::Update() noexcept
    {
        PROFILER_SCOPE;

        Create();
    }

    uint32_t RhiSwapchain::NextImage() noexcept
    {
        PROFILER_SCOPE;

        VK_VERIFY(m_driver.GetDevice().VkHandle().acquireNextImageKHR(
                      m_handle,
                      UINT64_MAX,
                      m_acquire_semaphore->VkHandle(),
                      VK_NULL_HANDLE,
                      &m_frame_index),
                  "Failed to acquire next image.");
        return m_frame_index;
    }

    void RhiSwapchain::AddPresetWaitSemaphore(const RhiSemaphore &semaphore) noexcept
    {
        PROFILER_SCOPE;

        m_present_wait_semaphores.emplace_back(semaphore.VkHandle());
    }

    void RhiSwapchain::Present() noexcept
    {
        PROFILER_SCOPE;
        
        auto queue = m_driver.GetQueue(ERhiQueueType::eGraphics).VkHandle();

        vk::PresentInfoKHR present_info{
            .waitSemaphoreCount = uint32_t(m_present_wait_semaphores.size()),
            .pWaitSemaphores = m_present_wait_semaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = &m_handle,
            .pImageIndices = &m_frame_index,
        };

        auto res = queue.presentKHR(present_info);
        if (res != vk::Result::eSuccess && res != vk::Result::eErrorOutOfDateKHR)
        {
            VK_VERIFY(res, "Failed to present.");
        }

        m_present_wait_semaphores.clear();
    }

}
