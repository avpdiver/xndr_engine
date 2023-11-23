#pragma once

namespace Be::Framework::RHI
{

    struct RhiImageViewDesc
    {
        RhiImageHandle image{};

        vk::DescriptorType view_type{};
        vk::ImageViewType dimension{vk::ImageViewType::e2D};

        uint32_t base_mip_level{0};
        uint32_t mip_levels{VK_REMAINING_MIP_LEVELS};
        uint32_t base_array_layer{0};
        uint32_t array_layers{VK_REMAINING_ARRAY_LAYERS};
        uint32_t slice{0};

        bool bindless{true};

        String debug_name{};
    };

    // static bool operator<(const VkImageSubresourceRange &lhs, const VkImageSubresourceRange &rhs)
    // {
    //     return std::tie(lhs.aspectMask, lhs.baseArrayLayer, lhs.baseMipLevel, lhs.layerCount, lhs.levelCount) <
    //            std::tie(rhs.aspectMask, rhs.baseArrayLayer, rhs.baseMipLevel, rhs.layerCount, rhs.levelCount);
    // };

    class RhiImageView final : public RhiResourceView
    {
    public:
        RhiImageView(RhiDriver &driver, const RhiImageViewDesc &view_desc) noexcept;

    public:
        ~RhiImageView() noexcept;

    public:
        [[nodiscard]] forceinline vk::ImageView VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eImageView;
        }

    public:
        [[nodiscard]] forceinline bool IsBindless() const noexcept
        {
            return m_desc.bindless;
        }

    public:
        [[nodiscard]] forceinline vk::Format GetFormat() const noexcept
        {
            return m_format;
        }

        [[nodiscard]] forceinline uint32_t GetBaseMipLevel() const noexcept
        {
            return m_desc.base_mip_level;
        }

        [[nodiscard]] forceinline uint32_t GetMipLevels() const noexcept
        {
            return m_desc.mip_levels;
        }

        [[nodiscard]] forceinline uint32_t GetBaseArrayLayer() const noexcept
        {
            return m_desc.base_array_layer;
        }

        [[nodiscard]] forceinline uint32_t GetArrayLayers() const noexcept
        {
            return m_desc.array_layers;
        }

    public:
        [[nodiscard]] forceinline RhiImageHandle GetImage() const noexcept
        {
            return m_image;
        }

    public:
        forceinline void SetSampler(RhiSamplerHandle sampler) noexcept
        {
            PROFILER_SCOPE;
            
            m_sampler = sampler;
            m_descriptor_image.sampler = sampler->VkHandle();
        }

        [[nodiscard]] forceinline RhiSamplerHandle GetSampler() const noexcept
        {
            return m_sampler;
        }

    private:
        vk::ImageView m_handle{VK_NULL_HANDLE};
        vk::DescriptorImageInfo m_descriptor_image{};

    private:
        RhiImageHandle m_image;
        RhiSamplerHandle m_sampler;

    private:
        RhiImageViewDesc m_desc;
        vk::Format m_format{vk::Format::eUndefined};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiImageView);

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiImageViewDesc>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiImageViewDesc &v) const
        {
            return size_t(Be::HashOf(&v, sizeof(Be::Framework::RHI::RhiImageViewDesc)));
        }
    };

}
