#pragma once

namespace Be::Framework::RHI
{

    struct RhiSamplerDesc
    {
        vk::Filter filter{vk::Filter::eLinear};
        vk::SamplerAddressMode mode{vk::SamplerAddressMode::eRepeat};
        vk::CompareOp func{vk::CompareOp::eNever};
    };

    class RhiSampler final : public RhiResource
    {
    public:
        RhiSampler(RhiDriver &driver, const RhiSamplerDesc &desc) noexcept;

    public:
        ~RhiSampler() noexcept;

    public:
        [[nodiscard]] forceinline vk::Sampler VkHandle() const noexcept
        {
            return m_handle;
        }

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eSampler;
        }

    private:
        vk::Sampler m_handle{VK_NULL_HANDLE};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiSampler);

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiSamplerDesc>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiSamplerDesc &v) const
        {
            return size_t(Be::HashOf(&v, sizeof(Be::Framework::RHI::RhiSamplerDesc)));
        }
    };

}
