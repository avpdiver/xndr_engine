#pragma once

#include "base/base.h"

#include "frameworks/threading/threading.h"
#include "frameworks/rhi/rhi.h"

#include "systems/renderer/renderer_types.h"
#include "systems/renderer/shader_interop/shader_interop.h"
#include "systems/renderer/queue/render_context.h"
#include "systems/renderer/queue/render_queue.h"
#include "systems/renderer/resources/texture/texture.h"
#include "systems/renderer/resources/material/material.h"
#include "systems/renderer/resources/mesh/mesh.h"
#include "systems/renderer/resources/model/model.h"
#include "systems/renderer/pipelines/forward/forward_pipeline.h"

namespace Be::System::Renderer
{

    class Renderer final : public Noncopyable
    {
    public:
        Renderer(Window &window) noexcept;
        ~Renderer() noexcept;

    public:
        [[nodiscard]] forceinline RhiDriver &GetRhiDriver() noexcept
        {
            return *m_driver;
        }

        [[nodiscard]] forceinline TextureManager &GetTextureManager() noexcept
        {
            return *m_texture_manager;
        }

        [[nodiscard]] forceinline ModelManager &GetModelManager() noexcept
        {
            return *m_model_manager;
        }

    private:
        Window &m_window;

    private:
        UniquePtr<RhiDriver> m_driver{nullptr};

    private:
        UniquePtr<TextureManager> m_texture_manager{nullptr};
        UniquePtr<MeshManager> m_mesh_manager{nullptr};
        UniquePtr<ModelManager> m_model_manager{nullptr};
    };

}