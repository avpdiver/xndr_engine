#include "systems/renderer/renderer.h"

namespace Be::System::Renderer::Pipelines
{

    ForwardPipeline::ForwardPipeline(const ForwardPipelineDesc &desc) noexcept
        : m_driver{desc.rhi_driver}
    {
        CreateRenderQueue(desc);
    }

    ForwardPipeline::~ForwardPipeline() noexcept
    {
    }

    void ForwardPipeline::CreateRenderQueue(const ForwardPipelineDesc &desc) noexcept
    {
        m_render_queue_mem_arena = MakeUnique<MallocMemoryArena>(desc.render_queue_memory_size);
        m_render_queue_allocator = MakeUnique<LinearAllocatorWaitFree>(*m_render_queue_mem_arena);

        Array<RenderGroupHandle> groups{OPAQUE_GROUP, TRANSPARENT_GROUP};

        m_render_queue = MakeUnique<RenderQueue>(m_driver, *m_render_queue_allocator, groups);
    }

    void ForwardPipeline::BeginFrame() noexcept
    {
        PROFILER_SCOPE;
    }

    void ForwardPipeline::EndFrame() noexcept
    {
        PROFILER_SCOPE;
    }

}