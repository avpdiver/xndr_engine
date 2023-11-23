#pragma once

namespace Be::Framework::RHI
{

    struct RhiImageDesc
    {
        ERhiImageType type{ERhiImageType::e2D};

        ERhiBindFlagBits bind_flag{ERhiBindFlag::eUnknown};
        vk::Format format{vk::Format::eUndefined};

        uint32_t width{1};
        uint32_t height{1};
        uint32_t depth{1};
        uint32_t mip_levels{1};
        uint32_t sample_count{1};

        ERhiResourceUsage mem_usage{ERhiResourceUsage::eDefault};
        ERhiResourceState initial_state{ERhiResourceState::eUndefined};

        Array<RhiSubresourceData> data{};
        bool generate_mips{false};

        String debug_name;
    };

    class RhiImage final : public RhiMemoryResource
    {
    protected:
        RhiImage(RhiDriver &driver, const RhiImageDesc &desc) noexcept;

    public:
        RhiImage(RhiDriver &driver, vk::Image image, vk::Format format, uint32_t width, uint32_t height) noexcept;

        ~RhiImage();

    public:
        [[nodiscard]] forceinline vk::Image VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eImage;
        }

    public:
        [[nodiscard]] forceinline vk::Format GetFormat() const noexcept
        {
            return m_format;
        }

        [[nodiscard]] forceinline uint32_t GetWidth() const noexcept
        {
            return m_size.width;
        }

        [[nodiscard]] forceinline uint32_t GetHeight() const noexcept
        {
            return m_size.height;
        }

        [[nodiscard]] forceinline uint32_t GetDepth() const noexcept
        {
            return m_size.depth;
        }

        [[nodiscard]] forceinline uint32_t GetArrayLayers() const noexcept
        {
            return m_array_layers;
        }

        [[nodiscard]] forceinline uint32_t GetMipLevels() const noexcept
        {
            return m_mip_levels;
        }

        [[nodiscard]] forceinline uint32_t GetSampleCount() const noexcept
        {
            return m_sample_count;
        }

        [[nodiscard]] forceinline bool IsBackBuffer() const noexcept
        {
            return m_back_buffer;
        }

        vk::ImageLayout GetLayout() const noexcept;

    private:
        vk::Image m_handle{VK_NULL_HANDLE};

    private:
        vk::Format m_format{vk::Format::eUndefined};
        vk::Extent3D m_size{};
        uint32_t m_mip_levels{1};
        uint32_t m_array_layers{1};
        uint32_t m_sample_count{1};

    private:
        bool m_back_buffer{false};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiImage);

    [[nodiscard]] forceinline uint32_t MipLevel(uint32_t w, uint32_t h) noexcept
    {
        return uint32_t(std::floor(std::log2(std::max(w, h))));
    }

    [[nodiscard]] forceinline uint32_t MipLevelsCount(uint32_t w, uint32_t h) noexcept
    {
        return MipLevel(w, h) + 1u;
    }
}