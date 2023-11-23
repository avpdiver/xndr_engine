#pragma once

namespace Be::System::Renderer
{

    BIT_ENUM(EMeshManagerFlag, uint8_t, eNone = 0);

    class MeshManager final : public Noncopyable
    {
    public:
        MeshManager(RhiDriver &driver) noexcept;

    public:
        [[nodiscard]] MeshHandle Load(const String &key, InputStream &stream, EMeshManagerFlag flags = EMeshManagerFlag::eNone) noexcept;
        [[nodiscard]] MeshHandle Load(const String &key, const Path &path, EMeshManagerFlag flags = EMeshManagerFlag::eNone) noexcept;

    private:
        RhiDriver &m_driver;        
    };

}