#pragma once

namespace Be::Framework::RHI
{

    struct RhiDepthStencilState
    {
    public:
        forceinline void SetDepthTest(bool v) noexcept
        {
            if (depth_test == v)
            {
                return;
            }
            depth_test = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsDepthTest() const noexcept
        {
            return depth_test;
        }

    public:
        forceinline void SetDepthWrite(bool v) noexcept
        {
            if (depth_write == v)
            {
                return;
            }
            depth_write = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsDepthWrite() const noexcept
        {
            return depth_write;
        }

    public:
        forceinline void SetDepthBoundsTest(bool v) noexcept
        {
            if (depth_bounds_test == v)
            {
                return;
            }
            depth_bounds_test = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsDepthBoundsTest() const noexcept
        {
            return depth_bounds_test;
        }

    public:
        forceinline void SetDepthFunc(vk::CompareOp v) noexcept
        {
            if (depth_func == v)
            {
                return;
            }
            depth_func = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::CompareOp GetDepthFunc() const noexcept
        {
            return depth_func;
        }

    public:
        forceinline void SetStencil(bool v) noexcept
        {
            if (stencil == v)
            {
                return;
            }
            stencil = v;
            hash = 0;
        }

        [[nodiscard]] forceinline bool IsStencil() const noexcept
        {
            return stencil;
        }

    public:
        forceinline void SetStencilReadMask(uint8_t v) noexcept
        {
            if (stencil_read_mask == v)
            {
                return;
            }
            stencil_read_mask = v;
            hash = 0;
        }

        [[nodiscard]] forceinline uint8_t GetStencilReadMask() const noexcept
        {
            return stencil_read_mask;
        }

    public:
        forceinline void SetStencilWriteMask(uint8_t v) noexcept
        {
            if (stencil_write_mask == v)
            {
                return;
            }
            stencil_write_mask = v;
            hash = 0;
        }

        [[nodiscard]] forceinline uint8_t GetStencilWriteMask() const noexcept
        {
            return stencil_write_mask;
        }

    public:
        forceinline void SetFrontFaceStencilOp(vk::StencilOpState v) noexcept
        {
            if (front_face == v)
            {
                return;
            }
            front_face = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::StencilOpState GetFrontFaceStencilOp() const noexcept
        {
            return front_face;
        }

    public:
        forceinline void SetBackFaceStencilOp(vk::StencilOpState v) noexcept
        {
            if (back_face == v)
            {
                return;
            }
            back_face = v;
            hash = 0;
        }

        [[nodiscard]] forceinline vk::StencilOpState GetBackFaceStencilOp() const noexcept
        {
            return back_face;
        }

    public:
        [[nodiscard]] HashValue Hash() const noexcept
        {
            if (size_t(hash) == 0)
            {
                hash = HashOf(depth_test);
                hash += HashOf(depth_write);
                hash += HashOf(depth_bounds_test);

                if (depth_test)
                {
                    hash += HashOf(depth_func);
                }

                hash += HashOf(stencil);
                if (stencil)
                {
                    hash += HashOf(stencil_read_mask);
                    hash += HashOf(stencil_write_mask);
                    hash += HashOf(&front_face, sizeof(vk::StencilOpState));
                    hash += HashOf(&back_face, sizeof(vk::StencilOpState));
                }
            }
            return hash;
        }

    public:
        [[nodiscard]] auto operator<=>(const RhiDepthStencilState &) const noexcept = default;

    public:
        bool depth_test{true};
        bool depth_write{true};
        bool depth_bounds_test{false};

        vk::CompareOp depth_func{vk::CompareOp::eLess};

        bool stencil{false};

        uint8_t stencil_read_mask{0xff};
        uint8_t stencil_write_mask{0xff};

        vk::StencilOpState front_face{};
        vk::StencilOpState back_face{};

    private:
        mutable HashValue hash{0};
    };

}

namespace std
{

    template <>
    struct hash<Be::Framework::RHI::RhiDepthStencilState>
    {
        std::size_t operator()(const Be::Framework::RHI::RhiDepthStencilState &v) const noexcept
        {
            return size_t(v.Hash());
        }
    };

}