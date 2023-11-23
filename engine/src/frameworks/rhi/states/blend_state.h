#pragma once

namespace Be::Framework::RHI
{

    struct RhiBlendState
    {
    public:
        forceinline void SetLogicOpEnabled(bool v) noexcept
        {
            if (logic_op_enabled == v)
            {
                return;
            }
            logic_op_enabled = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsLogicOpEnabled() const noexcept
        {
            return logic_op_enabled;
        }

    public:
        forceinline void SetLogicOp(vk::LogicOp v) noexcept
        {
            if (logic_op == v)
            {
                return;
            }
            logic_op = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::LogicOp GetLogicOp() const noexcept
        {
            return logic_op;
        }

    public:
        forceinline void SetBlendEnabled(uint32_t a, bool v) noexcept
        {
            if (blend_enable[a] == v)
            {
                return;
            }
            blend_enable[a] = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsBlendEnabled(uint32_t a) const noexcept
        {
            return blend_enable[a];
        }

        [[nodiscard]] forceinline const VkBool32 *IsBlendsEnabled() const noexcept
        {
            return (VkBool32 *)blend_enable.data();
        }

    public:
        forceinline void SetBlendEquation(uint32_t a,
                                          vk::BlendFactor src_factor, vk::BlendFactor dst_factor, vk::BlendOp op,
                                          vk::BlendFactor src_alpha_factor, vk::BlendFactor dst_alpha_factor, vk::BlendOp alpha_op) noexcept
        {
            auto &b = blend_equations[a];
            b.srcColorBlendFactor = src_factor;
            b.dstColorBlendFactor = dst_factor;
            b.colorBlendOp = op;

            b.srcAlphaBlendFactor = src_alpha_factor;
            b.dstAlphaBlendFactor = dst_alpha_factor;
            b.alphaBlendOp = alpha_op;

            hash = 0;
        }

        [[nodiscard]] forceinline const vk::ColorBlendEquationEXT &GetBlendEquation(uint32_t a) const noexcept
        {
            return blend_equations[a];
        }

        [[nodiscard]] forceinline const vk::ColorBlendEquationEXT *GetBlendEquations() const noexcept
        {
            return blend_equations.data();
        }

    public:
        forceinline void SetColorWriteMask(uint32_t a, vk::ColorComponentFlags v) noexcept
        {
            if (color_write_mask[a] == v)
            {
                return;
            }
            color_write_mask[a] = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::ColorComponentFlags GetColorWriteMask(uint32_t a) const noexcept
        {
            return color_write_mask[a];
        }

        [[nodiscard]] forceinline const vk::ColorComponentFlags *GetColorWriteMasks() const noexcept
        {
            return color_write_mask.data();
        }

    public:
        HashValue Hash() const noexcept
        {
            if (size_t(hash) == 0)
            {
                hash = HashOf(logic_op_enabled);
                if (logic_op_enabled)
                {
                    hash += HashOf(logic_op);
                }
                for (uint32_t a = 0; a < MAX_COLOR_ATTACHMENTS; a++)
                {
                    hash += HashOf(blend_enable[a]);
                    hash += HashOf(blend_equations[a].colorBlendOp);
                    hash += HashOf(blend_equations[a].srcColorBlendFactor);
                    hash += HashOf(blend_equations[a].dstColorBlendFactor);
                    hash += HashOf(blend_equations[a].alphaBlendOp);
                    hash += HashOf(blend_equations[a].srcAlphaBlendFactor);
                    hash += HashOf(blend_equations[a].dstAlphaBlendFactor);
                    hash += HashOf(color_write_mask[a].m_mask);
                }
            }
            return hash;
        }

    public:
        [[nodiscard]] auto operator<=>(const RhiBlendState &) const noexcept = default;

    private:
        bool logic_op_enabled{false};
        vk::LogicOp logic_op{vk::LogicOp::eNoOp};

        FixedArray<bool, MAX_COLOR_ATTACHMENTS> blend_enable = MakeFixedArray<bool, MAX_COLOR_ATTACHMENTS>(false);
        FixedArray<vk::ColorBlendEquationEXT, MAX_COLOR_ATTACHMENTS> blend_equations = MakeFixedArray<vk::ColorBlendEquationEXT, MAX_COLOR_ATTACHMENTS>({});
        FixedArray<vk::ColorComponentFlags, MAX_COLOR_ATTACHMENTS> color_write_mask = MakeFixedArray<vk::ColorComponentFlags, MAX_COLOR_ATTACHMENTS>(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);

    private:
        mutable HashValue hash{0};
    };

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiBlendState>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiBlendState &v) const noexcept
        {
            return size_t(Be::HashOf(&v, sizeof(v)));
        }
    };

}