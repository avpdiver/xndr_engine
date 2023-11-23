#pragma once

namespace Be::Framework::RHI
{

    struct RhiProgramBinding
    {
        uint32_t set{0};
        uint32_t binding{0};
        uint32_t count{1};
        vk::DescriptorType descriptor_type{};
        vk::ShaderStageFlags shader_stage{};
    };
    using RhiProgramBindingName = NamedHandle<RhiProgramBinding, 64, BE_OPTIMIZE_IDS>;

    struct RhiDescriptorSetPool
    {
        vk::DescriptorPool pool{VK_NULL_HANDLE};
        vk::DescriptorSet set{VK_NULL_HANDLE};
    };

    struct RhiWriteBindingDesc
    {
        RhiProgramBinding binding;
        const RhiResourceView *view;
    };

    class RhiProgram final : public RhiResource
    {
    public:
        static constexpr FixedArray<vk::ShaderStageFlagBits, 5> SHADER_STAGES{
            vk::ShaderStageFlagBits::eVertex,
            vk::ShaderStageFlagBits::eFragment,
            vk::ShaderStageFlagBits::eCompute,
            vk::ShaderStageFlagBits::eTaskEXT,
            vk::ShaderStageFlagBits::eMeshEXT,
        };

    public:
        RhiProgram(RhiDriver &driver, const Array<RhiShaderHandle> &shaders) noexcept;

    public:
        ~RhiProgram() noexcept;

    public:
        [[nodiscard]] forceinline ERhiResourceType GetResourceType() const noexcept override
        {
            return ERhiResourceType::eProgram;
        }

    public:
        [[nodiscard]] forceinline const Array<vk::ShaderEXT> &GetShaders() const noexcept
        {
            return m_shaders;
        }

    public:
        [[nodiscard]] forceinline uint32_t GetBindingMask(uint8_t set) const noexcept
        {
            return m_binding_masks[set];
        }

        [[nodiscard]] forceinline const RhiProgramBinding &GetBinding(uint8_t set, uint8_t binding) const noexcept
        {
            return m_bindings[set][binding];
        }

        [[nodiscard]] forceinline const RhiProgramBinding &GetBinding(const RhiProgramBindingName &name) const noexcept
        {
            PROFILER_SCOPE;
            
            ASSERT(m_binding_names.contains(name));
            const auto &p = m_binding_names.at(name);
            return m_bindings[p.first][p.second];
        }

        void GetBindingNames(OUT Set<RhiProgramBindingName>& names) const noexcept;

    public:
        [[nodiscard]] forceinline const Array<vk::DescriptorSet> &GetDescriptorSets() const noexcept
        {
            return m_descriptor_sets;
        }

        [[nodiscard]] forceinline vk::PipelineLayout GetPipelineLayout() const noexcept
        {
            return m_pipeline_layout;
        }

        [[nodiscard]] forceinline vk::PipelineBindPoint GetPipelineBindPoint() const noexcept
        {
            return m_pipeline_bind_point;
        }

    public:
        void WriteBinding(const RhiWriteBindingDesc &binding) const noexcept;
        void WriteBindings(const Array<RhiWriteBindingDesc> &bindings) const noexcept;

    public:
        [[nodiscard]] forceinline HashValue Hash() const noexcept
        {
            return m_hash;
        }

    private:
        void CreateDescriptorSets(const Array<RhiShaderHandle> &shaders) noexcept;
        void CreateShaders(const Array<RhiShaderHandle> &shaders) noexcept;

    private:
        Array<vk::ShaderEXT> m_shaders;

    private:
        RhiProgramBinding m_bindings[MAX_DESCRIPTOR_SETS][MAX_BINDINGS]; // [set, binding]
        uint32_t m_binding_masks[MAX_DESCRIPTOR_SETS];                   // [set]
        Map<RhiProgramBindingName, Pair<uint32_t, uint32_t>> m_binding_names;

    private:
        vk::PipelineBindPoint m_pipeline_bind_point;
        vk::PipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

    private:
        Array<vk::DescriptorSetLayout> m_descriptor_set_layouts;
        Array<vk::DescriptorSet> m_descriptor_sets;
        Array<RhiDescriptorSetPool> m_descriptors;

    private:
        HashValue m_hash{0};

        friend class RhiDriver;
    };

    DEFINE_RHI_HANDLE(RhiProgram);

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiProgram>
    {
        [[nodiscard]] size_t operator()(const Be::Framework::RHI::RhiProgram &v) const
        {
            return size_t(v.Hash());
        }
    };

}