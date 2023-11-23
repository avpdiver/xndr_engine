#pragma once

namespace Be::System::Renderer
{

    BIT_ENUM(ETextureManagerFlag, uint8_t, eNone = 0);

    class TextureManager final : public Noncopyable
    {
    public:
        TextureManager(RhiDriver &driver) noexcept;

    public:
        [[nodiscard]] TextureHandle Load(const String &key, InputStream &stream, ETextureManagerFlag flags = ETextureManagerFlag::eNone) noexcept;
        [[nodiscard]] TextureHandle Load(const String &key, const Path &path, ETextureManagerFlag flags = ETextureManagerFlag::eNone) noexcept;

    private:
        void CreateDummyTextures() noexcept;

    private:
        RhiDriver &m_driver;

    private:
        TextureHandle m_dummy_texture;
        RhiImageHandle m_dummy_image;
    };

}