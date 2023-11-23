#pragma once

namespace Be::Framework::RHI
{

    struct RhiRasterizerState
    {
    public:
        forceinline void SetFillMode(vk::PolygonMode v) noexcept
        {
            if (fill_mode == v)
            {
                return;
            }
            fill_mode = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::PolygonMode GetFillMode() const noexcept
        {
            return fill_mode;
        }

    public:
        forceinline void SetCullMode(vk::CullModeFlagBits v) noexcept
        {
            if (cull_mode == v)
            {
                return;
            }
            cull_mode = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::CullModeFlagBits GetCullMode() const noexcept
        {
            return cull_mode;
        }

    public:
        forceinline void SetFrontFace(vk::FrontFace v) noexcept
        {
            if (front_face == v)
            {
                return;
            }
            front_face = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::FrontFace GetFrontFace() const noexcept
        {
            return front_face;
        }

    public:
        forceinline void SetDepthBias(float v) noexcept
        {
            if (depth_bias == v)
            {
                return;
            }
            depth_bias = v;
            hash = 0;
        }

        [[nodiscard]] forceinline float GetDepthBias() const noexcept
        {
            return depth_bias;
        }

    public:
        forceinline void SetDepthBiasClamp(float v) noexcept
        {
            if (depth_bias_clamp == v)
            {
                return;
            }
            depth_bias_clamp = v;
            hash = 0;
        }

        [[nodiscard]] forceinline float GetDepthBiasClamp() const noexcept
        {
            return depth_bias_clamp;
        }

    public:
        [[nodiscard]] HashValue Hash() const noexcept
        {
            if (size_t(hash) == 0)
            {
                hash = HashOf(fill_mode);
                hash += HashOf(cull_mode);
                hash += HashOf(front_face);
                hash += HashOf(depth_bias);
                hash += HashOf(depth_bias_clamp);
            }
            return hash;
        }

    public:
        [[nodiscard]] auto operator<=>(const RhiRasterizerState &) const noexcept = default;

    private:
        vk::PolygonMode fill_mode{vk::PolygonMode::eFill};
        vk::CullModeFlagBits cull_mode{vk::CullModeFlagBits::eBack};
        vk::FrontFace front_face{vk::FrontFace::eCounterClockwise};

        float depth_bias{0.0f};
        float depth_bias_clamp{0.0f};

    private:
        mutable HashValue hash{0};
    };

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiRasterizerState>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiRasterizerState &v) const noexcept
        {
            return size_t(v.Hash());
        }
    };

}
