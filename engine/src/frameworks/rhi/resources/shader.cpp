#include "frameworks/rhi/rhi.h"

namespace Be::Framework::RHI
{

    RhiShader::RhiShader(RhiDriver &driver, const Array<uint32_t> &spirv, vk::ShaderStageFlagBits shader_stage) noexcept
        : RhiResource{driver},
          m_shader_stage{shader_stage},
          m_spirv{spirv}
    {
        PROFILER_SCOPE;
        
        m_hash = size_t(HashOf(m_spirv));

        const auto reflection = RhiShaderReflection::Create(m_spirv);

        m_entry_points = reflection.GetEntryPoints();

        m_bindings = reflection.GetBindings();
        auto &input_parameters = reflection.GetInputParameters();
        for (const auto &ip : input_parameters)
        {
            auto &p = m_input_params.emplace_back();
            p.binding = ip.binding;
            p.location = ip.location;
            p.semantic_name = ip.semantic_name;
            p.format = ip.format;
            p.stride = RhiFormat::BytesPerPixel(p.format);
        }

        std::sort(std::begin(m_input_params), std::end(m_input_params));

        if (!(shader_stage & vk::ShaderStageFlagBits::eVertex))
        {
            return;
        }

        Map<uint32_t, uint32_t> bindings;
        for (const auto &p : m_input_params)
        {
            auto &attribute = m_vertex_input_attributes_desc.emplace_back();

            attribute.location = p.location;
            attribute.binding = p.binding;
            attribute.format = p.format;
            attribute.offset = bindings[attribute.binding];

            bindings[attribute.binding] += p.stride;
        }

        for (const auto &[b, stride] : bindings)
        {
            auto &binding = m_vertex_input_bindings_desc.emplace_back();
            binding.binding = b;
            binding.inputRate = vk::VertexInputRate::eVertex;
            binding.stride = stride;
        }
    }

}
