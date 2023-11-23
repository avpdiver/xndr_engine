#pragma once

namespace Be::Framework::RHI
{

    struct RhiDynamicRenderingState
    {
    public:
        forceinline void SetColorFormat(uint32_t index, vk::Format v) noexcept
        {
            if (color_attachments[index] == v)
            {
                return;
            }
            color_attachments[index] = v;
            hash = 0;
        }

        [[nodiscard]] forceinline const FixedArray<vk::Format, 8> &GetColorFormats() const noexcept
        {
            return color_attachments;
        }

    public:
        forceinline void SetDepthFormat(vk::Format v) noexcept
        {
            if (depth_format == v)
            {
                return;
            }
            depth_format = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::Format GetDepthFormat() const noexcept
        {
            return depth_format;
        }

    public:
        forceinline void SetStencilFormat(vk::Format v) noexcept
        {
            if (stencil_format == v)
            {
                return;
            }
            stencil_format = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::Format GetStencilFormat() const noexcept
        {
            return stencil_format;
        }

    public:
        [[nodiscard]] HashValue Hash() const noexcept
        {
            if (size_t(hash) == 0)
            {
                hash = HashOf(color_attachments.data(), color_attachments.size());
                hash += HashOf(depth_format);
                hash += HashOf(stencil_format);
            }
            return hash;
        }

    private:
        FixedArray<vk::Format, 8> color_attachments{vk::Format::eUndefined};
        vk::Format depth_format{vk::Format::eUndefined};
        vk::Format stencil_format{vk::Format::eUndefined};

    private:
        mutable HashValue hash{0};
    };

}


namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiDynamicRenderingState>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiDynamicRenderingState &v) const noexcept
        {
            return size_t(Be::HashOf(&v, sizeof(Be::Framework::RHI::RhiDynamicRenderingState)));
        }
    };

}
