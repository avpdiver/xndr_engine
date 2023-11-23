#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiSampler::RhiSampler(RhiDriver &driver, const RhiSamplerDesc &desc) noexcept
        : RhiResource{driver}
    {
        PROFILER_SCOPE;

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.anisotropyEnable = true;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = std::numeric_limits<float>::max();
        samplerInfo.addressModeU = desc.mode;
        samplerInfo.addressModeV = desc.mode;
        samplerInfo.addressModeW = desc.mode;
        samplerInfo.compareOp = desc.func;
        samplerInfo.compareEnable = (desc.func != vk::CompareOp::eNever);

        m_handle = m_device.VkHandle().createSampler(samplerInfo);
    }

    RhiSampler::~RhiSampler() noexcept
    {
        PROFILER_SCOPE;
        
        if (m_handle)
        {
            m_driver.Delete(m_handle);
        }
    }

}
