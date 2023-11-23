#pragma once

namespace Be::System::Renderer
{
    
    class Texture final : public RefCounter
    {
    private:
        Texture() noexcept = default;

    public:
        [[nodiscard]] forceinline const RhiImageHandle &GetImage() const noexcept
        {
            return m_image;
        }

        [[nodiscard]] forceinline const RhiImageViewHandle &GetShaderView() const noexcept
        {
            return m_image_srv;
        }

        [[nodiscard]] forceinline const RhiSamplerHandle &GetSampler() const noexcept
        {
            return m_sampler;
        }

    public:
        [[nodiscard]] forceinline bool IsImageValid() const noexcept
        {
            return bool(m_image);
        }

    private:
        void SetImage(const RhiImageHandle &image) noexcept;

    private:
        RhiImageHandle m_image;
        RhiImageViewHandle m_image_srv;
        RhiSamplerHandle m_sampler;

    private:
        Array<RhiSubresourceData> m_data{};

        friend class TextureManager;
    };

    DEFINE_RENDERER_HANDLE(Texture);
}

#include "systems/renderer/resources/texture/texture_manager.h"