#pragma once

namespace Be::Framework::RHI
{

    class RhiSwapchain final
    {
    public:
        struct BackBuffer
        {
            RhiImageHandle image;
            RhiImageViewHandle input_attachment_view;
        };

    public:
        RhiSwapchain(RhiDriver &driver, uint32_t frame_count, bool vsync) noexcept;

    public:
        ~RhiSwapchain() noexcept;

    public:
        [[nodiscard]] forceinline vk::Format GetFormat() const noexcept
        {
            return m_swapchain_color_format;
        }

        [[nodiscard]] forceinline const UInt2 &GetSize() const noexcept
        {
            return m_size;
        }

    public:
        [[nodiscard]] forceinline uint32_t GetBackBuffersCount() const noexcept
        {
            return uint32_t(m_back_buffers.size());
        }

        [[nodiscard]] forceinline const BackBuffer &GetBackBuffer(uint32_t index) const noexcept
        {
            return m_back_buffers[index];
        }

        [[nodiscard]] forceinline const BackBuffer &GetCurrentBackBuffer() const noexcept
        {
            return m_back_buffers[m_frame_index];
        }

        [[nodiscard]] forceinline uint32_t GetCurrentBackBufferIndex() const noexcept
        {
            return m_frame_index;
        }

        [[nodiscard]] forceinline const RhiSemaphoreHandle &GetAcquireSemaphore() const noexcept
        {
            return m_acquire_semaphore;
        }

    public:
        void AddPresetWaitSemaphore(const RhiSemaphore &semaphore) noexcept;

    protected:
        uint32_t NextImage() noexcept;
        void Present() noexcept;
        void Update() noexcept;

    private:
        void Create() noexcept;

    private:
        RhiDriver &m_driver;

    private:
        vk::SwapchainKHR m_handle{VK_NULL_HANDLE};
        vk::SurfaceKHR m_surface{VK_NULL_HANDLE};

    private:
        vk::Format m_swapchain_color_format{vk::Format::eUndefined};
        UInt2 m_size{};

    private:
        Array<BackBuffer> m_back_buffers;

    private:
        Array<vk::Semaphore> m_present_wait_semaphores;
        RhiSemaphoreHandle m_acquire_semaphore;

    private:
        uint32_t m_back_buffers_count{0};
        uint32_t m_frame_index{0};
        bool m_vsync{false};
        
        friend class RhiDriver;
    };

}
