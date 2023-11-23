#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    void Texture::SetImage(const RhiImageHandle &image) noexcept
    {
        PROFILER_SCOPE;
        
        if (m_image == image)
        {
            return;
        }

        m_image = image;
        m_image_srv.Reset();

        if (!m_image)
        {
            return;
        }

        if (!m_sampler)
        {
            m_sampler = m_image->GetDriver().CreateSampler();
        }

        RhiImageViewDesc desc{
            .image = m_image,
            .view_type = vk::DescriptorType::eCombinedImageSampler,
            .dimension = vk::ImageViewType::e2D,
            .bindless = true,
        };

        m_image_srv = m_image->GetDriver().CreateImageView(desc);
        m_image_srv->SetSampler(m_sampler);
    }

}