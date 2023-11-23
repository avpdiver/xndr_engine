#pragma once

namespace Be::System::Renderer::Pipelines
{
    struct ForwardPipelineDesc
    {
        RhiDriver &rhi_driver;
        usize_t render_queue_memory_size{1'000'000};
    };

    class ForwardPipeline final : public Noncopyable
    {
    public:
        static constexpr RenderGroupHandle OPAQUE_GROUP{"Opaque"};
        static constexpr RenderGroupHandle TRANSPARENT_GROUP{"Transparent"};

    public:
        ForwardPipeline(const ForwardPipelineDesc &desc) noexcept;
        ~ForwardPipeline() noexcept;

    public:
        void BeginFrame() noexcept;
        void EndFrame() noexcept;

    private:
        void CreateRenderQueue(const ForwardPipelineDesc &desc) noexcept;

    private:
        RhiDriver &m_driver;

    private:
        UniquePtr<MallocMemoryArena> m_render_queue_mem_arena{nullptr};
        UniquePtr<MemoryAllocator> m_render_queue_allocator{nullptr};
        UniquePtr<RenderQueue> m_render_queue{nullptr};
    };

}