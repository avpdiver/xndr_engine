#pragma once

namespace Be::System::Renderer
{

    using RenderGroupHandle = NamedHandle<RenderQueue, 64, BE_OPTIMIZE_IDS>;
    using RenderFunction = void (*)(RhiCommandBuffer &, const RenderableItemData *, uint32_t);

    template <typename T>
    concept RenderableItem = requires(T t, RenderQueue &q, const RenderContext &c, const Matrix4x4 &m) { t.EnqueueToRenderQueue(q, c, m); };

    struct RenderableItemData
    {
        RenderFunction function;
        const void *data;
        HashValue item_hash;
        uint64_t sorting_key; // Lower sorting keys will appear earlier.
    };

    class RenderQueue final : public Noncopyable
    {
    public:
        RenderQueue(RhiDriver &rhi_driver, MemoryAllocator &allocator, const Array<RenderGroupHandle> &groups);
        ~RenderQueue();

    public:
        void BeginFrame(const RenderContext &context);
        void Render(const RenderGroupHandle &group, RhiCommandBuffer &cmd);
        void EndFrame();

    public:
        forceinline void Push(const RenderableItem auto &item, const Matrix4x4 &model_matrix)
        {
            item.EnqueueToRenderQueue(*this, m_context, model_matrix);
        }

    public:
        template <typename T>
        void *Push(const RenderGroupHandle &group, uint64_t renderable_item_key, uint64_t instance_sorting_key, RenderFunction render_func)
        {
            PROFILER_SCOPE;
            
            auto &groups = m_thread_contexts[ThreadUtils::GetCurrentThreadIndex()];
            auto &rd = groups[group].emplace_back();
            rd.function = render_func;
            rd.data = m_render_data_allocator.Alloc(sizeof(T), alignof(T));
            rd.item_hash = HashOf(renderable_item_key) + HashOf(render_func);
            rd.sorting_key = instance_sorting_key;

            return rd.data;
        }

    private:
        RhiDriver &m_rhi_driver;

    private:
        RenderContext m_context;

    private:
        using RenderDataPool = Array<RenderableItemData>;
        using RenderGroups = Map<RenderGroupHandle, RenderDataPool>;
        using ThreadContexts = Array<RenderGroups>;

        ThreadContexts m_thread_contexts{};
        RenderGroups m_render_groups{};

    private:
        Map<uint64_t, void *> m_allocated_renderable_item_data;
        MemoryAllocator &m_render_data_allocator;
    };

}
