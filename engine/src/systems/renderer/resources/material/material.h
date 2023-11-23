#pragma once

namespace Be::System::Renderer
{

    ITERABLE_ENUM(EMaterialProperyType, uint8_t,
                  eTex2d,
                  eTex3d,
                  eFloat,
                  eColor,
                  eVec4);

    using MaterialPropertyName = FixedString<64>;

    struct MaterialProperty final
    {
    public:
        MaterialProperty() noexcept = default;

    public:
        MaterialPropertyName name;
        EMaterialProperyType type;
        union
        {
            uint32_t uint_value;
            int32_t sint_value;
            float float_value;
            float color_value[4];
            float vec4_value[4];
        };
    };

    using RenderGroupName = FixedString<RenderGroupHandle::Size>;
    struct MaterialBlueprint final
    {
        RenderGroupName render_group{};
        Array<MaterialProperty> properties{};
    };

    class Material final
    {
    private:
        struct InnerProperty
        {
            EMaterialProperyType type;
            uint32_t offset;

            TextureHandle texture;
            RhiImageViewHandle image_view;
        };

    public:
        Material() noexcept;
        ~Material() noexcept;

    public:
        void SetProgram(const RhiProgramHandle &prog) noexcept;

    public:
        void SetProperty(uint32_t index, const TextureHandle &value) noexcept;
        void SetProperty(uint32_t index, const RhiImageViewHandle &value) noexcept;
        void SetProperty(uint32_t index, RhiBindlessIndex value) noexcept;
        void SetProperty(uint32_t index, float value) noexcept;
        void SetProperty(uint32_t index, const Float4 &value) noexcept;

    public:
        forceinline void SetRenderGroup(const RenderGroupHandle &group) noexcept
        {
            m_render_group = group;
        }

    public:
        [[nodiscard]] forceinline const void *GetData() const noexcept
        {
            return m_data.data();
        }

    public:
        [[nodiscard]] forceinline const RenderGroupHandle &GetRenderGroup() const noexcept
        {
            return m_render_group;
        }

    private:
        RhiProgramHandle m_rhi_program;

    private:
        Array<InnerProperty> m_inner_properties;
        Array<byte_t> m_data{};

    private:
        RenderGroupHandle m_render_group;
    };

}