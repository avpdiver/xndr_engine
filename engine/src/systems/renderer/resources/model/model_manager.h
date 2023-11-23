#pragma once

namespace Be::System::Renderer
{

    BIT_ENUM(EModelManagerFlag, uint8_t, eNone = 0);

    class ModelManager final : public Noncopyable
    {
    public:
        ModelManager(RhiDriver &driver, TextureManager &texture_manager, MeshManager &mesh_manager) noexcept;

    public:
        [[nodiscard]] ModelHandle Load(const String &key, InputStream &stream, EModelManagerFlag flags = EModelManagerFlag::eNone) noexcept;
        [[nodiscard]] ModelHandle Load(const String &key, const Path &path, EModelManagerFlag flags = EModelManagerFlag::eNone) noexcept;

    private:
        RhiDriver &m_driver;
        TextureManager &m_texture_manager;
        MeshManager &m_mesh_manager;
    };

}