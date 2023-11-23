#pragma once

namespace Be::Framework::RHI
{

    struct RhiShaderEntryPoint
    {
        String name{};
        vk::ShaderStageFlagBits stage{};
        uint32_t payload_size{0};
        uint32_t attribute_size{0};
    };

    struct RhiShaderInputParameterDesc
    {
        uint32_t binding{0};
        uint32_t location{0};
        vk::Format format{};
        String semantic_name{};
    };

    struct RhiShaderOutputParameterDesc
    {
        uint32_t binding{0};
    };

    struct RhiShaderBindingDesc
    {
        String name{};
        vk::DescriptorType type{};
        uint32_t set{};
        uint32_t binding{};
        uint32_t count{};
        vk::ImageViewType dimension{};
        uint32_t structure_stride{};
    };

    struct RhiShaderFeatureInfo
    {
        bool resource_descriptor_heap_indexing{false};
        bool sampler_descriptor_heap_indexing{false};
        FixedArray<uint32_t, 3> numthreads{};
    };

    class RhiShaderReflection final
    {
    public:
        static RhiShaderReflection Create(const Array<uint32_t> &spirv) noexcept;

    private:
        RhiShaderReflection(const Array<uint32_t> &spirv) noexcept;

    public:
        [[nodiscard]] const Array<RhiShaderEntryPoint> &GetEntryPoints() const noexcept;
        [[nodiscard]] const Array<RhiShaderInputParameterDesc> &GetInputParameters() const noexcept;
        [[nodiscard]] const Array<RhiShaderOutputParameterDesc> &GetOutputParameters() const noexcept;
        [[nodiscard]] const Array<RhiShaderBindingDesc> &GetBindings() const noexcept;

    public:
        const RhiShaderFeatureInfo &GetShaderFeatureInfo() const noexcept;

    private:
        Array<uint32_t> m_blob;
        Array<RhiShaderEntryPoint> m_entry_points;
        Array<RhiShaderBindingDesc> m_bindings;
        Array<RhiShaderInputParameterDesc> m_input_parameters;
        Array<RhiShaderOutputParameterDesc> m_output_parameters;

    private:
        RhiShaderFeatureInfo m_shader_feature_info{};
    };

    inline bool operator==(const RhiShaderEntryPoint &lhs, const RhiShaderEntryPoint &rhs)
    {
        return std::tie(lhs.name, lhs.stage) == std::tie(rhs.name, rhs.stage);
    }

    inline bool operator<(const RhiShaderEntryPoint &lhs, const RhiShaderEntryPoint &rhs)
    {
        return std::tie(lhs.name, lhs.stage) < std::tie(rhs.name, rhs.stage);
    }

    inline auto MakeTie(const RhiShaderBindingDesc &desc)
    {
        return std::tie(desc.name, desc.type, desc.binding, desc.set, desc.dimension);
    };

    inline bool operator==(const RhiShaderBindingDesc &lhs, const RhiShaderBindingDesc &rhs)
    {
        return MakeTie(lhs) == MakeTie(rhs);
    }

    inline bool operator<(const RhiShaderBindingDesc &lhs, const RhiShaderBindingDesc &rhs)
    {
        return MakeTie(lhs) < MakeTie(rhs);
    }

}
