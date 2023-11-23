#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    Renderer::Renderer(Window &window) noexcept
        : m_window{window}
    {
        RhiDriverCreateInfo ci{
            .window = window,
            .frame_count = 2,
            .vsync = true,
        };

        m_driver = MakeUnique<RhiDriver>(ci);
        m_texture_manager = MakeUnique<TextureManager>(*m_driver);
        m_mesh_manager = MakeUnique<MeshManager>(*m_driver);
        m_model_manager = MakeUnique<ModelManager>(*m_driver, *m_texture_manager, *m_mesh_manager);
    }

    Renderer::~Renderer() noexcept
    {
    }

}