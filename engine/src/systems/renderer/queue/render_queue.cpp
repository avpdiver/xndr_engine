#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    RenderQueue::RenderQueue(RhiDriver &rhi_driver, MemoryAllocator &allocator, const Array<RenderGroupHandle> &groups)
        : m_rhi_driver{rhi_driver},
          m_render_data_allocator{allocator}
    {
        auto thread_count = ThreadUtils::MaxThreadCount();
        m_thread_contexts.resize(thread_count);

        for (auto &ctx : m_thread_contexts)
        {
            for (const auto &g : groups)
            {
                ctx[g].clear();
            }
        }
    }

    RenderQueue::~RenderQueue()
    {
    }

    void RenderQueue::BeginFrame(const RenderContext &context)
    {
        PROFILER_SCOPE;

        m_context = context;

        for (auto &groups : m_thread_contexts)
        {
            for (auto &[name, pool] : groups)
            {
                pool.clear();
            }
        }
        m_render_data_allocator.Reset();
    }

    void RenderQueue::Render(const RenderGroupHandle &group, RhiCommandBuffer &cmd)
    {
        PROFILER_SCOPE;

        auto &render_data = m_render_groups[group];
        render_data.clear();

        for (auto &groups : m_thread_contexts)
        {
            const auto &g = groups.at(group);
            render_data.insert(render_data.end(), g.begin(), g.end());
        }

        std::sort(render_data.begin(), render_data.end(),
                  [](const auto &a, const auto &b)
                  { return a.sorting_key < b.sorting_key; });

        for (uint32_t i = 0; i < render_data.size(); i++)
        {
            const auto &rd = render_data[i];
            uint32_t instance_count{1};
            i++;
            for (; i < render_data.size() && render_data[i].item_hash == rd.item_hash; i++)
            {
                instance_count++;
            }
            rd.function(cmd, &rd, instance_count);
        }
    }

    void RenderQueue::EndFrame()
    {
        PROFILER_SCOPE;
    }

}