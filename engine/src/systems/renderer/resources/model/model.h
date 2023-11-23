#pragma once

namespace Be::System::Renderer
{

    using VertexPositionType = UInt2;
    using VertexNormalType = UInt2;
    using VertexColorType = uint32_t;
    using VertexUVType = uint32_t;

    class Model final : public RefCounter
    {
    private:
        MeshHandle m_mesh;
        Array<TextureHandle> m_textures;
        Array<Material> m_materials;
        Array<uint32_t> m_submesh_materials;

        friend class ModelManager;
    };

    DEFINE_RENDERER_HANDLE(Model);

}

#include "systems/renderer/resources/model/model_manager.h"