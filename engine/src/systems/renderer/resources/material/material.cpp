#include "systems/renderer/renderer.h"

namespace Be::System::Renderer
{

    static usize_t MaterialProperyTypeSize(EMaterialProperyType t) noexcept
    {
        switch (t)
        {
        case EMaterialProperyType::eTex2d:
        case EMaterialProperyType::eTex3d:
            return sizeof(RhiBindlessIndex);

        case EMaterialProperyType::eFloat:
            return sizeof(float);

        case EMaterialProperyType::eColor:
        case EMaterialProperyType::eVec4:
            return sizeof(Float4);

        default:
            FATAL("Invalid EMaterialProperyType");
            return 0;
        }
    }

    Material::Material() noexcept
    {
        PROFILER_SCOPE;

        usize_t size{0};
        m_data.resize(size);
    }

    Material::~Material()
    {
        PROFILER_SCOPE;
    }

    void Material::SetProgram(const RhiProgramHandle &prog) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(prog);

        m_rhi_program = prog;

        Set<RhiProgramBindingName> binding_names{};
        m_rhi_program->GetBindingNames(binding_names);
    }

    void Material::SetProperty(uint32_t index, const TextureHandle &value) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(index < m_inner_properties.size());

        auto &p = m_inner_properties[index];
        ASSERT(p.type == EMaterialProperyType::eTex2d || p.type == EMaterialProperyType::eTex3d);

        *((RhiBindlessIndex *)(m_data.data() + p.offset)) = value->GetShaderView()->GetDescriptorIndex();

        p.texture = value;
        p.image_view.Reset();
    }

    void Material::SetProperty(uint32_t index, const RhiImageViewHandle &value) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(index < m_inner_properties.size());

        auto &p = m_inner_properties[index];
        ASSERT(p.type == EMaterialProperyType::eTex2d || p.type == EMaterialProperyType::eTex3d);

        *((RhiBindlessIndex *)(m_data.data() + p.offset)) = value->GetDescriptorIndex();

        p.texture.Reset();
        p.image_view = value;
    }

    void Material::SetProperty(uint32_t index, RhiBindlessIndex value) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(index < m_inner_properties.size());

        auto &p = m_inner_properties[index];
        ASSERT(p.type == EMaterialProperyType::eTex2d || p.type == EMaterialProperyType::eTex3d);

        *((RhiBindlessIndex *)(m_data.data() + p.offset)) = value;

        p.texture.Reset();
        p.image_view.Reset();
    }

    void Material::SetProperty(uint32_t index, float value) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(index < m_inner_properties.size());

        const auto &p = m_inner_properties.at(index);
        ASSERT(p.type == EMaterialProperyType::eFloat);

        *((float *)(m_data.data() + p.offset)) = value;
    }

    void Material::SetProperty(uint32_t index, const Float4 &value) noexcept
    {
        PROFILER_SCOPE;

        ASSERT(index < m_inner_properties.size());

        const auto &p = m_inner_properties.at(index);
        ASSERT(p.type == EMaterialProperyType::eVec4 || p.type == EMaterialProperyType::eColor);

        MemCopy(m_data.data() + p.offset, &value, sizeof(Float4));
    }
}