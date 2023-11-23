#pragma once

namespace Be::Framework::RHI
{

    struct RhiShaderInputParameter
    {
        uint32_t binding{0};
        uint32_t location{0};
        String semantic_name{};
        vk::Format format{vk::Format::eUndefined};
        uint32_t stride{0};
    };

    class RhiShader final : public RhiResource
    {
    protected:
        RhiShader(RhiDriver &driver, const Array<uint32_t> &spirv, vk::ShaderStageFlagBits shader_stage) noexcept;

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eShader;
        }

    public:
        [[nodiscard]] forceinline vk::ShaderStageFlagBits GetStage() const noexcept
        {
            return m_shader_stage;
        }

        [[nodiscard]] forceinline const Array<uint32_t> &GetSpirv() const noexcept
        {
            return m_spirv;
        }

        [[nodiscard]] forceinline const Array<RhiShaderEntryPoint> &GetEntryPoints() const noexcept
        {
            return m_entry_points;
        }

        [[nodiscard]] forceinline const Array<RhiShaderInputParameter> &GetInputParameters() const noexcept
        {
            return m_input_params;
        }

        [[nodiscard]] forceinline const Array<RhiShaderBindingDesc> &GetBindings() const noexcept
        {
            return m_bindings;
        }

        [[nodiscard]] forceinline const Array<vk::VertexInputAttributeDescription> &GetVertexInputAttributes() const noexcept
        {
            return m_vertex_input_attributes_desc;
        }

        [[nodiscard]] forceinline const Array<vk::VertexInputBindingDescription> &GetVertexInputBindings() const noexcept
        {
            return m_vertex_input_bindings_desc;
        }

        [[nodiscard]] forceinline size_t GetHash() const noexcept
        {
            return m_hash;
        }

    private:
        vk::ShaderStageFlagBits m_shader_stage;

    private:
        Array<uint32_t> m_spirv;
        Array<RhiShaderEntryPoint> m_entry_points;

    private:
        Array<RhiShaderBindingDesc> m_bindings;
        Array<RhiShaderInputParameter> m_input_params;

    private:
        Array<vk::VertexInputAttributeDescription> m_vertex_input_attributes_desc;
        Array<vk::VertexInputBindingDescription> m_vertex_input_bindings_desc;

    private:
        size_t m_hash{0};

         friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiShader);

    inline bool operator==(const RhiShaderInputParameter &lhs, const RhiShaderInputParameter &rhs)
    {
        return std::tie(lhs.binding, lhs.location) == std::tie(rhs.binding, rhs.location);
    }

    inline bool operator<(const RhiShaderInputParameter &lhs, const RhiShaderInputParameter &rhs)
    {
        return std::tie(lhs.binding, lhs.location) < std::tie(rhs.binding, rhs.location);
    }

}
